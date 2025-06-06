/*
 * HT handling
 *
 * Copyright 2003, Jouni Malinen <jkmaline@cc.hut.fi>
 * Copyright 2002-2005, Instant802 Networks, Inc.
 * Copyright 2005-2006, Devicescape Software, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2007, Michael Wu <flamingice@sourmilk.net>
 * Copyright 2007-2010, Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * DOC: RX A-MPDU aggregation
 *
 * Aggregation on the RX side requires only implementing the
 * @ampdu_action callback that is invoked to start/stop any
 * block-ack sessions for RX aggregation.
 *
 * When RX aggregation is started by the peer, the driver is
 * notified via @ampdu_action function, with the
 * %IEEE80211_AMPDU_RX_START action, and may reject the request
 * in which case a negative response is sent to the peer, if it
 * accepts it a positive response is sent.
 *
 * While the session is active, the device/driver are required
 * to de-aggregate frames and pass them up one by one to mac80211,
 * which will handle the reorder buffer.
 *
 * When the aggregation session is stopped again by the peer or
 * ourselves, the driver's @ampdu_action function will be called
 * with the action %IEEE80211_AMPDU_RX_STOP. In this case, the
 * call must not fail.
 */

#include <linux/ieee80211.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <net/atbm_mac80211.h>
#include "ieee80211_i.h"
#include "driver-ops.h"

static int ieee80211_rx_ba_alloc_hw_token(struct ieee80211_local *local)
{
	int token = -1;
	
	spin_lock_bh(&local->aggr_lock);

	if(ieee80211_hw_setup_ba(&local->hw)){
		token = 0;
		goto exit;
	}
	token = find_first_zero_bit(local->rx_aggr_map,local->hw.max_hw_support_rx_aggs);

	if(token < 0 || token >= local->hw.max_hw_support_rx_aggs){
		token = -1;
		goto exit;
	}

	__set_bit(token,local->rx_aggr_map);
exit:
	spin_unlock_bh(&local->aggr_lock);
	return token;
}

static void ieee80211_rx_ba_free_hw_token(struct ieee80211_local *local,int token)
{
	spin_lock_bh(&local->aggr_lock);
	
	if(ieee80211_hw_setup_ba(&local->hw)){
		goto exit;
	}
	if(token > local->hw.max_hw_support_rx_aggs || token < 0){
		goto exit;
	}

	__clear_bit(token,local->rx_aggr_map);
exit:
	spin_unlock_bh(&local->aggr_lock);
}

static void ieee80211_free_tid_rx(struct rcu_head *h)
{
	struct tid_ampdu_rx *tid_rx =
		container_of(h, struct tid_ampdu_rx, rcu_head);
	int i;

	for (i = 0; i < tid_rx->buf_size; i++)
		atbm_dev_kfree_skb(tid_rx->reorder_buf[i]);
	atbm_kfree(tid_rx->reorder_buf);
	atbm_kfree(tid_rx->reorder_time);
	atbm_kfree(tid_rx);
}

void ___ieee80211_stop_rx_ba_session(struct sta_info *sta, u16 tid,
				     u16 initiator, u16 reason, bool tx)
{
	struct ieee80211_local *local = sta->local;
	struct tid_ampdu_rx *tid_rx;

	lockdep_assert_held(&sta->ampdu_mlme.mtx);

	tid_rx = rcu_dereference_protected(sta->ampdu_mlme.tid_rx[tid],
					lockdep_is_held(&sta->ampdu_mlme.mtx));

	if (!tid_rx)
		return;

	RCU_INIT_POINTER(sta->ampdu_mlme.tid_rx[tid], NULL);

#ifdef CONFIG_MAC80211_ATBM_HT_DEBUG
	atbm_printk_agg("Rx BA session stop requested for %pM tid %u\n",
	       sta->sta.addr, tid);
#endif /* CONFIG_MAC80211_ATBM_HT_DEBUG */

	if (drv_ampdu_action(local, sta->sdata, IEEE80211_AMPDU_RX_STOP,
			     &sta->sta, tid, NULL, 0, tid_rx->hw_token))
		atbm_printk_agg( "HW problem - can not stop rx "
				"aggregation for tid %d\n", tid);
#ifdef CONFIG_ATBM_DRIVER_PROCESS_BA
	/* check if this is a self generated aggregation halt */
	if (initiator == WLAN_BACK_RECIPIENT && tx && !ieee80211_hw_setup_ba(&local->hw))
		ieee80211_send_delba(sta->sdata, sta->sta.addr,
				     tid, 0, reason);
#endif
	atbm_del_timer_sync(&tid_rx->session_timer);
	atbm_del_timer_sync(&tid_rx->reorder_timer);
	ieee80211_rx_ba_free_hw_token(local,tid_rx->hw_token);

	call_rcu(&tid_rx->rcu_head, ieee80211_free_tid_rx);
}

void __ieee80211_stop_rx_ba_session(struct sta_info *sta, u16 tid,
				    u16 initiator, u16 reason, bool tx)
{
	mutex_lock(&sta->ampdu_mlme.mtx);
	___ieee80211_stop_rx_ba_session(sta, tid, initiator, reason, tx);
	mutex_unlock(&sta->ampdu_mlme.mtx);
}
#ifdef CONFIG_ATBM_MAC80211_NO_USE
void ieee80211_stop_rx_ba_session(struct ieee80211_vif *vif, u16 ba_rx_bitmap,
				  const u8 *addr)
{
	struct ieee80211_sub_if_data *sdata = vif_to_sdata(vif);
	struct sta_info *sta;
	int i;

	rcu_read_lock();
	sta = sta_info_get(sdata, addr);
	if (!sta) {
		rcu_read_unlock();
		return;
	}

	for (i = 0; i < STA_TID_NUM; i++)
		if (ba_rx_bitmap & BIT(i))
			set_bit(i, sta->ampdu_mlme.tid_rx_stop_requested);

	ieee80211_queue_work(&sta->local->hw, &sta->ampdu_mlme.work);
	rcu_read_unlock();
}
//EXPORT_SYMBOL(ieee80211_stop_rx_ba_session);
#endif
/*
 * After accepting the AddBA Request we activated a timer,
 * resetting it after each frame that arrives from the originator.
 */
static void sta_rx_agg_session_timer_expired(unsigned long data)
{
	/* not an elegant detour, but there is no choice as the timer passes
	 * only one argument, and various sta_info are needed here, so init
	 * flow in sta_info_create gives the TID as data, while the timer_to_id
	 * array gives the sta through container_of */
	u8 *ptid = (u8 *)data;
	u8 *timer_to_id = ptid - *ptid;
	struct sta_info *sta = container_of(timer_to_id, struct sta_info,
					 timer_to_tid[0]);

#ifdef CONFIG_MAC80211_ATBM_HT_DEBUG
	atbm_printk_agg("rx session timer expired on tid %d\n", (u16)*ptid);
#endif
	set_bit(*ptid, sta->ampdu_mlme.tid_rx_timer_expired);
	ieee80211_queue_work(&sta->local->hw, &sta->ampdu_mlme.work);
}

static void sta_rx_agg_reorder_timer_expired(unsigned long data)
{
	u8 *ptid = (u8 *)data;
	u8 *timer_to_id = ptid - *ptid;
	struct sta_info *sta = container_of(timer_to_id, struct sta_info,
			timer_to_tid[0]);

	rcu_read_lock();
	ieee80211_release_reorder_timeout(sta, *ptid);
	rcu_read_unlock();
}
#ifdef CONFIG_ATBM_DRIVER_PROCESS_BA
static void ieee80211_send_addba_resp(struct ieee80211_sub_if_data *sdata, u8 *da, u16 tid,
				      u8 dialog_token, u16 status, u16 policy,
				      u16 buf_size, u16 timeout)
{
	struct ieee80211_local *local = sdata->local;
	struct sk_buff *skb;
	struct atbm_ieee80211_mgmt *mgmt;
	u16 capab;

	skb = atbm_dev_alloc_skb(sizeof(*mgmt) + local->hw.extra_tx_headroom);
	if (!skb)
		return;

	atbm_skb_reserve(skb, local->hw.extra_tx_headroom);
	mgmt = (struct atbm_ieee80211_mgmt *) atbm_skb_put(skb, 24);
	memset(mgmt, 0, 24);
	memcpy(mgmt->da, da, ETH_ALEN);
	memcpy(mgmt->sa, sdata->vif.addr, ETH_ALEN);
	if (sdata->vif.type == NL80211_IFTYPE_AP ||
	    sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
		memcpy(mgmt->bssid, sdata->vif.addr, ETH_ALEN);
	else if (sdata->vif.type == NL80211_IFTYPE_STATION)
		memcpy(mgmt->bssid, sdata->u.mgd.bssid, ETH_ALEN);

	mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
					  IEEE80211_STYPE_ACTION);

	atbm_skb_put(skb, 1 + sizeof(mgmt->u.action.u.addba_resp));
	mgmt->u.action.category = ATBM_WLAN_CATEGORY_BACK;
	mgmt->u.action.u.addba_resp.action_code = WLAN_ACTION_ADDBA_RESP;
	mgmt->u.action.u.addba_resp.dialog_token = dialog_token;

	capab = (u16)(policy << 1);	/* bit 1 aggregation policy */
	capab |= (u16)(tid << 2); 	/* bit 5:2 TID number */
	capab |= (u16)(buf_size << 6);	/* bit 15:6 max size of aggregation */

	mgmt->u.action.u.addba_resp.capab = cpu_to_le16(capab);
	mgmt->u.action.u.addba_resp.timeout = cpu_to_le16(timeout);
	mgmt->u.action.u.addba_resp.status = cpu_to_le16(status);

	ieee80211_tx_skb(sdata, skb);
}
#endif
void ieee80211_process_addba_request(struct ieee80211_local *local,
				     struct sta_info *sta,
				     struct atbm_ieee80211_mgmt *mgmt,
				     size_t len)
{
	struct tid_ampdu_rx *tid_agg_rx;
	u16 capab, tid, timeout, ba_policy, buf_size, start_seq_num;
#ifdef CONFIG_ATBM_DRIVER_PROCESS_BA
	u16 status;
	int tx = !ieee80211_hw_setup_ba(&local->hw);
#endif
	u8 dialog_token;
	int ret = -EOPNOTSUPP;
	int hw_token = -1;
	/* extract session parameters from addba request frame */
	dialog_token = mgmt->u.action.u.addba_req.dialog_token;
	timeout = le16_to_cpu(mgmt->u.action.u.addba_req.timeout);
	start_seq_num =
		le16_to_cpu(mgmt->u.action.u.addba_req.start_seq_num) >> 4;

	capab = le16_to_cpu(mgmt->u.action.u.addba_req.capab);
	ba_policy = (capab & IEEE80211_ADDBA_PARAM_POLICY_MASK) >> 1;
	tid = (capab & IEEE80211_ADDBA_PARAM_TID_MASK) >> 2;
	buf_size = (capab & IEEE80211_ADDBA_PARAM_BUF_SIZE_MASK) >> 6;
#ifdef CONFIG_ATBM_DRIVER_PROCESS_BA

	status = WLAN_STATUS_REQUEST_DECLINED;
#endif
	if (test_sta_flag(sta, WLAN_STA_BLOCK_BA)) {
#ifdef CONFIG_MAC80211_ATBM_HT_DEBUG
		atbm_printk_agg("Suspend in progress. "
		       "Denying ADDBA request\n");
#endif
		goto end_no_lock;
	}

	/* sanity check for incoming parameters:
	 * check if configuration can support the BA policy
	 * and if buffer size does not exceeds max value */
	/* XXX: check own ht delayed BA capability?? */
	if (((ba_policy != 1) &&
	     (!(sta->sta.ht_cap.cap & IEEE80211_HT_CAP_DELAY_BA))) ||
	    (buf_size > 64/*IEEE80211_MAX_AMPDU_BUF*/)) {
#ifdef CONFIG_ATBM_DRIVER_PROCESS_BA

		status = WLAN_STATUS_INVALID_QOS_PARAM;
#endif
#ifdef CONFIG_MAC80211_ATBM_HT_DEBUG
		if (net_ratelimit())
			atbm_printk_agg( "AddBA Req with bad params from "
				"%pM on tid %u. policy %d, buffer size %d\n",
				mgmt->sa, tid, ba_policy,
				buf_size);
#endif /* CONFIG_MAC80211_ATBM_HT_DEBUG */
		goto end_no_lock;
	}
	/* determine default buffer size */
	if (buf_size == 0)
		buf_size = 64;//IEEE80211_MAX_AMPDU_BUF;

	/* make sure the size doesn't exceed the maximum supported by the hw */
	if (buf_size > local->hw.max_rx_aggregation_subframes)
		buf_size = local->hw.max_rx_aggregation_subframes;

	/* examine state machine */
	mutex_lock(&sta->ampdu_mlme.mtx);

	if (sta->ampdu_mlme.tid_rx[tid]) {
#ifdef CONFIG_MAC80211_ATBM_HT_DEBUG
		if (net_ratelimit())
			atbm_printk_agg( "unexpected AddBA Req from "
				"%pM on tid %u\n",
				mgmt->sa, tid);
#endif /* CONFIG_MAC80211_ATBM_HT_DEBUG */

		/* delete existing Rx BA session on the same tid */
		___ieee80211_stop_rx_ba_session(sta, tid, WLAN_BACK_RECIPIENT,
						WLAN_STATUS_UNSPECIFIED_QOS,
						false);
	}

	/* prepare A-MPDU MLME for Rx aggregation */
	tid_agg_rx = atbm_kmalloc(sizeof(struct tid_ampdu_rx), GFP_KERNEL);
	if (!tid_agg_rx)
		goto end;


	
	atbm_printk_init("%s:mac[%pM],tid[%d]\n",__func__,sta->sta.addr,tid);
	spin_lock_init(&tid_agg_rx->reorder_lock);

	/* rx timer */
	tid_agg_rx->session_timer.function = sta_rx_agg_session_timer_expired;
	tid_agg_rx->session_timer.data = (unsigned long)&sta->timer_to_tid[tid];
	atbm_init_timer(&tid_agg_rx->session_timer);

	/* rx reorder timer */
	tid_agg_rx->reorder_timer.function = sta_rx_agg_reorder_timer_expired;
	tid_agg_rx->reorder_timer.data = (unsigned long)&sta->timer_to_tid[tid];
	atbm_init_timer(&tid_agg_rx->reorder_timer);

	/* prepare reordering buffer */
	tid_agg_rx->reorder_buf =
		atbm_kcalloc(buf_size, sizeof(struct sk_buff *), GFP_KERNEL);
	tid_agg_rx->reorder_time =
		atbm_kcalloc(buf_size, sizeof(unsigned long), GFP_KERNEL);
	if (!tid_agg_rx->reorder_buf || !tid_agg_rx->reorder_time) {
		atbm_kfree(tid_agg_rx->reorder_buf);
		atbm_kfree(tid_agg_rx->reorder_time);
		atbm_kfree(tid_agg_rx);
		goto end;
	}

	hw_token = ieee80211_rx_ba_alloc_hw_token(local);
	
	if (hw_token == -1){
		atbm_kfree(tid_agg_rx->reorder_buf);
		atbm_kfree(tid_agg_rx->reorder_time);
		atbm_kfree(tid_agg_rx);
		atbm_printk_err("aggr token not enough\n");
		goto end;
	}

		
	ret = drv_ampdu_action(local, sta->sdata, IEEE80211_AMPDU_RX_START,
			       &sta->sta, tid, &start_seq_num, 0, hw_token);
#ifdef CONFIG_MAC80211_ATBM_HT_DEBUG
	atbm_printk_agg( "Rx A-MPDU request on tid %d result %d\n", tid, ret);
#endif /* CONFIG_MAC80211_ATBM_HT_DEBUG */

	if (ret) {
		atbm_kfree(tid_agg_rx->reorder_buf);
		atbm_kfree(tid_agg_rx->reorder_time);
		atbm_kfree(tid_agg_rx);
		ieee80211_rx_ba_free_hw_token(local, hw_token);
		goto end;
	}

	tid_agg_rx->hw_token = hw_token;
	/* update data */
	tid_agg_rx->dialog_token = dialog_token;
	tid_agg_rx->ssn = start_seq_num;
	tid_agg_rx->head_seq_num = start_seq_num;
	tid_agg_rx->buf_size = buf_size;
	tid_agg_rx->timeout = timeout;
	tid_agg_rx->stored_mpdu_num = 0;
#ifdef CONFIG_ATBM_DRIVER_PROCESS_BA
	status = WLAN_STATUS_SUCCESS;
#endif
	/* activate it for RX */
	rcu_assign_pointer(sta->ampdu_mlme.tid_rx[tid], tid_agg_rx);

	if (timeout)
		atbm_mod_timer(&tid_agg_rx->session_timer, TU_TO_EXP_TIME(timeout));

end:
	mutex_unlock(&sta->ampdu_mlme.mtx);

end_no_lock:
#ifdef CONFIG_ATBM_DRIVER_PROCESS_BA
	if(tx)
		ieee80211_send_addba_resp(sta->sdata, sta->sta.addr, tid,
				  dialog_token, status, 1, buf_size, timeout);
#endif

	return;
}
