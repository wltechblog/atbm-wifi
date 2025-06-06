/*
 * mac80211 <-> driver interface
 *
 * Copyright 2002-2005, Devicescape Software, Inc.
 * Copyright 2006-2007	Jiri Benc <jbenc@suse.cz>
 * Copyright 2007-2010	Johannes Berg <johannes@sipsolutions.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef ATBM_MAC80211_H
#define ATBM_MAC80211_H

#include <linux/kernel.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/device.h>
#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#include <asm/unaligned.h>
#include <linux/hash.h>
#include <linux/module.h>
#include <net/ieee80211_radiotap.h>
#include <linux/platform_device.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <uapi/linux/sched/types.h>
#endif

#ifdef CONFIG_ATBM_SELF_WORKQUEUE
struct atbm_work_struct;
struct atbm_delayed_work;
struct atbm_workqueue_struct;
#else
#define atbm_workqueue_struct			workqueue_struct
#define atbm_delayed_work 				delayed_work
#define atbm_work_struct  				work_struct
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
#define IEEE80211_BAND_2GHZ NL80211_BAND_2GHZ
#define IEEE80211_BAND_5GHZ NL80211_BAND_5GHZ
#define IEEE80211_NUM_BANDS (NL80211_BAND_5GHZ+1)
#define ieee80211_band nl80211_band 
#define atbm_notify_scan_done(__local,__scan_request,__abort) \
do	{			\
	__local->scan_info.aborted = __abort;		\
	cfg80211_scan_done(__scan_request,&__local->scan_info);		\
}while(0)
#else
struct cfg80211_scan_info{
	bool aborted;
};
#define atbm_notify_scan_done(__local,__scan_request,__abort) \
do {			\
	BUG_ON(__local==NULL);			\
	cfg80211_scan_done(__scan_request,__abort);				\
}while(0)
#endif

#ifdef IPV6_FILTERING
#include <linux/in6.h>
#endif /*IPV6_FILTERING*/

#ifndef IEEE80211_QOS_CTL_EOSP
#define IEEE80211_QOS_CTL_EOSP			0x0010
#endif

#ifndef IEEE80211_BAR_CTRL_MULTI_TID
#define IEEE80211_BAR_CTRL_MULTI_TID		0x0002
#endif
#ifndef IEEE80211_BAR_CTRL_TID_INFO_MASK
#define IEEE80211_BAR_CTRL_TID_INFO_MASK	0xf000
#endif

#ifndef IEEE80211_BAR_CTRL_TID_INFO_SHIFT
#define IEEE80211_BAR_CTRL_TID_INFO_SHIFT	12
#endif
#ifndef IEEE80211_RADIOTAP_F_TX_NOACK
#define IEEE80211_RADIOTAP_F_TX_NOACK	0x0008	/* don't expect an ack */
#endif
#ifndef IEEE80211_QOS_CTL_ACK_POLICY_NOACK
#define IEEE80211_QOS_CTL_ACK_POLICY_NOACK	0x0020
#endif

#ifndef IEEE80211_QOS_CTL_MESH_CONTROL_PRESENT
#define IEEE80211_QOS_CTL_MESH_CONTROL_PRESENT  0x0100
#endif

#ifndef ERP_INFO_BYTE_OFFSET
#define ERP_INFO_BYTE_OFFSET 2
#endif
int atbm_ieee80211_init(void);
void atbm_ieee80211_exit(void);

extern int cfg80211_altmtest_reply(struct wiphy *wiphy,
				const void *data, int len);
#define DBUG_AUTHEN(fc)		if(ieee80211_is_auth(fc)) atbm_printk_mgmt("%s:%d,fc(%x)\n",__func__,__LINE__,fc);
static inline unsigned atbm_compare_ether_addr(const u8 *addr1, const u8 *addr2)
{
	const u16 *a = (const u16 *) addr1;
	const u16 *b = (const u16 *) addr2;

	BUILD_BUG_ON(ETH_ALEN != 6);
	return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) != 0;
}

#if 0
static inline u32 le32_get_bits(u32 v, u32 field)
{									
	return ((v) & field)/((field) & -(field));
}
#ifdef __LITTLE_ENDIAN

#define u32_get_bits le32_get_bits

#else
#endif  //#ifdef __LITTLE_ENDIAN__
#else //0
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4,5,0))
#include <linux/bitfield.h>
#endif
#endif  //0

/* Information Element IDs */
enum atbm_ieee80211_eid {
	ATBM_WLAN_EID_SSID = 0,
	ATBM_WLAN_EID_SUPP_RATES = 1,
	ATBM_WLAN_EID_FH_PARAMS = 2,
	ATBM_WLAN_EID_DS_PARAMS = 3,
	ATBM_WLAN_EID_CF_PARAMS = 4,
	ATBM_WLAN_EID_TIM = 5,
	ATBM_WLAN_EID_IBSS_PARAMS = 6,
	ATBM_WLAN_EID_CHALLENGE = 16,

	ATBM_WLAN_EID_COUNTRY = 7,
	ATBM_WLAN_EID_HP_PARAMS = 8,
	ATBM_WLAN_EID_HP_TABLE = 9,
	ATBM_WLAN_EID_REQUEST = 10,

	ATBM_WLAN_EID_QBSS_LOAD = 11,
	ATBM_WLAN_EID_EDCA_PARAM_SET = 12,
	ATBM_WLAN_EID_TSPEC = 13,
	ATBM_WLAN_EID_TCLAS = 14,
	ATBM_WLAN_EID_SCHEDULE = 15,
	ATBM_WLAN_EID_TS_DELAY = 43,
	ATBM_WLAN_EID_TCLAS_PROCESSING = 44,
	ATBM_WLAN_EID_QOS_CAPA = 46,
	/* 802.11z */
	ATBM_WLAN_EID_LINK_ID = 101,
	/* 802.11s */
	ATBM_WLAN_EID_MESH_CONFIG = 113,
	ATBM_WLAN_EID_MESH_ID = 114,
	ATBM_WLAN_EID_LINK_METRIC_REPORT = 115,
	ATBM_WLAN_EID_CONGESTION_NOTIFICATION = 116,
	ATBM_WLAN_EID_PEER_MGMT = 117,
	ATBM_WLAN_EID_CHAN_SWITCH_PARAM = 118,
	ATBM_WLAN_EID_MESH_AWAKE_WINDOW = 119,
	ATBM_WLAN_EID_BEACON_TIMING = 120,
	ATBM_WLAN_EID_MCCAOP_SETUP_REQ = 121,
	ATBM_WLAN_EID_MCCAOP_SETUP_RESP = 122,
	ATBM_WLAN_EID_MCCAOP_ADVERT = 123,
	ATBM_WLAN_EID_MCCAOP_TEARDOWN = 124,
	ATBM_WLAN_EID_GANN = 125,
	ATBM_WLAN_EID_RANN = 126,
	ATBM_WLAN_EID_PREQ = 130,
	ATBM_WLAN_EID_PREP = 131,
	ATBM_WLAN_EID_PERR = 132,
	ATBM_WLAN_EID_PXU = 137,
	ATBM_WLAN_EID_PXUC = 138,
	ATBM_WLAN_EID_AUTH_MESH_PEER_EXCH = 139,
	ATBM_WLAN_EID_MIC = 140,

	ATBM_WLAN_EID_PWR_CONSTRAINT = 32,
	ATBM_WLAN_EID_PWR_CAPABILITY = 33,
	ATBM_WLAN_EID_TPC_REQUEST = 34,
	ATBM_WLAN_EID_TPC_REPORT = 35,
	ATBM_WLAN_EID_SUPPORTED_CHANNELS = 36,
	ATBM_WLAN_EID_CHANNEL_SWITCH = 37,
	ATBM_WLAN_EID_MEASURE_REQUEST = 38,
	ATBM_WLAN_EID_MEASURE_REPORT = 39,
	ATBM_WLAN_EID_QUIET = 40,
	ATBM_WLAN_EID_IBSS_DFS = 41,

	ATBM_WLAN_EID_ERP_INFO = 42,
	ATBM_WLAN_EID_EXT_SUPP_RATES = 50,

	ATBM_WLAN_EID_HT_CAPABILITY = 45,
	ATBM_WLAN_EID_HT_INFORMATION = 61,

	ATBM_WLAN_EID_RSN = 48,
	ATBM_WLAN_EID_MMIE = 76,
	ATBM_WLAN_EID_WPA = 221,
	ATBM_WLAN_EID_GENERIC = 221,
	ATBM_WLAN_EID_VENDOR_SPECIFIC = 221,
	ATBM_WLAN_EID_QOS_PARAMETER = 222,

	ATBM_WLAN_EID_AP_CHAN_REPORT = 51,
	ATBM_WLAN_EID_NEIGHBOR_REPORT = 52,
	ATBM_WLAN_EID_RCPI = 53,
	ATBM_WLAN_EID_BSS_AVG_ACCESS_DELAY = 63,
	ATBM_WLAN_EID_ANTENNA_INFO = 64,
	ATBM_WLAN_EID_RSNI = 65,
	ATBM_WLAN_EID_MEASUREMENT_PILOT_TX_INFO = 66,
	ATBM_WLAN_EID_BSS_AVAILABLE_CAPACITY = 67,
	ATBM_WLAN_EID_BSS_AC_ACCESS_DELAY = 68,
	ATBM_WLAN_EID_RRM_ENABLED_CAPABILITIES = 70,
	ATBM_WLAN_EID_MULTIPLE_BSSID = 71,
	ATBM_WLAN_EID_BSS_COEX_2040 = 72,
	ATBM_WLAN_EID_OVERLAP_BSS_SCAN_PARAM = 74,
	ATBM_WLAN_EID_EXT_CAPABILITY = 127,

	ATBM_WLAN_EID_MOBILITY_DOMAIN = 54,
	ATBM_WLAN_EID_FAST_BSS_TRANSITION = 55,
	ATBM_WLAN_EID_TIMEOUT_INTERVAL = 56,
	ATBM_WLAN_EID_RIC_DATA = 57,
	ATBM_WLAN_EID_RIC_DESCRIPTOR = 75,

	ATBM_WLAN_EID_DSE_REGISTERED_LOCATION = 58,
	ATBM_WLAN_EID_SUPPORTED_REGULATORY_CLASSES = 59,
	ATBM_WLAN_EID_EXT_CHANSWITCH_ANN = 60,
	ATBM_WLAN_EID_SECONDARY_CH_OFFSET = 62,
	ATBM_WLAN_EID_PRIVATE = 233,
};
/* Action category code */
enum atbm_ieee80211_category {
	ATBM_WLAN_CATEGORY_SPECTRUM_MGMT = 0,
	ATBM_WLAN_CATEGORY_QOS = 1,
	ATBM_WLAN_CATEGORY_DLS = 2,
	ATBM_WLAN_CATEGORY_BACK = 3,
	ATBM_WLAN_CATEGORY_PUBLIC = 4,
	ATBM_WLAN_CATEGORY_RADIO_MEASUREMENT = 5,
	ATBM_WLAN_CATEGORY_HT = 7,
	ATBM_WLAN_CATEGORY_SA_QUERY = 8,
	ATBM_WLAN_CATEGORY_PROTECTED_DUAL_OF_ACTION = 9,
	ATBM_WLAN_CATEGORY_WNM = 10,
	ATBM_WLAN_CATEGORY_WNM_UNPROTECTED = 11,
	ATBM_WLAN_CATEGORY_TDLS = 12,
	ATBM_WLAN_CATEGORY_MESH_ACTION = 13,
	ATBM_WLAN_CATEGORY_MULTIHOP_ACTION = 14,
	ATBM_WLAN_CATEGORY_SELF_PROTECTED = 15,
	ATBM_WLAN_CATEGORY_DMG = 16,
	ATBM_WLAN_CATEGORY_WMM = 17,
	ATBM_WLAN_CATEGORY_FST = 18,
	ATBM_WLAN_CATEGORY_UNPROT_DMG = 20,
	ATBM_WLAN_CATEGORY_VHT = 21,
	ATBM_WLAN_CATEGORY_VENDOR_SPECIFIC_PROTECTED = 126,
	ATBM_WLAN_CATEGORY_VENDOR_SPECIFIC = 127,
};
/* Status codes */
enum atbm_ieee80211_statuscode {
	ATBM_WLAN_STATUS_SUCCESS = 0,
	ATBM_WLAN_STATUS_UNSPECIFIED_FAILURE = 1,
	ATBM_WLAN_STATUS_CAPS_UNSUPPORTED = 10,
	ATBM_WLAN_STATUS_REASSOC_NO_ASSOC = 11,
	ATBM_WLAN_STATUS_ASSOC_DENIED_UNSPEC = 12,
	ATBM_WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG = 13,
	ATBM_WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION = 14,
	ATBM_WLAN_STATUS_CHALLENGE_FAIL = 15,
	ATBM_WLAN_STATUS_AUTH_TIMEOUT = 16,
	ATBM_WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA = 17,
	ATBM_WLAN_STATUS_ASSOC_DENIED_RATES = 18,
	/* 802.11b */
	ATBM_WLAN_STATUS_ASSOC_DENIED_NOSHORTPREAMBLE = 19,
	ATBM_WLAN_STATUS_ASSOC_DENIED_NOPBCC = 20,
	ATBM_WLAN_STATUS_ASSOC_DENIED_NOAGILITY = 21,
	/* 802.11h */
	ATBM_WLAN_STATUS_ASSOC_DENIED_NOSPECTRUM = 22,
	ATBM_WLAN_STATUS_ASSOC_REJECTED_BAD_POWER = 23,
	ATBM_WLAN_STATUS_ASSOC_REJECTED_BAD_SUPP_CHAN = 24,
	/* 802.11g */
	ATBM_WLAN_STATUS_ASSOC_DENIED_NOSHORTTIME = 25,
	ATBM_WLAN_STATUS_ASSOC_DENIED_NODSSSOFDM = 26,
	/* 802.11w */
	ATBM_WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY = 30,
	ATBM_WLAN_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION = 31,
	/* 802.11i */
	ATBM_WLAN_STATUS_INVALID_IE = 40,
	ATBM_WLAN_STATUS_INVALID_GROUP_CIPHER = 41,
	ATBM_WLAN_STATUS_INVALID_PAIRWISE_CIPHER = 42,
	ATBM_WLAN_STATUS_INVALID_AKMP = 43,
	ATBM_WLAN_STATUS_UNSUPP_RSN_VERSION = 44,
	ATBM_WLAN_STATUS_INVALID_RSN_IE_CAP = 45,
	ATBM_WLAN_STATUS_CIPHER_SUITE_REJECTED = 46,
	/* 802.11e */
	ATBM_WLAN_STATUS_UNSPECIFIED_QOS = 32,
	ATBM_WLAN_STATUS_ASSOC_DENIED_NOBANDWIDTH = 33,
	ATBM_WLAN_STATUS_ASSOC_DENIED_LOWACK = 34,
	ATBM_WLAN_STATUS_ASSOC_DENIED_UNSUPP_QOS = 35,
	ATBM_WLAN_STATUS_REQUEST_DECLINED = 37,
	ATBM_WLAN_STATUS_INVALID_QOS_PARAM = 38,
	ATBM_WLAN_STATUS_CHANGE_TSPEC = 39,
	ATBM_WLAN_STATUS_WAIT_TS_DELAY = 47,
	ATBM_WLAN_STATUS_NO_DIRECT_LINK = 48,
	ATBM_WLAN_STATUS_STA_NOT_PRESENT = 49,
	ATBM_WLAN_STATUS_STA_NOT_QSTA = 50,
	/* 802.11s */
	ATBM_WLAN_STATUS_ANTI_CLOG_REQUIRED = 76,
	ATBM_WLAN_STATUS_FCG_NOT_SUPP = 78,
	ATBM_WLAN_STATUS_STA_NO_TBTT = 78,
	/* 802.11ad */
	ATBM_WLAN_STATUS_REJECTED_WITH_SUGGESTED_CHANGES = 39,
	ATBM_WLAN_STATUS_REJECTED_FOR_DELAY_PERIOD = 47,
	ATBM_WLAN_STATUS_REJECT_WITH_SCHEDULE = 83,
	ATBM_WLAN_STATUS_PENDING_ADMITTING_FST_SESSION = 86,
	ATBM_WLAN_STATUS_PERFORMING_FST_NOW = 87,
	ATBM_WLAN_STATUS_PENDING_GAP_IN_BA_WINDOW = 88,
	ATBM_WLAN_STATUS_REJECT_U_PID_SETTING = 89,
	ATBM_WLAN_STATUS_REJECT_DSE_BAND = 96,
	ATBM_WLAN_STATUS_DENIED_WITH_SUGGESTED_BAND_AND_CHANNEL = 99,
	ATBM_WLAN_STATUS_DENIED_DUE_TO_SPECTRUM_MANAGEMENT = 103,
	/* 802.11ai */
	ATBM_WLAN_STATUS_FILS_AUTHENTICATION_FAILURE = 108,
	ATBM_WLAN_STATUS_UNKNOWN_AUTHENTICATION_SERVER = 109,
	ATBM_WLAN_STATUS_SAE_HASH_TO_ELEMENT = 126,
	ATBM_WLAN_STATUS_SAE_PK = 127,
};

struct atbm_wpa_ie_data {
	int proto;
	int pairwise_cipher;
	int group_cipher;
	int key_mgmt;
	int wpa_capabilities;
	int rsn_capabilities;
	size_t num_pmkid;
	const u8 *pmkid;
	int mgmt_group_cipher;
};
#define ATBM_WLAN_CAPABILITY_ESS BIT(0)
#define ATBM_WLAN_CAPABILITY_IBSS BIT(1)
#define ATBM_WLAN_CAPABILITY_CF_POLLABLE BIT(2)
#define ATBM_WLAN_CAPABILITY_CF_POLL_REQUEST BIT(3)
#define ATBM_WLAN_CAPABILITY_PRIVACY BIT(4)
#define ATBM_WLAN_CAPABILITY_SHORT_PREAMBLE BIT(5)
#define ATBM_WLAN_CAPABILITY_PBCC BIT(6)
#define ATBM_WLAN_CAPABILITY_CHANNEL_AGILITY BIT(7)
#define ATBM_WLAN_CAPABILITY_SPECTRUM_MGMT BIT(8)
#define ATBM_WLAN_CAPABILITY_SHORT_SLOT_TIME BIT(10)
#define ATBM_WLAN_CAPABILITY_DSSS_OFDM BIT(13)

/* IEEE 802.11, 7.3.2.25.3 RSN Capabilities */
#define ATBM_WPA_CAPABILITY_PREAUTH BIT(0)
#define ATBM_WPA_CAPABILITY_NO_PAIRWISE BIT(1)
/* B2-B3: PTKSA Replay Counter */
/* B4-B5: GTKSA Replay Counter */
#define ATBM_WPA_CAPABILITY_MFPR BIT(6)
#define ATBM_WPA_CAPABILITY_MFPC BIT(7)
/* B8: Reserved */
#define ATBM_WPA_CAPABILITY_PEERKEY_ENABLED BIT(9)
#define ATBM_WPA_CAPABILITY_SPP_A_MSDU_CAPABLE BIT(10)
#define ATBM_WPA_CAPABILITY_SPP_A_MSDU_REQUIRED BIT(11)
#define ATBM_WPA_CAPABILITY_PBAC BIT(12)
#define ATBM_WPA_CAPABILITY_EXT_KEY_ID_FOR_UNICAST BIT(13)

#define ATBM_WPA_PROTO_WPA BIT(0)
#define ATBM_WPA_PROTO_RSN BIT(1)
#define ATBM_WPA_PROTO_WAPI BIT(2)

#define ATBM_WPA_CIPHER_NONE BIT(0)
#define ATBM_WPA_CIPHER_WEP40 BIT(1)
#define ATBM_WPA_CIPHER_WEP104 BIT(2)
#define ATBM_WPA_CIPHER_TKIP BIT(3)
#define ATBM_WPA_CIPHER_CCMP BIT(4)
#define ATBM_WPA_CIPHER_AES_128_CMAC BIT(5)
#define ATBM_WPA_CIPHER_GCMP BIT(6)
#define ATBM_WPA_CIPHER_SMS4 BIT(7)


#define ATBM_WPA_KEY_MGMT_IEEE8021X BIT(0)
#define ATBM_WPA_KEY_MGMT_PSK BIT(1)
#define ATBM_WPA_KEY_MGMT_NONE BIT(2)
#define ATBM_WPA_KEY_MGMT_IEEE8021X_NO_WPA BIT(3)
#define ATBM_WPA_KEY_MGMT_WPA_NONE BIT(4)
#define ATBM_WPA_KEY_MGMT_FT_IEEE8021X BIT(5)
#define ATBM_WPA_KEY_MGMT_FT_PSK BIT(6)
#define ATBM_WPA_KEY_MGMT_IEEE8021X_SHA256 BIT(7)
#define ATBM_WPA_KEY_MGMT_PSK_SHA256 BIT(8)
#define ATBM_WPA_KEY_MGMT_WPS BIT(9)
#define ATBM_WPA_KEY_MGMT_SAE BIT(10)
#define ATBM_WPA_KEY_MGMT_FT_SAE BIT(11)
#define ATBM_WPA_KEY_MGMT_WAPI_PSK BIT(12)
#define ATBM_WPA_KEY_MGMT_WAPI_CERT BIT(13)
#define ATBM_WPA_KEY_MGMT_CCKM BIT(14)

#define ATBM_OUI_MICROSOFT 0x0050f2 /* Microsoft (also used in Wi-Fi specs)*/
#define ATBM_WMM_OUI_TYPE  2
#define ATBM_WMM_OUI_SUBTYPE_INFORMATION_ELEMENT 0
#define ATBM_WMM_OUI_SUBTYPE_PARAMETER_ELEMENT 1
#define ATBM_WMM_OUI_SUBTYPE_TSPEC_ELEMENT 2

#define ATBM_OUI_WFA 0x506f9a
#define ATBM_P2P_IE_VENDOR_TYPE 0x506f9a09
#define ATBM_WFD_IE_VENDOR_TYPE 0x506f9a0a
#define ATBM_WFD_OUI_TYPE 10
#define ATBM_HS20_IE_VENDOR_TYPE 0x506f9a10
#define ATBM_HS20_INDICATION_OUI_TYPE 16

#define ATBM_P2P_OUI_TYPE 9
#define ATBM_OUI_BROADCOM 0x00904c /* Broadcom (Epigram) */

#define ATBM_WPA_SELECTOR_LEN 4
#define ATBM_WPA_VERSION 1
#define ATBM_RSN_SELECTOR_LEN 4
#define ATBM_RSN_VERSION 1

static inline u32 ATBM_WPA_GET_BE24(const u8 *a)
{
	return (a[0] << 16) | (a[1] << 8) | a[2];
}

static inline u32 ATBM_WPA_GET_BE32(const u8 *a)
{
	return (a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3];
}

static inline u16 ATBM_WPA_GET_LE16(const u8 *a)
{
	return (a[1] << 8) | a[0];
}

#define ATBM_RSN_CIPHER_SUITE_NONE 					0x000fac00
#define ATBM_RSN_CIPHER_SUITE_WEP40					0x000fac01
#define ATBM_RSN_CIPHER_SUITE_TKIP 					0x000fac02
#define ATBM_RSN_CIPHER_SUITE_CCMP 					0x000fac04
#define ATBM_RSN_CIPHER_SUITE_WEP104 				0x000fac05
#define ATBM_RSN_CIPHER_SUITE_AES_128_CMAC 			0x000fac06
#define ATBM_RSN_CIPHER_SUITE_NO_GROUP_ADDRESSED 	0x000fac07
#define ATBM_RSN_CIPHER_SUITE_GCMP 					0x000fac08


#define ATBM_RSN_AUTH_KEY_MGMT_UNSPEC_802_1X 		0x000fac01
#define ATBM_RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X 		0x000fac02
#define ATBM_RSN_AUTH_KEY_MGMT_FT_802_1X 			0x000fac03
#define ATBM_RSN_AUTH_KEY_MGMT_FT_PSK 				0x000fac04
#define ATBM_RSN_AUTH_KEY_MGMT_802_1X_SHA256 		0x000fac05
#define ATBM_RSN_AUTH_KEY_MGMT_PSK_SHA256 			0x000fac06
#define ATBM_RSN_AUTH_KEY_MGMT_TPK_HANDSHAKE 		0x000fac07
#define ATBM_RSN_AUTH_KEY_MGMT_SAE 					0x000fac08
#define ATBM_RSN_AUTH_KEY_MGMT_FT_SAE				0x000fac09
#define ATBM_RSN_AUTH_KEY_MGMT_CCKM 				0x00409600

/* AKM suite selectors */
#define ATBM_WLAN_AKM_SUITE_8021X					0x000FAC01
#define ATBM_WLAN_AKM_SUITE_PSK						0x000FAC02
#define ATBM_WLAN_AKM_SUITE_8021X_SHA256			0x000FAC05
#define ATBM_WLAN_AKM_SUITE_PSK_SHA256				0x000FAC06
#define ATBM_WLAN_AKM_SUITE_TDLS					0x000FAC07
#define ATBM_WLAN_AKM_SUITE_SAE						0x000FAC08
#define ATBM_WLAN_AKM_SUITE_FT_OVER_SAE				0x000FAC09
#define ATBM_WLAN_AKM_SUITE_802_1X_SUITE_B_192 		0x000fac12
#define ATBM_WLAN_AKM_SUITE_FILS_SHA256 			0x000fac14
#define ATBM_WLAN_AKM_SUITE_FILS_SHA384 			0x000fac15
#define ATBM_WLAN_AKM_SUITE_FT_FILS_SHA256 			0x000fac16
#define ATBM_WLAN_AKM_SUITE_FT_FILS_SHA384 			0x000fac17

#define ATBM_PMKID_LEN 16
#define ATBM_PMK_LEN 32
#define ATBM_WPA_REPLAY_COUNTER_LEN 8
#define ATBM_WPA_NONCE_LEN 32
#define ATBM_WPA_KEY_RSC_LEN 8
#define ATBM_WPA_GMK_LEN 32
#define ATBM_WPA_GTK_MAX_LEN 32


#define ATBM_WPA_AUTH_KEY_MGMT_NONE 				0x0050f200
#define ATBM_WPA_AUTH_KEY_MGMT_UNSPEC_802_1X 		0x0050f201
#define ATBM_WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X 		0x0050f202
#define ATBM_WPA_AUTH_KEY_MGMT_CCKM 				0x00409600
#define ATBM_WPA_CIPHER_SUITE_NONE 					0x0050f200
#define ATBM_WPA_CIPHER_SUITE_WEP40 				0x0050f201
#define ATBM_WPA_CIPHER_SUITE_TKIP 					0x0050f202
#define ATBM_WPA_CIPHER_SUITE_CCMP 					0x0050f204
#define ATBM_WPA_CIPHER_SUITE_WEP104 				0x0050f205

#define ATBM_PRIVATE_OUI							0x4154424D

#define ATBM_MICHAEL_MIC_LEN 8

enum atbm_p2p_attr_id {
	ATBM_P2P_ATTR_STATUS = 0,
	ATBM_P2P_ATTR_MINOR_REASON_CODE = 1,
	ATBM_P2P_ATTR_CAPABILITY = 2,
	ATBM_P2P_ATTR_DEVICE_ID = 3,
	ATBM_P2P_ATTR_GROUP_OWNER_INTENT = 4,
	ATBM_P2P_ATTR_CONFIGURATION_TIMEOUT = 5,
	ATBM_P2P_ATTR_LISTEN_CHANNEL = 6,
	ATBM_P2P_ATTR_GROUP_BSSID = 7,
	ATBM_P2P_ATTR_EXT_LISTEN_TIMING = 8,
	ATBM_P2P_ATTR_INTENDED_INTERFACE_ADDR = 9,
	ATBM_P2P_ATTR_MANAGEABILITY = 10,
	ATBM_P2P_ATTR_CHANNEL_LIST = 11,
	ATBM_P2P_ATTR_NOTICE_OF_ABSENCE = 12,
	ATBM_P2P_ATTR_DEVICE_INFO = 13,
	ATBM_P2P_ATTR_GROUP_INFO = 14,
	ATBM_P2P_ATTR_GROUP_ID = 15,
	ATBM_P2P_ATTR_INTERFACE = 16,
	ATBM_P2P_ATTR_OPERATING_CHANNEL = 17,
	ATBM_P2P_ATTR_INVITATION_FLAGS = 18,
	ATBM_P2P_ATTR_VENDOR_SPECIFIC = 221
};

/* P2P public action frames */
enum atbm_p2p_action_frame_type {
	ATBM_P2P_GO_NEG_REQ = 0,
	ATBM_P2P_GO_NEG_RESP = 1,
	ATBM_P2P_GO_NEG_CONF = 2,
	ATBM_P2P_INVITATION_REQ = 3,
	ATBM_P2P_INVITATION_RESP = 4,
	ATBM_P2P_DEV_DISC_REQ = 5,
	ATBM_P2P_DEV_DISC_RESP = 6,
	ATBM_P2P_PROV_DISC_REQ = 7,
	ATBM_P2P_PROV_DISC_RESP = 8
};
struct atbm_p2p_message {
	u8 dialog_token;

	u8 *capability;
	u8 *go_intent;
	u8 *status;
	u8 *operating_channel;
	u8 *channel_list;
	u8 channel_list_len;
	
	u8 *group_bssid;
	u8 *invitation_flags;

	u8 *group_info;
	size_t group_info_len;

	u8 *group_id;
	size_t group_id_len;

	u8 *device_id;

	u8 *manageability;
	u8 *intended_addr;
};

struct atbm_ieee80211_vendor_ie {
	u8 element_id;
	u8 len;
	u8 oui[3];
	u8 oui_type;
} __packed;


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
struct beacon_parameters {
	const u8 *head, *tail;
	int interval, dtim_period;
	int head_len, tail_len;
	const u8 *ssid;
	size_t ssid_len;
	enum nl80211_hidden_ssid hidden_ssid;
	struct cfg80211_crypto_settings crypto;
	bool privacy;
	enum nl80211_auth_type auth_type;
	const u8 *beacon_ies;
	size_t beacon_ies_len;
	const u8 *proberesp_ies;
	size_t proberesp_ies_len;
	const u8 *assocresp_ies;
	size_t assocresp_ies_len;
};
#endif

#ifdef ATBM_SUPPORT_WIDTH_40M

/**
 *
 *
 *
 */
struct atbm_ieee80211_ext_chansw_ie{
	u8 mode;
	u8 new_operaring_class;
	u8 new_ch_num;
	u8 count;
} __attribute__ ((packed));

struct atbm_ieee80211_sec_chan_offs_ie {
	u8 sec_chan_offs;
} __attribute__ ((packed));


struct atbm_ieee80211_channel_sw_packed_ie{
	struct ieee80211_channel_sw_ie *chan_sw_ie;
	struct atbm_ieee80211_ext_chansw_ie *ex_chan_sw_ie;
	struct atbm_ieee80211_sec_chan_offs_ie *sec_chan_offs_ie;
}__attribute__ ((packed));


/* Public action codes */
enum atbm_ieee80211_pub_actioncode {
	ATBM_WLAN_PUB_ACTION_EX_CHL_SW_ANNOUNCE = 4,
};

#endif
struct atbm_ieee80211_mgmt {
	__le16 frame_control;
	__le16 duration;
	u8 da[6];
	u8 sa[6];
	u8 bssid[6];
	__le16 seq_ctrl;
	union {
		struct {
			__le16 auth_alg;
			__le16 auth_transaction;
			__le16 status_code;
			/* possibly followed by Challenge text */
			u8 variable[0];
		} __attribute__ ((packed)) auth;
		struct {
			__le16 reason_code;
		} __attribute__ ((packed)) deauth;
		struct {
			__le16 capab_info;
			__le16 listen_interval;
			/* followed by SSID and Supported rates */
			u8 variable[0];
		} __attribute__ ((packed)) assoc_req;
		struct {
			__le16 capab_info;
			__le16 status_code;
			__le16 aid;
			/* followed by Supported rates */
			u8 variable[0];
		} __attribute__ ((packed)) assoc_resp, reassoc_resp;
		struct {
			__le16 capab_info;
			__le16 listen_interval;
			u8 current_ap[6];
			/* followed by SSID and Supported rates */
			u8 variable[0];
		} __attribute__ ((packed)) reassoc_req;
		struct {
			__le16 reason_code;
		} __attribute__ ((packed)) disassoc;
		struct {
			__le64 timestamp;
			__le16 beacon_int;
			__le16 capab_info;
			/* followed by some of SSID, Supported rates,
			 * FH Params, DS Params, CF Params, IBSS Params, TIM */
			u8 variable[0];
		} __attribute__ ((packed)) beacon;
		struct {
			/* only variable items: SSID, Supported rates */
			u8 variable[0];
		} __attribute__ ((packed)) probe_req;
		struct {
			__le64 timestamp;
			__le16 beacon_int;
			__le16 capab_info;
			/* followed by some of SSID, Supported rates,
			 * FH Params, DS Params, CF Params, IBSS Params */
			u8 variable[0];
		} __attribute__ ((packed)) probe_resp;
		struct {
			u8 category;
			union {
				struct {
					u8 action; /* 9 */
					u8 oui[3];
					/* Vendor-specific content */
					u8 variable[0];
				}  __attribute__ ((packed)) vs_public_action;
				struct {
					u8 action_code;
					u8 dialog_token;
					u8 status_code;
					u8 variable[0];
				} __attribute__ ((packed)) wme_action;
				struct{
					u8 action_code;
					u8 element_id;
					u8 length;
					struct ieee80211_channel_sw_ie sw_elem;
				} __attribute__((packed)) chan_switch;
				#ifdef ATBM_SUPPORT_WIDTH_40M
				struct{
					u8	action_code;
					struct atbm_ieee80211_ext_chansw_ie ext_sw_elem;
					u8 variable[0];
				}__attribute__((packed)) ext_chan_switch;
				struct{
					u8 action_code;
					u8 chan_width;
				}__attribute__((packed)) notify_chan_width;
				#endif
				struct{
					u8 action_code;
					u8 dialog_token;
					u8 element_id;
					u8 length;
					struct ieee80211_msrment_ie msr_elem;
				} __attribute__((packed)) measurement;
				struct{
					u8 action_code;
					u8 dialog_token;
					__le16 capab;
					__le16 timeout;
					__le16 start_seq_num;
				} __attribute__((packed)) addba_req;
				struct{
					u8 action_code;
					u8 dialog_token;
					__le16 status;
					__le16 capab;
					__le16 timeout;
				} __attribute__((packed)) addba_resp;
				struct{
					u8 action_code;
					__le16 params;
					__le16 reason_code;
				} __attribute__((packed)) delba;
				struct {
					u8 action_code;
					u8 variable[0];
				} __attribute__((packed)) self_prot;
				struct{
					u8 action_code;
					u8 variable[0];
				} __attribute__((packed)) mesh_action;
				struct {
					u8 action;
					u8 trans_id[WLAN_SA_QUERY_TR_ID_LEN];
				} __attribute__ ((packed)) sa_query;
				struct {
					u8 action;
					u8 smps_control;
				} __attribute__ ((packed)) ht_smps;
				struct {
					u8 action_code;
					u8 dialog_token;
					__le16 capability;
					u8 variable[0];
				} __packed tdls_discover_resp;
			} u;
		} __attribute__ ((packed)) action;
	} u;
} __attribute__ ((packed));

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 1, 0))
struct ieee80211_tdls_lnkie {
	u8 ie_type; /* Link Identifier IE */
	u8 ie_len;
	u8 bssid[6];
	u8 init_sta[6];
	u8 resp_sta[6];
} __packed;

struct ieee80211_tdls_data {
	u8 da[6];
	u8 sa[6];
	__be16 ether_type;
	u8 payload_type;
	u8 category;
	u8 action_code;
	union {
		struct {
			u8 dialog_token;
			__le16 capability;
			u8 variable[0];
		} __packed setup_req;
		struct {
			__le16 status_code;
			u8 dialog_token;
			__le16 capability;
			u8 variable[0];
		} __packed setup_resp;
		struct {
			__le16 status_code;
			u8 dialog_token;
			u8 variable[0];
		} __packed setup_cfm;
		struct {
			__le16 reason_code;
			u8 variable[0];
		} __packed teardown;
		struct {
			u8 dialog_token;
			u8 variable[0];
		} __packed discover_req;
	} u;
} __packed;
/* TDLS action codes */
enum ieee80211_tdls_actioncode {
	WLAN_TDLS_SETUP_REQUEST = 0,
	WLAN_TDLS_SETUP_RESPONSE = 1,
	WLAN_TDLS_SETUP_CONFIRM = 2,
	WLAN_TDLS_TEARDOWN = 3,
	WLAN_TDLS_PEER_TRAFFIC_INDICATION = 4,
	WLAN_TDLS_CHANNEL_SWITCH_REQUEST = 5,
	WLAN_TDLS_CHANNEL_SWITCH_RESPONSE = 6,
	WLAN_TDLS_PEER_PSM_REQUEST = 7,
	WLAN_TDLS_PEER_PSM_RESPONSE = 8,
	WLAN_TDLS_PEER_TRAFFIC_RESPONSE = 9,
	WLAN_TDLS_DISCOVERY_REQUEST = 10,
};
/* Self Protected Action codes */
enum ieee80211_self_protected_actioncode {
	WLAN_SP_RESERVED = 0,
	WLAN_SP_MESH_PEERING_OPEN = 1,
	WLAN_SP_MESH_PEERING_CONFIRM = 2,
	WLAN_SP_MESH_PEERING_CLOSE = 3,
	WLAN_SP_MGK_INFORM = 4,
	WLAN_SP_MGK_ACK = 5,
};
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0))
static inline bool ieee80211_is_public_action(struct ieee80211_hdr *hdr,
					      size_t len)
{
	struct atbm_ieee80211_mgmt *mgmt = (void *)hdr;

	if (len < IEEE80211_MIN_ACTION_SIZE)
		return false;
	if (!ieee80211_is_action(hdr->frame_control))
		return false;
	return mgmt->u.action.category == ATBM_WLAN_CATEGORY_PUBLIC;
}
#endif

#ifndef WLAN_EXT_CAPA5_TDLS_ENABLED
#define WLAN_EXT_CAPA5_TDLS_ENABLED	BIT(5)
#endif
#ifndef WLAN_TDLS_SNAP_RFTYPE
#define WLAN_TDLS_SNAP_RFTYPE	 0x2
#endif
/**
 * DOC: Introduction
 *
 * mac80211 is the Linux stack for 802.11 hardware that implements
 * only partial functionality in hard- or firmware. This document
 * defines the interface between mac80211 and low-level hardware
 * drivers.
 */

/**
 * DOC: Calling mac80211 from interrupts
 *
 * Only ieee80211_tx_status_irqsafe() and ieee80211_rx_irqsafe() can be
 * called in hardware interrupt context. The low-level driver must not call any
 * other functions in hardware interrupt context. If there is a need for such
 * call, the low-level driver should first ACK the interrupt and perform the
 * IEEE 802.11 code call after this, e.g. from a scheduled workqueue or even
 * tasklet function.
 *
 * NOTE: If the driver opts to use the _irqsafe() functions, it may not also
 *	 use the non-IRQ-safe functions!
 */

/**
 * DOC: Warning
 *
 * If you're reading this document and not the header file itself, it will
 * be incomplete because not all documentation has been converted yet.
 */

/**
 * DOC: Frame format
 *
 * As a general rule, when frames are passed between mac80211 and the driver,
 * they start with the IEEE 802.11 header and include the same octets that are
 * sent over the air except for the FCS which should be calculated by the
 * hardware.
 *
 * There are, however, various exceptions to this rule for advanced features:
 *
 * The first exception is for hardware encryption and decryption offload
 * where the IV/ICV may or may not be generated in hardware.
 *
 * Secondly, when the hardware handles fragmentation, the frame handed to
 * the driver from mac80211 is the MSDU, not the MPDU.
 *
 * Finally, for received frames, the driver is able to indicate that it has
 * filled a radiotap header and put that in front of the frame; if it does
 * not do so then mac80211 may add this under certain circumstances.
 */

/**
 * DOC: mac80211 workqueue
 *
 * mac80211 provides its own workqueue for drivers and internal mac80211 use.
 * The workqueue is a single threaded workqueue and can only be accessed by
 * helpers for sanity checking. Drivers must ensure all work added onto the
 * mac80211 workqueue should be cancelled on the driver stop() callback.
 *
 * mac80211 will flushed the workqueue upon interface removal and during
 * suspend.
 *
 * All work performed on the mac80211 workqueue must not acquire the RTNL lock.
 *
 */

/**
 * enum ieee80211_max_queues - maximum number of queues
 *
 * @IEEE80211_MAX_QUEUES: Maximum number of regular device queues.
 */
enum ieee80211_max_queues {
	IEEE80211_MAX_QUEUES =		16,
};

#define IEEE80211_INVAL_HW_QUEUE	0xff

/**
 * enum ieee80211_ac_numbers - AC numbers as used in mac80211
 * @IEEE80211_AC_VO: voice
 * @IEEE80211_AC_VI: video
 * @IEEE80211_AC_BE: best effort
 * @IEEE80211_AC_BK: background
 */
enum ieee80211_ac_numbers {
	IEEE80211_AC_VO		= 0,
	IEEE80211_AC_VI		= 1,
	IEEE80211_AC_BE		= 2,
	IEEE80211_AC_BK		= 3,
};
#define IEEE80211_NUM_ACS	4

/**
 * struct ieee80211_tx_queue_params - transmit queue configuration
 *
 * The information provided in this structure is required for QoS
 * transmit queue configuration. Cf. IEEE 802.11 7.3.2.29.
 *
 * @aifs: arbitration interframe space [0..255]
 * @cw_min: minimum contention window [a value of the form
 *	2^n-1 in the range 1..32767]
 * @cw_max: maximum contention window [like @cw_min]
 * @txop: maximum burst time in units of 32 usecs, 0 meaning disabled
 * @uapsd: is U-APSD mode enabled for the queue
 */
struct ieee80211_tx_queue_params {
	u16 txop;
	u16 cw_min;
	u16 cw_max;
	u8 aifs;
	bool uapsd;
};

struct ieee80211_low_level_stats {
	unsigned int dot11ACKFailureCount;
	unsigned int dot11RTSFailureCount;
	unsigned int dot11FCSErrorCount;
	unsigned int dot11RTSSuccessCount;
};

/**
 * enum ieee80211_bss_change - BSS change notification flags
 *
 * These flags are used with the bss_info_changed() callback
 * to indicate which BSS parameter changed.
 *
 * @BSS_CHANGED_ASSOC: association status changed (associated/disassociated),
 *	also implies a change in the AID.
 * @BSS_CHANGED_ERP_CTS_PROT: CTS protection changed
 * @BSS_CHANGED_ERP_PREAMBLE: preamble changed
 * @BSS_CHANGED_ERP_SLOT: slot timing changed
 * @BSS_CHANGED_HT: 802.11n parameters changed
 * @BSS_CHANGED_BASIC_RATES: Basic rateset changed
 * @BSS_CHANGED_BEACON_INT: Beacon interval changed
 * @BSS_CHANGED_BSSID: BSSID changed, for whatever
 *	reason (IBSS and managed mode)
 * @BSS_CHANGED_BEACON: Beacon data changed, retrieve
 *	new beacon (beaconing modes)
 * @BSS_CHANGED_BEACON_ENABLED: Beaconing should be
 *	enabled/disabled (beaconing modes)
 * @BSS_CHANGED_CQM: Connection quality monitor config changed
 * @BSS_CHANGED_IBSS: IBSS join status changed
 * @BSS_CHANGED_ARP_FILTER: Hardware ARP filter address list or state changed.
 * @BSS_CHANGED_QOS: QoS for this association was enabled/disabled. Note
 *	that it is only ever disabled for station mode.
 * @BSS_CHANGED_IDLE: Idle changed for this BSS/interface.
 * @BSS_CHANGED_SSID: SSID changed for this BSS (AP mode)
 * @BSS_CHANGED_PS: the PS flag or dynamic PS timeout changed
 * @BSS_CHANGED_CHANNEL: vif channel changed
 * @BSS_CHANGED_P2P_PS: NOA parameters changed.
 */
enum ieee80211_bss_change {
	BSS_CHANGED_ASSOC		= 1<<0,
	BSS_CHANGED_ERP_CTS_PROT	= 1<<1,
	BSS_CHANGED_ERP_PREAMBLE	= 1<<2,
	BSS_CHANGED_ERP_SLOT		= 1<<3,
	BSS_CHANGED_HT			= 1<<4,
	BSS_CHANGED_BASIC_RATES		= 1<<5,
	BSS_CHANGED_BEACON_INT		= 1<<6,
	BSS_CHANGED_BSSID		= 1<<7,
	BSS_CHANGED_BEACON		= 1<<8,
	BSS_CHANGED_BEACON_ENABLED	= 1<<9,
	BSS_CHANGED_CQM			= 1<<10,
	BSS_CHANGED_IBSS		= 1<<11,
	BSS_CHANGED_ARP_FILTER		= 1<<12,
	BSS_CHANGED_QOS			= 1<<13,
	BSS_CHANGED_IDLE		= 1<<14,
	BSS_CHANGED_SSID		= 1<<15,
	BSS_CHANGED_PS			= 1<<16,
	BSS_CHANGED_CHANNEL		= 1<<17, // XXX: COMBO: should this be merged with _HT?
	BSS_CHANGED_RETRY_LIMITS	= 1<<18,
	BSS_CHANGED_P2P_PS		= 1<<19,
#ifdef IPV6_FILTERING
	BSS_CHANGED_NDP_FILTER		= 1<<20,
#endif /*IPV6_FILTERING*/
	BSS_CHANGED_STA_RESTART		= 1<<21,
	BSS_CHANGED_STA_DTIM		= 1<<22,
	BSS_CHANGED_HT_CHANNEL_TYPE = 1<<23,
	/* when adding here, make sure to change ieee80211_reconfig */
};

/*
 * The maximum number of IPv4 addresses listed for ARP filtering. If the number
 * of addresses for an interface increase beyond this value, hardware ARP
 * filtering will be disabled.
 */
#define IEEE80211_BSS_ARP_ADDR_LIST_LEN 4

#ifdef IPV6_FILTERING
/*
 * The maximum number of IPv6 addresses listed for NDP filtering. If the number
 * of addresses for an interface increase beyond this value, hardware NDP
 * filtering will be disabled.
 */
#define IEEE80211_BSS_NDP_ADDR_LIST_LEN 4
#endif /*IPV6_FILTERING*/

/**
 * enum ieee80211_rssi_event - RSSI threshold event
 * An indicator for when RSSI goes below/above a certain threshold.
 * @RSSI_EVENT_HIGH: AP's rssi crossed the high threshold set by the driver.
 * @RSSI_EVENT_LOW: AP's rssi crossed the low threshold set by the driver.
 */
enum ieee80211_rssi_event {
	RSSI_EVENT_HIGH,
	RSSI_EVENT_LOW,
};

/**
 * ieee80211_channel_conf - channel configuration
 *
 * @channel: the channel to tune to
 * @channel_type: the channel (HT) type
 * @offchannel: The device is currently not on its main operating channel.
 */
struct ieee80211_channel_conf {
	struct ieee80211_channel *channel;
	enum nl80211_channel_type channel_type;
	bool offchannel;
};
/**
 * struct cfg80211_p2p_ps - p2p power save params
 *
 * This structure ...
 *
 * @legacy_ps: 0=disable, 1=enable, 2=maximum_ps, -1=no change
 * @opp_ps: 0=disable, 1=enable, -1=no change
 * @ctwidnow: 0... - change in ms, -1=no change
 * @count: 0..255 - count
 * @start: Start time in ms from next TBTT
 * @duration: Duration in ms
 * @interval: interval in ms
 */
struct cfg80211_p2p_ps {
	int	legacy_ps;

	/* Opportunistic Power Save */
	int	opp_ps;
	int	ctwindow;

	/* Notice of Absence */
	int	count;
	int	start;
	int	duration;
	int	interval;

	/* Index */
	u8	index;
};

/**
 * struct ieee80211_bss_conf - holds the BSS's changing parameters
 *
 * This structure keeps information about a BSS (and an association
 * to that BSS) that can change during the lifetime of the BSS.
 *
 * @assoc: association status
 * @ibss_joined: indicates whether this station is part of an IBSS
 *	or not
 * @aid: association ID number, valid only when @assoc is true
 * @use_cts_prot: use CTS protection
 * @use_short_preamble: use 802.11b short preamble;
 *	if the hardware cannot handle this it must set the
 *	IEEE80211_HW_2GHZ_SHORT_PREAMBLE_INCAPABLE hardware flag
 * @use_short_slot: use short slot time (only relevant for ERP);
 *	if the hardware cannot handle this it must set the
 *	IEEE80211_HW_2GHZ_SHORT_SLOT_INCAPABLE hardware flag
 * @dtim_period: num of beacons before the next DTIM, for beaconing,
 *	valid in station mode only while @assoc is true and if also
 *	requested by %IEEE80211_HW_NEED_DTIM_PERIOD (cf. also hw conf
 *	@ps_dtim_period)
 * @timestamp: beacon timestamp
 * @beacon_int: beacon interval
 * @assoc_capability: capabilities taken from assoc resp
 * @basic_rates: bitmap of basic rates, each bit stands for an
 *	index into the rate table configured by the driver in
 *	the current band.
 * @mcast_rate: per-band multicast rate index + 1 (0: disabled)
 * @bssid: The BSSID for this BSS
 * @enable_beacon: whether beaconing should be enabled or not
 * @channel_type: Channel type for this BSS -- the hardware might be
 *	configured for HT40+ while this BSS only uses no-HT, for
 *	example.
 * @ht_operation_mode: HT operation mode (like in &struct ieee80211_ht_info).
 *	This field is only valid when the channel type is one of the HT types.
 * @cqm_rssi_thold: Connection quality monitor RSSI threshold, a zero value
 *	implies disabled
 * @cqm_rssi_hyst: Connection quality monitor RSSI hysteresis
 * @cqm_beacon_miss_thold: Connection quality monitor beacon threshold, a zero
 *	value implies disabled
 * @cqm_tx_fail_thold: Connection quality monitor tx threshold, a zero value
 *	implies disabled
 * @arp_addr_list: List of IPv4 addresses for hardware ARP filtering. The
 *	may filter ARP queries targeted for other addresses than listed here.
 *	The driver must allow ARP queries targeted for all address listed here
 *	to pass through. An empty list implies no ARP queries need to pass.
 * @arp_addr_cnt: Number of addresses currently on the list.
 * @arp_filter_enabled: Enable ARP filtering - if enabled, the hardware may
 *	filter ARP queries based on the @arp_addr_list, if disabled, the
 *	hardware must not perform any ARP filtering. Note, that the filter will
 *	be enabled also in promiscuous mode.
 * @qos: This is a QoS-enabled BSS.
 * @idle: This interface is idle. There's also a global idle flag in the
 *	hardware config which may be more appropriate depending on what
 *	your driver/device needs to do.
 * @ps_enabled: Enable 802.11 power save mode (managed mode only). This is the power
 *	save mode defined by IEEE 802.11-2007 section 11.2, meaning that the
 *	hardware still wakes up for beacons, is able to transmit frames and
 *	receive the possible acknowledgment frames. Not to be confused with
 *	hardware specific wakeup/sleep states, driver is responsible for that.
 *	See the section "Powersave support" for more.
 * @listen_interval: listen interval in units of beacon interval
 * @max_sleep_period: the maximum number of beacon intervals to sleep for
 *	before checking the beacon for a TIM bit (managed mode only); this
 *	value will be only achievable between DTIM frames, the hardware
 *	needs to check for the multicast traffic bit in DTIM beacons.
 *	This variable is valid only when the ps_enabled variable is true.
 * @ps_dtim_period: The DTIM period of the AP we're connected to, for use
 *	in power saving. Power saving will not be enabled until a beacon
 *	has been received and the DTIM period is known.
 * @dynamic_ps_timeout: The dynamic powersave timeout (in ms), see the
 *	powersave documentation below. This variable is valid only when
 *	the ps_enabled variable is true.
 * @ssid: The SSID of the current vif. Only valid in AP-mode.
 * @ssid_len: Length of SSID given in @ssid.
 * @hidden_ssid: The SSID of the current vif is hidden. Only valid in AP-mode.
 * @chan_conf: current channel configuration (i.e. channel, channel type)
 * @ndp_addr_list: List of IPv6 addresses for hardware NDP filtering. The
 *	may filter NDP queries targeted for other addresses than listed here.
 *	The driver must allow NDP queries targeted for all address listed here
 *	to pass through. An empty list implies no NDP queries need to pass.
 * @ndp_addr_cnt: Number of addresses currently on the list.
 * @ndp_filter_enabled: Enable NDP filtering - if enabled, the hardware may
 *	filter NDP queries based on the @ndp_addr_list, if disabled, the
 *	hardware must not perform any NDP filtering. Note, that the filter will
 *	be enabled also in promiscuous mode.
 */
struct ieee80211_bss_conf {
	const u8 *bssid;
	/* association related data */
	bool assoc, ibss_joined;
	u16 aid;
	/* erp related data */
	bool use_cts_prot;
	bool use_short_preamble;
	bool use_short_slot;
	bool enable_beacon;
	u8 dtim_period;
	u16 beacon_int;
	u16 assoc_capability;
	u64 timestamp;
	u32 basic_rates;
	int mcast_rate[IEEE80211_NUM_BANDS];
	u16 ht_operation_mode;
	s32 cqm_rssi_thold;
	u32 cqm_rssi_hyst;
	u32 cqm_beacon_miss_thold;
	u32 cqm_tx_fail_thold;
	enum nl80211_channel_type channel_type;
	enum nl80211_auth_type auth_type;
	u32 auth_type_bit;
	__be32 arp_addr_list[IEEE80211_BSS_ARP_ADDR_LIST_LEN];
	u8 arp_addr_cnt;
	bool arp_filter_enabled;
	bool qos;
	bool idle;
	bool ps_enabled;
	int dynamic_ps_timeout;
	int max_sleep_period;
	int listen_interval;
	int ps_dtim_period;
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	size_t ssid_len;
	bool hidden_ssid;
	struct ieee80211_channel_conf *chan_conf;
	u8 retry_short;
	u8 retry_long;
	u32 rts_threshold;
	struct cfg80211_p2p_ps p2p_ps;

#ifdef IPV6_FILTERING
	struct in6_addr ndp_addr_list[IEEE80211_BSS_NDP_ADDR_LIST_LEN];
	u8 ndp_addr_cnt;
	bool ndp_filter_enabled;
#endif /*IPV6_FILTERING*/
#ifdef ATBM_AP_SME
	struct atbm_wpa_ie_data wpa_ap_data;
	u16 aid_map;
#endif
	bool privacy;
	u8 num_sta_no_short_preamble;
	u8 num_sta_no_short_slot_time;
};

/**
 * enum mac80211_tx_control_flags - flags to describe transmission information/status
 *
 * These flags are used with the @flags member of &ieee80211_tx_info.
 *
 * @IEEE80211_TX_CTL_REQ_TX_STATUS: require TX status callback for this frame.
 * @IEEE80211_TX_CTL_ASSIGN_SEQ: The driver has to assign a sequence
 *	number to this frame, taking care of not overwriting the fragment
 *	number and increasing the sequence number only when the
 *	IEEE80211_TX_CTL_FIRST_FRAGMENT flag is set. mac80211 will properly
 *	assign sequence numbers to QoS-data frames but cannot do so correctly
 *	for non-QoS-data and management frames because beacons need them from
 *	that counter as well and mac80211 cannot guarantee proper sequencing.
 *	If this flag is set, the driver should instruct the hardware to
 *	assign a sequence number to the frame or assign one itself. Cf. IEEE
 *	802.11-2007 7.1.3.4.1 paragraph 3. This flag will always be set for
 *	beacons and always be clear for frames without a sequence number field.
 * @IEEE80211_TX_CTL_NO_ACK: tell the low level not to wait for an ack
 * @IEEE80211_TX_CTL_CLEAR_PS_FILT: clear powersave filter for destination
 *	station
 * @IEEE80211_TX_CTL_FIRST_FRAGMENT: this is a first fragment of the frame
 * @IEEE80211_TX_CTL_SEND_AFTER_DTIM: send this frame after DTIM beacon
 * @IEEE80211_TX_CTL_AMPDU: this frame should be sent as part of an A-MPDU
 * @IEEE80211_TX_CTL_INJECTED: Frame was injected, internal to mac80211.
 * @IEEE80211_TX_STAT_TX_FILTERED: The frame was not transmitted
 *	because the destination STA was in powersave mode. Note that to
 *	avoid race conditions, the filter must be set by the hardware or
 *	firmware upon receiving a frame that indicates that the station
 *	went to sleep (must be done on device to filter frames already on
 *	the queue) and may only be unset after mac80211 gives the OK for
 *	that by setting the IEEE80211_TX_CTL_CLEAR_PS_FILT (see above),
 *	since only then is it guaranteed that no more frames are in the
 *	hardware queue.
 * @IEEE80211_TX_STAT_ACK: Frame was acknowledged
 * @IEEE80211_TX_STAT_AMPDU: The frame was aggregated, so status
 * 	is for the whole aggregation.
 * @IEEE80211_TX_STAT_AMPDU_NO_BACK: no block ack was returned,
 * 	so consider using block ack request (BAR).
 * @IEEE80211_TX_CTL_RATE_CTRL_PROBE: internal to mac80211, can be
 *	set by rate control algorithms to indicate probe rate, will
 *	be cleared for fragmented frames (except on the last fragment)
 * @IEEE80211_TX_INTFL_NEED_TXPROCESSING: completely internal to mac80211,
 *	used to indicate that a pending frame requires TX processing before
 *	it can be sent out.
 * @IEEE80211_TX_INTFL_RETRIED: completely internal to mac80211,
 *	used to indicate that a frame was already retried due to PS
 * @IEEE80211_TX_INTFL_DONT_ENCRYPT: completely internal to mac80211,
 *	used to indicate frame should not be encrypted
 * @IEEE80211_TX_CTL_POLL_RESPONSE: This frame is a response to a poll
 *	frame (PS-Poll or uAPSD) and should be sent although the station
 *	is in powersave mode.
 * @IEEE80211_TX_CTL_MORE_FRAMES: More frames will be passed to the
 *	transmit function after the current frame, this can be used
 *	by drivers to kick the DMA queue only if unset or when the
 *	queue gets full.
 * @IEEE80211_TX_INTFL_RETRANSMISSION: This frame is being retransmitted
 *	after TX status because the destination was asleep, it must not
 *	be modified again (no seqno assignment, crypto, etc.)
 * @IEEE80211_TX_INTFL_NL80211_FRAME_TX: Frame was requested through nl80211
 *	MLME command (internal to mac80211 to figure out whether to send TX
 *	status to user space)
 * @IEEE80211_TX_CTL_LDPC: tells the driver to use LDPC for this frame
 * @IEEE80211_TX_CTL_STBC: Enables Space-Time Block Coding (STBC) for this
 *	frame and selects the maximum number of streams that it can use.
 * @IEEE80211_TX_CTL_TX_OFFCHAN: Marks this packet to be transmitted on
 *	the off-channel channel when a remain-on-channel offload is done
 *	in hardware -- normal packets still flow and are expected to be
 *	handled properly by the device.
 * @IEEE80211_TX_INTFL_TKIP_MIC_FAILURE: Marks this packet to be used for TKIP
 *	testing. It will be sent out with incorrect Michael MIC key to allow
 *	TKIP countermeasures to be tested.
 * @IEEE80211_TX_CTL_NO_CCK_RATE: This frame will be sent at non CCK rate.
 *	This flag is actually used for management frame especially for P2P
 *	frames not being sent at CCK rate in 2GHz band.
 * @IEEE80211_TX_STATUS_EOSP: This packet marks the end of service period,
 *	when its status is reported the service period ends. For frames in
 *	an SP that mac80211 transmits, it is already set; for driver frames
 *	the driver may set this flag. It is also used to do the same for
 *	PS-Poll responses.
 * @IEEE80211_TX_CTL_USE_MINRATE: This frame will be sent at lowest rate.
 *	This flag is used to send nullfunc frame at minimum rate when
 *	the nullfunc is used for connection monitoring purpose.
 * @IEEE80211_TX_CTL_DONTFRAG: Don't fragment this packet even if it
 *	would be fragmented by size (this is optional, only used for
 *	monitor injection).
 *
 * Note: If you have to add new flags to the enumeration, then don't
 *	 forget to update %IEEE80211_TX_TEMPORARY_FLAGS when necessary.
 */
enum mac80211_tx_control_flags {
	IEEE80211_TX_CTL_REQ_TX_STATUS		= BIT(0),
	IEEE80211_TX_CTL_ASSIGN_SEQ		= BIT(1),
	IEEE80211_TX_CTL_NO_ACK			= BIT(2),
	IEEE80211_TX_CTL_CLEAR_PS_FILT		= BIT(3),
	IEEE80211_TX_CTL_FIRST_FRAGMENT		= BIT(4),
	IEEE80211_TX_CTL_SEND_AFTER_DTIM	= BIT(5),
	IEEE80211_TX_CTL_AMPDU			= BIT(6),
	IEEE80211_TX_CTL_INJECTED		= BIT(7),
	IEEE80211_TX_STAT_TX_FILTERED		= BIT(8),
	IEEE80211_TX_STAT_ACK			= BIT(9),
	IEEE80211_TX_STAT_AMPDU			= BIT(10),
	IEEE80211_TX_STAT_AMPDU_NO_BACK		= BIT(11),
	IEEE80211_TX_CTL_RATE_CTRL_PROBE	= BIT(12),
	IEEE80211_TX_INTFL_NEED_TXPROCESSING	= BIT(14),
	IEEE80211_TX_INTFL_RETRIED		= BIT(15),
	IEEE80211_TX_INTFL_DONT_ENCRYPT		= BIT(16),
	IEEE80211_TX_CTL_POLL_RESPONSE		= BIT(17),
	IEEE80211_TX_CTL_MORE_FRAMES		= BIT(18),
	IEEE80211_TX_INTFL_RETRANSMISSION	= BIT(19),
	IEEE80211_TX_HANDSHAKE			    = BIT(20),
	/* hole at 20, use later */
	IEEE80211_TX_INTFL_NL80211_FRAME_TX	= BIT(21),
	IEEE80211_TX_CTL_LDPC			= BIT(22),
	IEEE80211_TX_CTL_STBC			= BIT(23) | BIT(24),
	IEEE80211_TX_CTL_TX_OFFCHAN		= BIT(25),
	IEEE80211_TX_INTFL_TKIP_MIC_FAILURE	= BIT(26),
	IEEE80211_TX_CTL_NO_CCK_RATE		= BIT(27),
	IEEE80211_TX_STATUS_EOSP		= BIT(28),
	IEEE80211_TX_CTL_USE_MINRATE		= BIT(29),
	IEEE80211_TX_CTL_DONTFRAG		= BIT(30),
	IEEE80211_TX_AP_HANDLE_STATUS	= BIT(31),
};

#define IEEE80211_TX_CTL_STBC_SHIFT		23

/*
 * This definition is used as a mask to clear all temporary flags, which are
 * set by the tx handlers for each transmission attempt by the mac80211 stack.
 */
#define IEEE80211_TX_TEMPORARY_FLAGS (IEEE80211_TX_CTL_NO_ACK |		      \
	IEEE80211_TX_CTL_CLEAR_PS_FILT | IEEE80211_TX_CTL_FIRST_FRAGMENT |    \
	IEEE80211_TX_CTL_SEND_AFTER_DTIM | IEEE80211_TX_CTL_AMPDU |	      \
	IEEE80211_TX_STAT_TX_FILTERED |	IEEE80211_TX_STAT_ACK |		      \
	IEEE80211_TX_STAT_AMPDU | IEEE80211_TX_STAT_AMPDU_NO_BACK |	      \
	IEEE80211_TX_CTL_RATE_CTRL_PROBE | IEEE80211_TX_CTL_POLL_RESPONSE |   \
	IEEE80211_TX_CTL_MORE_FRAMES | IEEE80211_TX_CTL_LDPC |		      \
	IEEE80211_TX_CTL_STBC | IEEE80211_TX_STATUS_EOSP)

/**
 * enum mac80211_rate_control_flags - per-rate flags set by the
 *	Rate Control algorithm.
 *
 * These flags are set by the Rate control algorithm for each rate during tx,
 * in the @flags member of struct ieee80211_tx_rate.
 *
 * @IEEE80211_TX_RC_USE_RTS_CTS: Use RTS/CTS exchange for this rate.
 * @IEEE80211_TX_RC_USE_CTS_PROTECT: CTS-to-self protection is required.
 *	This is set if the current BSS requires ERP protection.
 * @IEEE80211_TX_RC_USE_SHORT_PREAMBLE: Use short preamble.
 * @IEEE80211_TX_RC_MCS: HT rate.
 * @IEEE80211_TX_RC_GREEN_FIELD: Indicates whether this rate should be used in
 *	Greenfield mode.
 * @IEEE80211_TX_RC_40_MHZ_WIDTH: Indicates if the Channel Width should be 40 MHz.
 * @IEEE80211_TX_RC_DUP_DATA: The frame should be transmitted on both of the
 *	adjacent 20 MHz channels, if the current channel type is
 *	NL80211_CHAN_HT40MINUS or NL80211_CHAN_HT40PLUS.
 * @IEEE80211_TX_RC_SHORT_GI: Short Guard interval should be used for this rate.
 */
enum mac80211_rate_control_flags {
	IEEE80211_TX_RC_USE_RTS_CTS		= BIT(0),
	IEEE80211_TX_RC_USE_CTS_PROTECT		= BIT(1),
	IEEE80211_TX_RC_USE_SHORT_PREAMBLE	= BIT(2),

	/* rate index is an MCS rate number instead of an index */
	IEEE80211_TX_RC_MCS			= BIT(3),
	IEEE80211_TX_RC_GREEN_FIELD		= BIT(4),
	IEEE80211_TX_RC_40_MHZ_WIDTH		= BIT(5),
	IEEE80211_TX_RC_DUP_DATA		= BIT(6),
	IEEE80211_TX_RC_SHORT_GI		= BIT(7),
};


/* there are 40 bytes if you don't need the rateset to be kept */
#define IEEE80211_TX_INFO_DRIVER_DATA_SIZE 40

/* if you do need the rateset, then you have less space */
#define IEEE80211_TX_INFO_RATE_DRIVER_DATA_SIZE 24

/* maximum number of rate stages */
#define IEEE80211_TX_MAX_RATES	5

/**
 * struct ieee80211_tx_rate - rate selection/status
 *
 * @idx: rate index to attempt to send with
 * @flags: rate control flags (&enum mac80211_rate_control_flags)
 * @count: number of tries in this rate before going to the next rate
 *
 * A value of -1 for @idx indicates an invalid rate and, if used
 * in an array of retry rates, that no more rates should be tried.
 *
 * When used for transmit status reporting, the driver should
 * always report the rate along with the flags it used.
 *
 * &struct ieee80211_tx_info contains an array of these structs
 * in the control information, and it will be filled by the rate
 * control algorithm according to what should be sent. For example,
 * if this array contains, in the format { <idx>, <count> } the
 * information
 *    { 3, 2 }, { 2, 2 }, { 1, 4 }, { -1, 0 }, { -1, 0 }
 * then this means that the frame should be transmitted
 * up to twice at rate 3, up to twice at rate 2, and up to four
 * times at rate 1 if it doesn't get acknowledged. Say it gets
 * acknowledged by the peer after the fifth attempt, the status
 * information should then contain
 *   { 3, 2 }, { 2, 2 }, { 1, 1 }, { -1, 0 } ...
 * since it was transmitted twice at rate 3, twice at rate 2
 * and once at rate 1 after which we received an acknowledgement.
 */
struct ieee80211_tx_rate {
	s8 idx;
	u8 count;
	u8 flags;
} __packed;

/**
 * struct ieee80211_tx_info - skb transmit information
 *
 * This structure is placed in skb->cb for three uses:
 *  (1) mac80211 TX control - mac80211 tells the driver what to do
 *  (2) driver internal use (if applicable)
 *  (3) TX status information - driver tells mac80211 what happened
 *
 * The TX control's sta pointer is only valid during the ->tx call,
 * it may be NULL.
 *
 * @flags: transmit info flags, defined above
 * @band: the band to transmit on (use for checking for races)
 * @antenna_sel_tx: antenna to use, 0 for automatic diversity
 * @hw_queue: HW queue to put the frame on, skb_get_queue_mapping() gives the AC
 * @pad: padding, ignore
 * @control: union for control data
 * @status: union for status data
 * @driver_data: array of driver_data pointers
 * @ampdu_ack_len: number of acked aggregated frames.
 * 	relevant only if IEEE80211_TX_STAT_AMPDU was set.
 * @ampdu_len: number of aggregated frames.
 * 	relevant only if IEEE80211_TX_STAT_AMPDU was set.
 * @ack_signal: signal strength of the ACK frame
 */
struct ieee80211_tx_info {
	/* common information */
	u32 flags;
	u8 band;

	u8 antenna_sel_tx;

	u8 hw_queue;
	/* 2 byte hole */
	//u8 pad[1];
	u8 sg_tailneed;

	union {
		struct {
			union {
				/* rate control */
				struct {
					struct ieee80211_tx_rate rates[
						IEEE80211_TX_MAX_RATES];
					s8 rts_cts_rate_idx;
				};
				/* only needed before rate control */
				unsigned long jiffies;
			};
			/* NB: vif can be NULL for injected frames */
			struct ieee80211_vif *vif;
			struct ieee80211_key_conf *hw_key;
			struct ieee80211_sta *sta;
		} control;
		struct {
			struct ieee80211_tx_rate rates[IEEE80211_TX_MAX_RATES];
			u8 ampdu_ack_len;
			int ack_signal;
			u8 ampdu_len;
			/* 15 bytes free */
		} status;
		struct {
			struct ieee80211_tx_rate driver_rates[
				IEEE80211_TX_MAX_RATES];
			void *rate_driver_data[
				IEEE80211_TX_INFO_RATE_DRIVER_DATA_SIZE / sizeof(void *)];
		};
		void *driver_data[
			IEEE80211_TX_INFO_DRIVER_DATA_SIZE / sizeof(void *)];
	};
};

/**
 * struct ieee80211_sched_scan_ies - scheduled scan IEs
 *
 * This structure is used to pass the appropriate IEs to be used in scheduled
 * scans for all bands.  It contains both the IEs passed from the userspace
 * and the ones generated by mac80211.
 *
 * @ie: array with the IEs for each supported band
 * @len: array with the total length of the IEs for each band
 */
struct ieee80211_sched_scan_ies {
	u8 *ie[IEEE80211_NUM_BANDS];
	size_t len[IEEE80211_NUM_BANDS];
};

static inline struct ieee80211_tx_info *IEEE80211_SKB_CB(struct sk_buff *skb)
{
	return (struct ieee80211_tx_info *)skb->cb;
}

static inline struct ieee80211_rx_status *IEEE80211_SKB_RXCB(struct sk_buff *skb)
{
	return (struct ieee80211_rx_status *)skb->cb;
}
#if 0
/**
 * ieee80211_tx_info_clear_status - clear TX status
 *
 * @info: The &struct ieee80211_tx_info to be cleared.
 *
 * When the driver passes an skb back to mac80211, it must report
 * a number of things in TX status. This function clears everything
 * in the TX status but the rate control information (it does clear
 * the count since you need to fill that in anyway).
 *
 * NOTE: You can only use this function if you do NOT use
 *	 info->driver_data! Use info->rate_driver_data
 *	 instead if you need only the less space that allows.
 */
static inline void
ieee80211_tx_info_clear_status(struct ieee80211_tx_info *info)
{
	int i;

	BUILD_BUG_ON(offsetof(struct ieee80211_tx_info, status.rates) !=
		     offsetof(struct ieee80211_tx_info, control.rates));
	BUILD_BUG_ON(offsetof(struct ieee80211_tx_info, status.rates) !=
		     offsetof(struct ieee80211_tx_info, driver_rates));
	BUILD_BUG_ON(offsetof(struct ieee80211_tx_info, status.rates) != 8);
	/* clear the rate counts */
	for (i = 0; i < IEEE80211_TX_MAX_RATES; i++)
		info->status.rates[i].count = 0;

	BUILD_BUG_ON(
	    offsetof(struct ieee80211_tx_info, status.ampdu_ack_len) != 23);
	memset(&info->status.ampdu_ack_len, 0,
	       sizeof(struct ieee80211_tx_info) -
	       offsetof(struct ieee80211_tx_info, status.ampdu_ack_len));
}

#endif
/**
 * enum mac80211_rx_flags - receive flags
 *
 * These flags are used with the @flag member of &struct ieee80211_rx_status.
 * @RX_FLAG_MMIC_ERROR: Michael MIC error was reported on this frame.
 *	Use together with %RX_FLAG_MMIC_STRIPPED.
 * @RX_FLAG_DECRYPTED: This frame was decrypted in hardware.
 * @RX_FLAG_MMIC_STRIPPED: the Michael MIC is stripped off this frame,
 *	verification has been done by the hardware.
 * @RX_FLAG_IV_STRIPPED: The IV/ICV are stripped from this frame.
 *	If this flag is set, the stack cannot do any replay detection
 *	hence the driver or hardware will have to do that.
 * @RX_FLAG_FAILED_FCS_CRC: Set this flag if the FCS check failed on
 *	the frame.
 * @RX_FLAG_FAILED_PLCP_CRC: Set this flag if the PCLP check failed on
 *	the frame.
 * @RX_FLAG_MACTIME_MPDU: The timestamp passed in the RX status (@mactime
 *	field) is valid and contains the time the first symbol of the MPDU
 *	was received. This is useful in monitor mode and for proper IBSS
 *	merging.
 * @RX_FLAG_SHORTPRE: Short preamble was used for this frame
 * @RX_FLAG_HT: HT MCS was used and rate_idx is MCS index
 * @RX_FLAG_40MHZ: HT40 (40 MHz) was used
 * @RX_FLAG_SHORT_GI: Short guard interval was used
 */
enum mac80211_rx_flags {
	RX_FLAG_MMIC_ERROR	= 1<<0,
	RX_FLAG_DECRYPTED	= 1<<1,
	RX_FLAG_MMIC_STRIPPED	= 1<<3,
	RX_FLAG_IV_STRIPPED	= 1<<4,
	RX_FLAG_FAILED_FCS_CRC	= 1<<5,
	RX_FLAG_FAILED_PLCP_CRC = 1<<6,
	RX_FLAG_MACTIME_MPDU	= 1<<7,
	RX_FLAG_SHORTPRE	= 1<<8,
	RX_FLAG_HT		= 1<<9,
	RX_FLAG_40MHZ		= 1<<10,
	RX_FLAG_SHORT_GI	= 1<<11,
	RX_FLAG_HW_CHKSUM_ERROR = 1<<12,
	RX_FLAG_STA_LISTEN  = 1<<13,
	RX_FLAG_UNKOWN_STA_FRAME = 1<<14,
	RX_FLAG_AMPDU =	1<<15,
	RX_FLAG_SKIP_KEY = 1<<16,
	RX_FLAG_QOS_DUP = 1<<17,
};

/**
 * struct ieee80211_rx_status - receive status
 *
 * The low-level driver should provide this information (the subset
 * supported by hardware) to the 802.11 code with each received
 * frame, in the skb's control buffer (cb).
 *
 * @mactime: value in microseconds of the 64-bit Time Synchronization Function
 * 	(TSF) timer when the first data symbol (MPDU) arrived at the hardware.
 * @band: the active band when this frame was received
 * @freq: frequency the radio was tuned to when receiving this frame, in MHz
 * @signal: signal strength when receiving this frame, either in dBm, in dB or
 *	unspecified depending on the hardware capabilities flags
 *	@IEEE80211_HW_SIGNAL_*
 * @antenna: antenna used
 * @rate_idx: index of data rate into band's supported rates or MCS index if
 *	HT rates are use (RX_FLAG_HT)
 * @flag: %RX_FLAG_*
 * @rx_flags: internal RX flags for mac80211
 */
struct ieee80211_rx_status {
	u64 mactime;
	enum ieee80211_band band;
	int freq;
	int signal;
	int antenna;
	int rate_idx;
	int flag;
	unsigned int rx_flags;
};

/**
 * enum ieee80211_conf_flags - configuration flags
 *
 * Flags to define PHY configuration options
 *
 * @IEEE80211_CONF_MONITOR: there's a monitor interface present -- use this
 *	to determine for example whether to calculate timestamps for packets
 *	or not, do not use instead of filter flags!
 * @IEEE80211_CONF_PS: Enable 802.11 power save mode (managed mode only).
 *	This is the power save mode defined by IEEE 802.11-2007 section 11.2,
 *	meaning that the hardware still wakes up for beacons, is able to
 *	transmit frames and receive the possible acknowledgment frames.
 *	Not to be confused with hardware specific wakeup/sleep states,
 *	driver is responsible for that. See the section "Powersave support"
 *	for more.
 * @IEEE80211_CONF_IDLE: The device is running, but idle; if the flag is set
 *	the driver should be prepared to handle configuration requests but
 *	may turn the device off as much as possible. Typically, this flag will
 *	be set when an interface is set UP but not associated or scanning, but
 *	it can also be unset in that case when monitor interfaces are active.
 */
enum ieee80211_conf_flags {
	IEEE80211_CONF_MONITOR		= (1<<0),
	IEEE80211_CONF_PS		= (1<<1),
	IEEE80211_CONF_IDLE		= (1<<2),
};


/**
 * enum ieee80211_conf_changed - denotes which configuration changed
 *
 * @IEEE80211_CONF_CHANGE_LISTEN_INTERVAL: the listen interval changed
 * @IEEE80211_CONF_CHANGE_MONITOR: the monitor flag changed
 * @IEEE80211_CONF_CHANGE_PS: the PS flag or dynamic PS timeout changed
 * @IEEE80211_CONF_CHANGE_POWER: the TX power changed
 * @IEEE80211_CONF_CHANGE_CHANNEL: the channel/channel_type changed
 * @IEEE80211_CONF_CHANGE_RETRY_LIMITS: retry limits changed
 * @IEEE80211_CONF_CHANGE_IDLE: Idle flag changed
 * @IEEE80211_CONF_CHANGE_SMPS: Spatial multiplexing powersave mode changed
 */
enum ieee80211_conf_changed {
	IEEE80211_CONF_CHANGE_SMPS		= BIT(1),
	IEEE80211_CONF_CHANGE_LISTEN_INTERVAL	= BIT(2),
	IEEE80211_CONF_CHANGE_MONITOR		= BIT(3),
	IEEE80211_CONF_CHANGE_PS		= BIT(4),
	IEEE80211_CONF_CHANGE_POWER		= BIT(5),
	IEEE80211_CONF_CHANGE_CHANNEL		= BIT(6),
	IEEE80211_CONF_CHANGE_RETRY_LIMITS	= BIT(7),
	IEEE80211_CONF_CHANGE_IDLE		= BIT(8),
};

/**
 * enum ieee80211_smps_mode - spatial multiplexing power save mode
 *
 * @IEEE80211_SMPS_AUTOMATIC: automatic
 * @IEEE80211_SMPS_OFF: off
 * @IEEE80211_SMPS_STATIC: static
 * @IEEE80211_SMPS_DYNAMIC: dynamic
 * @IEEE80211_SMPS_NUM_MODES: internal, don't use
 */
enum ieee80211_smps_mode {
	IEEE80211_SMPS_AUTOMATIC,
	IEEE80211_SMPS_OFF,
	IEEE80211_SMPS_STATIC,
	IEEE80211_SMPS_DYNAMIC,

	/* keep last */
	IEEE80211_SMPS_NUM_MODES,
};

/**
 * struct ieee80211_conf - configuration of the device
 *
 * This struct indicates how the driver shall configure the hardware.
 *
 * @flags: configuration flags defined above
 *
 * @power_level: requested transmit power (in dBm)
 *
 * @long_frame_max_tx_count: Maximum number of transmissions for a "long" frame
 *    (a frame not RTS protected), called "dot11LongRetryLimit" in 802.11,
 *    but actually means the number of transmissions not the number of retries
 * @short_frame_max_tx_count: Maximum number of transmissions for a "short"
 *    frame, called "dot11ShortRetryLimit" in 802.11, but actually means the
 *    number of transmissions not the number of retries
 *
 * @chan_conf: current channel configuration (i.e. channel, channel type)
 *
 * @smps_mode: spatial multiplexing powersave mode; note that
 *	%IEEE80211_SMPS_STATIC is used when the device is not
 *	configured for an HT channel
 */
struct ieee80211_conf {
	u32 flags;
	int power_level;
	int ps_dtim_period;

	u8 long_frame_max_tx_count, short_frame_max_tx_count;

	struct ieee80211_channel_conf *chan_conf;
	enum ieee80211_smps_mode smps_mode;
};

/**
 * struct ieee80211_channel_switch - holds the channel switch data
 *
 * The information provided in this structure is required for channel switch
 * operation.
 *
 * @timestamp: value in microseconds of the 64-bit Time Synchronization
 *	Function (TSF) timer when the frame containing the channel switch
 *	announcement was received. This is simply the rx.mactime parameter
 *	the driver passed into mac80211.
 * @block_tx: Indicates whether transmission must be blocked before the
 *	scheduled channel switch, as indicated by the AP.
 * @channel: the new channel to switch to
 * @count: the number of TBTT's until the channel switch event
 */
struct ieee80211_channel_switch {
	u64 timestamp;
	bool block_tx;
	struct ieee80211_channel *channel;
	u8 count;
};

/**
 * struct ieee80211_vif - per-interface data
 *
 * Data in this structure is continually present for driver
 * use during the life of a virtual interface.
 *
 * @type: type of this virtual interface
 * @bss_conf: BSS configuration for this interface, either our own
 *	or the BSS we're associated to
 * @addr: address of this interface
 * @p2p: indicates whether this AP or STA interface is a p2p
 *	interface, i.e. a GO or p2p-sta respectively
 * @hw_queue: hardware queue for each AC
 * @cab_queue: content-after-beacon (DTIM beacon really) queue, AP mode only
 * @drv_priv: data area for driver use, will always be aligned to
 *	sizeof(void *).
 */
struct ieee80211_vif {
	enum nl80211_iftype type;
	struct ieee80211_bss_conf bss_conf;
	u8 addr[ETH_ALEN];
	bool p2p;

	u8 cab_queue;
	u8 hw_queue[IEEE80211_NUM_ACS];
#ifdef CONFIG_ATBM_SUPPORT_REKEY
	int rekey_set;
	u8 kek[16];
	u8 kck[16];
	u8 replay_ctr[8];
#endif
	/* must be last */
	u8 drv_priv[0] __attribute__((__aligned__(sizeof(void *))));
};

static inline bool ieee80211_vif_is_mesh(struct ieee80211_vif *vif)
{
#ifdef CONFIG_MAC80211_ATBM_MESH
	return vif->type == NL80211_IFTYPE_MESH_POINT;
#endif
	return false;
}

/**
 * enum ieee80211_key_flags - key flags
 *
 * These flags are used for communication about keys between the driver
 * and mac80211, with the @flags parameter of &struct ieee80211_key_conf.
 *
 * @IEEE80211_KEY_FLAG_WMM_STA: Set by mac80211, this flag indicates
 *	that the STA this key will be used with could be using QoS.
 * @IEEE80211_KEY_FLAG_GENERATE_IV: This flag should be set by the
 *	driver to indicate that it requires IV generation for this
 *	particular key.
 * @IEEE80211_KEY_FLAG_GENERATE_MMIC: This flag should be set by
 *	the driver for a TKIP key if it requires Michael MIC
 *	generation in software.
 * @IEEE80211_KEY_FLAG_PAIRWISE: Set by mac80211, this flag indicates
 *	that the key is pairwise rather then a shared key.
 * @IEEE80211_KEY_FLAG_SW_MGMT: This flag should be set by the driver for a
 *	CCMP key if it requires CCMP encryption of management frames (MFP) to
 *	be done in software.
 */
enum ieee80211_key_flags {
	IEEE80211_KEY_FLAG_WMM_STA	= 1<<0,
	IEEE80211_KEY_FLAG_GENERATE_IV	= 1<<1,
	IEEE80211_KEY_FLAG_GENERATE_MMIC= 1<<2,
	IEEE80211_KEY_FLAG_PAIRWISE	= 1<<3,
	IEEE80211_KEY_FLAG_SW_MGMT	= 1<<4,
	IEEE80211_KEY_FLAG_ALLOC_IV	= 1<<5,
};

/**
 * struct ieee80211_key_conf - key information
 *
 * This key information is given by mac80211 to the driver by
 * the set_key() callback in &struct ieee80211_ops.
 *
 * @hw_key_idx: To be set by the driver, this is the key index the driver
 *	wants to be given when a frame is transmitted and needs to be
 *	encrypted in hardware.
 * @cipher: The key's cipher suite selector.
 * @flags: key flags, see &enum ieee80211_key_flags.
 * @keyidx: the key index (0-3)
 * @keylen: key material length
 * @key: key material. For ALG_TKIP the key is encoded as a 256-bit (32 byte)
 * 	data block:
 * 	- Temporal Encryption Key (128 bits)
 * 	- Temporal Authenticator Tx MIC Key (64 bits)
 * 	- Temporal Authenticator Rx MIC Key (64 bits)
 * @icv_len: The ICV length for this key type
 * @iv_len: The IV length for this key type
 */
struct ieee80211_key_conf {
	u32 cipher;
	u8 icv_len;
	u8 iv_len;
	u8 hw_key_idx;
	u8 flags;
	s8 keyidx;
	u8 keylen;
	u8 key[0];
};

/**
 * enum set_key_cmd - key command
 *
 * Used with the set_key() callback in &struct ieee80211_ops, this
 * indicates whether a key is being removed or added.
 *
 * @SET_KEY: a key is set
 * @DISABLE_KEY: a key must be disabled
 */
enum set_key_cmd {
	SET_KEY, DISABLE_KEY,
};

/**
 * struct ieee80211_sta - station table entry
 *
 * A station table entry represents a station we are possibly
 * communicating with. Since stations are RCU-managed in
 * mac80211, any ieee80211_sta pointer you get access to must
 * either be protected by rcu_read_lock() explicitly or implicitly,
 * or you must take good care to not use such a pointer after a
 * call to your sta_remove callback that removed it.
 *
 * @addr: MAC address
 * @aid: AID we assigned to the station if we're an AP
 * @supp_rates: Bitmap of supported rates (per band)
 * @ht_cap: HT capabilities of this STA; restricted to our own TX capabilities
 * @wme: indicates whether the STA supports WME. Only valid during AP-mode.
 * @drv_priv: data area for driver use, will always be aligned to
 *	sizeof(void *), size is determined in hw information.
 * @uapsd_queues: bitmap of queues configured for uapsd. Only valid
 *	if wme is supported.
 * @max_sp: max Service Period. Only valid if wme is supported.
 */
struct ieee80211_sta {
	u32 supp_rates[IEEE80211_NUM_BANDS];
	u8 addr[ETH_ALEN];
	u16 aid;
	struct ieee80211_sta_ht_cap ht_cap;
	bool wme;
	u8 uapsd_queues;
	u8 max_sp;
	struct atbm_wpa_ie_data wpa_sta_data;
	/* must be last */
	u8 drv_priv[0] __attribute__((__aligned__(sizeof(void *))));
};

/**
 * enum sta_notify_cmd - sta notify command
 *
 * Used with the sta_notify() callback in &struct ieee80211_ops, this
 * indicates if an associated station made a power state transition.
 *
 * @STA_NOTIFY_SLEEP: a station is now sleeping
 * @STA_NOTIFY_AWAKE: a sleeping station woke up
 */
enum sta_notify_cmd {
	STA_NOTIFY_SLEEP, STA_NOTIFY_AWAKE,
};

/**
 * enum ieee80211_hw_flags - hardware flags
 *
 * These flags are used to indicate hardware capabilities to
 * the stack. Generally, flags here should have their meaning
 * done in a way that the simplest hardware doesn't need setting
 * any particular flags. There are some exceptions to this rule,
 * however, so you are advised to review these flags carefully.
 *
 * @IEEE80211_HW_HAS_RATE_CONTROL:
 *	The hardware or firmware includes rate control, and cannot be
 *	controlled by the stack. As such, no rate control algorithm
 *	should be instantiated, and the TX rate reported to userspace
 *	will be taken from the TX status instead of the rate control
 *	algorithm.
 *	Note that this requires that the driver implement a number of
 *	callbacks so it has the correct information, it needs to have
 *	the @set_rts_threshold callback and must look at the BSS config
 *	@use_cts_prot for G/N protection, @use_short_slot for slot
 *	timing in 2.4 GHz and @use_short_preamble for preambles for
 *	CCK frames.
 *
 * @IEEE80211_HW_RX_INCLUDES_FCS:
 *	Indicates that received frames passed to the stack include
 *	the FCS at the end.
 *
 * @IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING:
 *	Some wireless LAN chipsets buffer broadcast/multicast frames
 *	for power saving stations in the hardware/firmware and others
 *	rely on the host system for such buffering. This option is used
 *	to configure the IEEE 802.11 upper layer to buffer broadcast and
 *	multicast frames when there are power saving stations so that
 *	the driver can fetch them with ieee80211_get_buffered_bc().
 *
 * @IEEE80211_HW_2GHZ_SHORT_SLOT_INCAPABLE:
 *	Hardware is not capable of short slot operation on the 2.4 GHz band.
 *
 * @IEEE80211_HW_2GHZ_SHORT_PREAMBLE_INCAPABLE:
 *	Hardware is not capable of receiving frames with short preamble on
 *	the 2.4 GHz band.
 *
 * @IEEE80211_HW_SIGNAL_UNSPEC:
 *	Hardware can provide signal values but we don't know its units. We
 *	expect values between 0 and @max_signal.
 *	If possible please provide dB or dBm instead.
 *
 * @IEEE80211_HW_SIGNAL_DBM:
 *	Hardware gives signal values in dBm, decibel difference from
 *	one milliwatt. This is the preferred method since it is standardized
 *	between different devices. @max_signal does not need to be set.
 *
 * @IEEE80211_HW_SPECTRUM_MGMT:
 * 	Hardware supports spectrum management defined in 802.11h
 * 	Measurement, Channel Switch, Quieting, TPC
 *
 * @IEEE80211_HW_AMPDU_AGGREGATION:
 *	Hardware supports 11n A-MPDU aggregation.
 *
 * @IEEE80211_HW_SUPPORTS_PS:
 *	Hardware has power save support (i.e. can go to sleep).
 *
 * @IEEE80211_HW_PS_NULLFUNC_STACK:
 *	Hardware requires nullfunc frame handling in stack, implies
 *	stack support for dynamic PS.
 *
 * @IEEE80211_HW_SUPPORTS_DYNAMIC_PS:
 *	Hardware has support for dynamic PS.
 *
 * @IEEE80211_HW_MFP_CAPABLE:
 *	Hardware supports management frame protection (MFP, IEEE 802.11w).
 *
 * @IEEE80211_HW_BEACON_FILTER:
 *	Hardware supports dropping of irrelevant beacon frames to
 *	avoid waking up cpu.
 *
 * @IEEE80211_HW_SUPPORTS_STATIC_SMPS:
 *	Hardware supports static spatial multiplexing powersave,
 *	ie. can turn off all but one chain even on HT connections
 *	that should be using more chains.
 *
 * @IEEE80211_HW_SUPPORTS_DYNAMIC_SMPS:
 *	Hardware supports dynamic spatial multiplexing powersave,
 *	ie. can turn off all but one chain and then wake the rest
 *	up as required after, for example, rts/cts handshake.
 *
 * @IEEE80211_HW_SUPPORTS_UAPSD:
 *	Hardware supports Unscheduled Automatic Power Save Delivery
 *	(U-APSD) in managed mode. The mode is configured with
 *	conf_tx() operation.
 *
 * @IEEE80211_HW_REPORTS_TX_ACK_STATUS:
 *	Hardware can provide ack status reports of Tx frames to
 *	the stack.
 *
 * @IEEE80211_HW_CONNECTION_MONITOR:
 *      The hardware performs its own connection monitoring, including
 *      periodic keep-alives to the AP and probing the AP on beacon loss.
 *      When this flag is set, signaling beacon-loss will cause an immediate
 *      change to disassociated state.
 *
 * @IEEE80211_HW_SUPPORTS_CQM_RSSI:
 *	Hardware can do connection quality monitoring - i.e. it can monitor
 *	connection quality related parameters, such as the RSSI level and
 *	provide notifications if configured trigger levels are reached.
 *
 * @IEEE80211_HW_NEED_DTIM_PERIOD:
 *	This device needs to know the DTIM period for the BSS before
 *	associating.
 *
 * @IEEE80211_HW_SUPPORTS_PER_STA_GTK: The device's crypto engine supports
 *	per-station GTKs as used by IBSS RSN or during fast transition. If
 *	the device doesn't support per-station GTKs, but can be asked not
 *	to decrypt group addressed frames, then IBSS RSN support is still
 *	possible but software crypto will be used. Advertise the wiphy flag
 *	only in that case.
 *
 * @IEEE80211_HW_AP_LINK_PS: When operating in AP mode the device
 *	autonomously manages the PS status of connected stations. When
 *	this flag is set mac80211 will not trigger PS mode for connected
 *	stations based on the PM bit of incoming frames.
 *	Use ieee80211_start_ps()/ieee8021_end_ps() to manually configure
 *	the PS mode of connected stations.
 *
 * @IEEE80211_HW_TX_AMPDU_SETUP_IN_HW: The device handles TX A-MPDU session
 *      setup strictly in HW. mac80211 should not attempt to do this in
 *      software.
 *
 * @IEEE80211_HW_SUPPORTS_CQM_BEACON_MISS
 *      Connection quality monitoring - beacon miss.
 *
 * @IEEE80211_HW_SUPPORTS_CQM_TX_FAIL
 *      Connection quality monitoring - tx failure.
 *
 * @IEEE80211_HW_SUPPORTS_MULTI_CHANNEL
 *      Hardware supports simultaneous operation on different channels.
 *
 * @IEEE80211_HW_QUEUE_CONTROL: The driver wants to control per-interface
 *	queue mapping in order to use different queues (not just one per AC)
 *	for different virtual interfaces. See the doc section on HW queue
 *	control for more details.
 */
enum ieee80211_hw_flags {
	IEEE80211_HW_HAS_RATE_CONTROL			= 1<<0,
	IEEE80211_HW_RX_INCLUDES_FCS			= 1<<1,
	IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING	= 1<<2,
	IEEE80211_HW_2GHZ_SHORT_SLOT_INCAPABLE		= 1<<3,
	IEEE80211_HW_2GHZ_SHORT_PREAMBLE_INCAPABLE	= 1<<4,
	IEEE80211_HW_SIGNAL_UNSPEC			= 1<<5,
	IEEE80211_HW_SIGNAL_DBM				= 1<<6,
	IEEE80211_HW_NEED_DTIM_PERIOD			= 1<<7,
	IEEE80211_HW_SPECTRUM_MGMT			= 1<<8,
	IEEE80211_HW_AMPDU_AGGREGATION			= 1<<9,
	IEEE80211_HW_SUPPORTS_PS			= 1<<10,
	IEEE80211_HW_PS_NULLFUNC_STACK			= 1<<11,
	IEEE80211_HW_SUPPORTS_DYNAMIC_PS		= 1<<12,
	IEEE80211_HW_MFP_CAPABLE			= 1<<13,
	IEEE80211_HW_BEACON_FILTER			= 1<<14,
	IEEE80211_HW_SUPPORTS_STATIC_SMPS		= 1<<15,
	IEEE80211_HW_SUPPORTS_DYNAMIC_SMPS		= 1<<16,
	IEEE80211_HW_SUPPORTS_UAPSD			= 1<<17,
	IEEE80211_HW_REPORTS_TX_ACK_STATUS		= 1<<18,
	IEEE80211_HW_CONNECTION_MONITOR			= 1<<19,
	IEEE80211_HW_SUPPORTS_CQM_RSSI			= 1<<20,
	IEEE80211_HW_SUPPORTS_PER_STA_GTK		= 1<<21,
	IEEE80211_HW_AP_LINK_PS				= 1<<22,
	IEEE80211_HW_AMPDU_SETUP_IN_HW		= 1<<23,
	IEEE80211_HW_SUPPORTS_CQM_BEACON_MISS		= 1<<24,
	IEEE80211_HW_SUPPORTS_CQM_TX_FAIL		= 1<<25,
	IEEE80211_HW_SUPPORTS_P2P_PS			= 1<<26,
	IEEE80211_HW_SUPPORTS_MULTI_CHANNEL		= 1<<27,
	IEEE80211_HW_QUEUE_CONTROL			= 1<<28,
	IEEE80211_HW_MONITOR_NEED_PRISM_HEADER		= 1<<29,
	IEEE80211_HW_SCHEDULE_TASKLET		= 1<<30,
};

/**
 * struct ieee80211_hw - hardware information and state
 *
 * This structure contains the configuration and hardware
 * information for an 802.11 PHY.
 *
 * @wiphy: This points to the &struct wiphy allocated for this
 *	802.11 PHY. You must fill in the @perm_addr and @dev
 *	members of this structure using SET_IEEE80211_DEV()
 *	and SET_IEEE80211_PERM_ADDR(). Additionally, all supported
 *	bands (with channels, bitrates) are registered here.
 *
 * @conf: &struct ieee80211_conf, device configuration, don't use.
 *
 * @priv: pointer to private area that was allocated for driver use
 *	along with this structure.
 *
 * @flags: hardware flags, see &enum ieee80211_hw_flags.
 *
 * @extra_tx_headroom: headroom to reserve in each transmit skb
 *	for use by the driver (e.g. for transmit headers.)
 *
 * @channel_change_time: time (in microseconds) it takes to change channels.
 *
 * @max_signal: Maximum value for signal (rssi) in RX information, used
 *     only when @IEEE80211_HW_SIGNAL_UNSPEC or @IEEE80211_HW_SIGNAL_DB
 *
 * @max_listen_interval: max listen interval in units of beacon interval
 *     that HW supports
 *
 * @queues: number of available hardware transmit queues for
 *	data packets. WMM/QoS requires at least four, these
 *	queues need to have configurable access parameters.
 *
 * @rate_control_algorithm: rate control algorithm for this hardware.
 *	If unset (NULL), the default algorithm will be used. Must be
 *	set before calling ieee80211_register_hw().
 *
 * @vif_data_size: size (in bytes) of the drv_priv data area
 *	within &struct ieee80211_vif.
 * @sta_data_size: size (in bytes) of the drv_priv data area
 *	within &struct ieee80211_sta.
 *
 * @max_rates: maximum number of alternate rate retry stages the hw
 *	can handle.
 * @max_report_rates: maximum number of alternate rate retry stages
 *	the hw can report back.
 * @max_rate_tries: maximum number of tries for each stage
 *
 * @napi_weight: weight used for NAPI polling.  You must specify an
 *	appropriate value here if a napi_poll operation is provided
 *	by your driver.
 *
 * @max_rx_aggregation_subframes: maximum buffer size (number of
 *	sub-frames) to be used for A-MPDU block ack receiver
 *	aggregation.
 *	This is only relevant if the device has restrictions on the
 *	number of subframes, if it relies on mac80211 to do reordering
 *	it shouldn't be set.
 *
 * @max_tx_aggregation_subframes: maximum number of subframes in an
 *	aggregate an HT driver will transmit, used by the peer as a
 *	hint to size its reorder buffer.
 *
 * @offchannel_tx_hw_queue: HW queue ID to use for offchannel TX
 *	(if %IEEE80211_HW_QUEUE_CONTROL is set)
 */
 struct response{
		u32 len;
		u32 respbuff[31];
};
struct ieee80211_hw {
	struct ieee80211_conf conf;
	struct wiphy *wiphy;
	const char *rate_control_algorithm;
	void *priv;
	u32 flags;
	unsigned int extra_tx_headroom;
	int channel_change_time;
	int vif_data_size;
	int sta_data_size;
#ifdef IEEE80211_SUPPORT_NAPI
	int napi_weight;
#endif
	u16 queues;
	u16 max_listen_interval;
	u16 max_hw_support_rx_aggs;
	u16 max_hw_support_tx_aggs;
	s8 max_signal;
	u8 max_rates;
	u8 max_report_rates;
	u8 max_rate_tries;
	u8 max_rx_aggregation_subframes;
	u8 max_tx_aggregation_subframes;
	u8 offchannel_tx_hw_queue;
	u8 vendcmd_nl80211;
	struct response vendreturn;
};
#define IEEE80211_ATBM_MAX_SCAN_CHANNEL_INDEX		64
enum ieee80211_scan_req_wrap_flags{
	IEEE80211_SCAN_REQ_CCA             = 1<<0,
	IEEE80211_SCAN_REQ_INTERNAL        = 1<<1, 
	IEEE80211_SCAN_REQ_RESULTS_HANDLE  = 1<<2, 
	IEEE80211_SCAN_REQ_PASSIVE_SCAN    = 1<<3,
	IEEE80211_SCAN_REQ_RESULTS_SKB     = 1<<4,
	IEEE80211_SCAN_REQ_SPILT	   	   = 1<<5,
	IEEE80211_SCAN_REQ_NEED_BSSID      = 1<<6,
	IEEE80211_SCAN_REQ_ONLY_PROB       = 1<<7,
	IEEE80211_SCAN_REQ_NEED_LISTEN	   = 1<<8,
	IEEE80211_SCAN_REQ_ETF			   = 1<<9,
	IEEE80211_SCAN_REQ_ETF_STOP		   = 1<<10,
	IEEE80211_SCAN_REQ_ETF_STARTED	   = 1<<11,
	IEEE80211_SCAN_REQ_ETF_NEED_STOP   = 1<<12,
	IEEE80211_SCAN_REQ_UPDATE_BSS	   = 1<<13,
};
enum ieee80211_etf_request_action{
	EFT_REQUEST_ACTION_START,
	EFT_REQUEST_ACTION_SCAN_COMP,
	EFT_REQUEST_ACTION_SCAN_FORCE_STOP,
	EFT_REQUEST_ACTION_RECEIVE,
};
struct ieee80211_scan_req_wrap{
	struct cfg80211_scan_request *req;
	u8 bssid[8];
	u32 flags;
	u8  cca_val[IEEE80211_ATBM_MAX_SCAN_CHANNEL_INDEX];
	struct ieee80211_internal_etf_request *etf;
};
struct ieee80211_internal_etf_request{
	struct kref ref;
	u32 version;
	u32 channel_type;
	u32 channel;
	u32 rate;
	u32 len;	
	u32 greedfiled;
	u32 precomp_sel;
	char threshold_param[100];
	int (*etf_process)(struct ieee80211_hw *hw,struct ieee80211_vif *vif,
					   enum ieee80211_etf_request_action action,
					   struct ieee80211_scan_req_wrap *req_wrap,struct sk_buff *skb);
};

#ifdef CONFIG_ATBM_MAC80211_NO_USE
/**
 * wiphy_to_ieee80211_hw - return a mac80211 driver hw struct from a wiphy
 *
 * @wiphy: the &struct wiphy which we want to query
 *
 * mac80211 drivers can use this to get to their respective
 * &struct ieee80211_hw. Drivers wishing to get to their own private
 * structure can then access it via hw->priv. Note that mac802111 drivers should
 * not use wiphy_priv() to try to get their private driver structure as this
 * is already used internally by mac80211.
 */
struct ieee80211_hw *wiphy_to_ieee80211_hw(struct wiphy *wiphy);
#endif
/**
 * SET_IEEE80211_DEV - set device for 802.11 hardware
 *
 * @hw: the &struct ieee80211_hw to set the device for
 * @dev: the &struct device of this 802.11 device
 */
static inline void SET_IEEE80211_DEV(struct ieee80211_hw *hw, struct device *dev)
{
	set_wiphy_dev(hw->wiphy, dev);
}

/**
 * SET_IEEE80211_PERM_ADDR - set the permanent MAC address for 802.11 hardware
 *
 * @hw: the &struct ieee80211_hw to set the MAC address for
 * @addr: the address to set
 */
static inline void SET_IEEE80211_PERM_ADDR(struct ieee80211_hw *hw, u8 *addr)
{
	memcpy(hw->wiphy->perm_addr, addr, ETH_ALEN);
}

static inline int ieee80211_hw_setup_ba(struct ieee80211_hw *hw)
{
	return !!(hw->flags & IEEE80211_HW_AMPDU_SETUP_IN_HW);
}
#if 0
static inline struct ieee80211_rate *
ieee80211_get_tx_rate(const struct ieee80211_hw *hw,
		      const struct ieee80211_tx_info *c)
{
	if (WARN_ON(c->control.rates[0].idx < 0))
		return NULL;
	return &hw->wiphy->bands[c->band]->bitrates[c->control.rates[0].idx];
}

static inline struct ieee80211_rate *
ieee80211_get_rts_cts_rate(const struct ieee80211_hw *hw,
			   const struct ieee80211_tx_info *c)
{
	if (c->control.rts_cts_rate_idx < 0)
		return NULL;
	return &hw->wiphy->bands[c->band]->bitrates[c->control.rts_cts_rate_idx];
}

static inline struct ieee80211_rate *
ieee80211_get_alt_retry_rate(const struct ieee80211_hw *hw,
			     const struct ieee80211_tx_info *c, int idx)
{
	if (c->control.rates[idx + 1].idx < 0)
		return NULL;
	return &hw->wiphy->bands[c->band]->bitrates[c->control.rates[idx + 1].idx];
}
#endif
/**
 * DOC: Hardware crypto acceleration
 *
 * mac80211 is capable of taking advantage of many hardware
 * acceleration designs for encryption and decryption operations.
 *
 * The set_key() callback in the &struct ieee80211_ops for a given
 * device is called to enable hardware acceleration of encryption and
 * decryption. The callback takes a @sta parameter that will be NULL
 * for default keys or keys used for transmission only, or point to
 * the station information for the peer for individual keys.
 * Multiple transmission keys with the same key index may be used when
 * VLANs are configured for an access point.
 *
 * When transmitting, the TX control data will use the @hw_key_idx
 * selected by the driver by modifying the &struct ieee80211_key_conf
 * pointed to by the @key parameter to the set_key() function.
 *
 * The set_key() call for the %SET_KEY command should return 0 if
 * the key is now in use, -%EOPNOTSUPP or -%ENOSPC if it couldn't be
 * added; if you return 0 then hw_key_idx must be assigned to the
 * hardware key index, you are free to use the full u8 range.
 *
 * When the cmd is %DISABLE_KEY then it must succeed.
 *
 * Note that it is permissible to not decrypt a frame even if a key
 * for it has been uploaded to hardware, the stack will not make any
 * decision based on whether a key has been uploaded or not but rather
 * based on the receive flags.
 *
 * The &struct ieee80211_key_conf structure pointed to by the @key
 * parameter is guaranteed to be valid until another call to set_key()
 * removes it, but it can only be used as a cookie to differentiate
 * keys.
 *
 * In TKIP some HW need to be provided a phase 1 key, for RX decryption
 * acceleration (i.e. iwlwifi). Those drivers should provide update_tkip_key
 * handler.
 * The update_tkip_key() call updates the driver with the new phase 1 key.
 * This happens every time the iv16 wraps around (every 65536 packets). The
 * set_key() call will happen only once for each key (unless the AP did
 * rekeying), it will not include a valid phase 1 key. The valid phase 1 key is
 * provided by update_tkip_key only. The trigger that makes mac80211 call this
 * handler is software decryption with wrap around of iv16.
 */

/**
 * DOC: Powersave support
 *
 * mac80211 has support for various powersave implementations.
 *
 * First, it can support hardware that handles all powersaving by itself,
 * such hardware should simply set the %IEEE80211_HW_SUPPORTS_PS hardware
 * flag. In that case, it will be told about the desired powersave mode
 * with the %IEEE80211_CONF_PS flag depending on the association status.
 * The hardware must take care of sending nullfunc frames when necessary,
 * i.e. when entering and leaving powersave mode. The hardware is required
 * to look at the AID in beacons and signal to the AP that it woke up when
 * it finds traffic directed to it.
 *
 * %IEEE80211_CONF_PS flag enabled means that the powersave mode defined in
 * IEEE 802.11-2007 section 11.2 is enabled. This is not to be confused
 * with hardware wakeup and sleep states. Driver is responsible for waking
 * up the hardware before issuing commands to the hardware and putting it
 * back to sleep at appropriate times.
 *
 * When PS is enabled, hardware needs to wakeup for beacons and receive the
 * buffered multicast/broadcast frames after the beacon. Also it must be
 * possible to send frames and receive the acknowledment frame.
 *
 * Other hardware designs cannot send nullfunc frames by themselves and also
 * need software support for parsing the TIM bitmap. This is also supported
 * by mac80211 by combining the %IEEE80211_HW_SUPPORTS_PS and
 * %IEEE80211_HW_PS_NULLFUNC_STACK flags. The hardware is of course still
 * required to pass up beacons. The hardware is still required to handle
 * waking up for multicast traffic; if it cannot the driver must handle that
 * as best as it can, mac80211 is too slow to do that.
 *
 * Dynamic powersave is an extension to normal powersave in which the
 * hardware stays awake for a user-specified period of time after sending a
 * frame so that reply frames need not be buffered and therefore delayed to
 * the next wakeup. It's compromise of getting good enough latency when
 * there's data traffic and still saving significantly power in idle
 * periods.
 *
 * Dynamic powersave is simply supported by mac80211 enabling and disabling
 * PS based on traffic. Driver needs to only set %IEEE80211_HW_SUPPORTS_PS
 * flag and mac80211 will handle everything automatically. Additionally,
 * hardware having support for the dynamic PS feature may set the
 * %IEEE80211_HW_SUPPORTS_DYNAMIC_PS flag to indicate that it can support
 * dynamic PS mode itself. The driver needs to look at the
 * @dynamic_ps_timeout hardware configuration value and use it that value
 * whenever %IEEE80211_CONF_PS is set. In this case mac80211 will disable
 * dynamic PS feature in stack and will just keep %IEEE80211_CONF_PS
 * enabled whenever user has enabled powersave.
 *
 * Some hardware need to toggle a single shared antenna between WLAN and
 * Bluetooth to facilitate co-existence. These types of hardware set
 * limitations on the use of host controlled dynamic powersave whenever there
 * is simultaneous WLAN and Bluetooth traffic. For these types of hardware, the
 * driver may request temporarily going into full power save, in order to
 * enable toggling the antenna between BT and WLAN. If the driver requests
 * disabling dynamic powersave, the @dynamic_ps_timeout value will be
 * temporarily set to zero until the driver re-enables dynamic powersave.
 *
 * Driver informs U-APSD client support by enabling
 * %IEEE80211_HW_SUPPORTS_UAPSD flag. The mode is configured through the
 * uapsd paramater in conf_tx() operation. Hardware needs to send the QoS
 * Nullfunc frames and stay awake until the service period has ended. To
 * utilize U-APSD, dynamic powersave is disabled for voip AC and all frames
 * from that AC are transmitted with powersave enabled.
 *
 * Note: U-APSD client mode is not yet supported with
 * %IEEE80211_HW_PS_NULLFUNC_STACK.
 */

/**
 * DOC: Beacon filter support
 *
 * Some hardware have beacon filter support to reduce host cpu wakeups
 * which will reduce system power consumption. It usuallly works so that
 * the firmware creates a checksum of the beacon but omits all constantly
 * changing elements (TSF, TIM etc). Whenever the checksum changes the
 * beacon is forwarded to the host, otherwise it will be just dropped. That
 * way the host will only receive beacons where some relevant information
 * (for example ERP protection or WMM settings) have changed.
 *
 * Beacon filter support is advertised with the %IEEE80211_HW_BEACON_FILTER
 * hardware capability. The driver needs to enable beacon filter support
 * whenever power save is enabled, that is %IEEE80211_CONF_PS is set. When
 * power save is enabled, the stack will not check for beacon loss and the
 * driver needs to notify about loss of beacons with ieee80211_beacon_loss().
 *
 * The time (or number of beacons missed) until the firmware notifies the
 * driver of a beacon loss event (which in turn causes the driver to call
 * ieee80211_beacon_loss()) should be configurable and will be controlled
 * by mac80211 and the roaming algorithm in the future.
 *
 * Since there may be constantly changing information elements that nothing
 * in the software stack cares about, we will, in the future, have mac80211
 * tell the driver which information elements are interesting in the sense
 * that we want to see changes in them. This will include
 *  - a list of information element IDs
 *  - a list of OUIs for the vendor information element
 *
 * Ideally, the hardware would filter out any beacons without changes in the
 * requested elements, but if it cannot support that it may, at the expense
 * of some efficiency, filter out only a subset. For example, if the device
 * doesn't support checking for OUIs it should pass up all changes in all
 * vendor information elements.
 *
 * Note that change, for the sake of simplification, also includes information
 * elements appearing or disappearing from the beacon.
 *
 * Some hardware supports an "ignore list" instead, just make sure nothing
 * that was requested is on the ignore list, and include commonly changing
 * information element IDs in the ignore list, for example 11 (BSS load) and
 * the various vendor-assigned IEs with unknown contents (128, 129, 133-136,
 * 149, 150, 155, 156, 173, 176, 178, 179, 219); for forward compatibility
 * it could also include some currently unused IDs.
 *
 *
 * In addition to these capabilities, hardware should support notifying the
 * host of changes in the beacon RSSI. This is relevant to implement roaming
 * when no traffic is flowing (when traffic is flowing we see the RSSI of
 * the received data packets). This can consist in notifying the host when
 * the RSSI changes significantly or when it drops below or rises above
 * configurable thresholds. In the future these thresholds will also be
 * configured by mac80211 (which gets them from userspace) to implement
 * them as the roaming algorithm requires.
 *
 * If the hardware cannot implement this, the driver should ask it to
 * periodically pass beacon frames to the host so that software can do the
 * signal strength threshold checking.
 */

/**
 * DOC: Spatial multiplexing power save
 *
 * SMPS (Spatial multiplexing power save) is a mechanism to conserve
 * power in an 802.11n implementation. For details on the mechanism
 * and rationale, please refer to 802.11 (as amended by 802.11n-2009)
 * "11.2.3 SM power save".
 *
 * The mac80211 implementation is capable of sending action frames
 * to update the AP about the station's SMPS mode, and will instruct
 * the driver to enter the specific mode. It will also announce the
 * requested SMPS mode during the association handshake. Hardware
 * support for this feature is required, and can be indicated by
 * hardware flags.
 *
 * The default mode will be "automatic", which nl80211/cfg80211
 * defines to be dynamic SMPS in (regular) powersave, and SMPS
 * turned off otherwise.
 *
 * To support this feature, the driver must set the appropriate
 * hardware support flags, and handle the SMPS flag to the config()
 * operation. It will then with this mechanism be instructed to
 * enter the requested SMPS mode while associated to an HT AP.
 */

/**
 * DOC: Frame filtering
 *
 * mac80211 requires to see many management frames for proper
 * operation, and users may want to see many more frames when
 * in monitor mode. However, for best CPU usage and power consumption,
 * having as few frames as possible percolate through the stack is
 * desirable. Hence, the hardware should filter as much as possible.
 *
 * To achieve this, mac80211 uses filter flags (see below) to tell
 * the driver's configure_filter() function which frames should be
 * passed to mac80211 and which should be filtered out.
 *
 * Before configure_filter() is invoked, the prepare_multicast()
 * callback is invoked with the parameters @mc_count and @mc_list
 * for the combined multicast address list of all virtual interfaces.
 * It's use is optional, and it returns a u64 that is passed to
 * configure_filter(). Additionally, configure_filter() has the
 * arguments @changed_flags telling which flags were changed and
 * @total_flags with the new flag states.
 *
 * If your device has no multicast address filters your driver will
 * need to check both the %FIF_ALLMULTI flag and the @mc_count
 * parameter to see whether multicast frames should be accepted
 * or dropped.
 *
 * All unsupported flags in @total_flags must be cleared.
 * Hardware does not support a flag if it is incapable of _passing_
 * the frame to the stack. Otherwise the driver must ignore
 * the flag, but not clear it.
 * You must _only_ clear the flag (announce no support for the
 * flag to mac80211) if you are not able to pass the packet type
 * to the stack (so the hardware always filters it).
 * So for example, you should clear @FIF_CONTROL, if your hardware
 * always filters control frames. If your hardware always passes
 * control frames to the kernel and is incapable of filtering them,
 * you do _not_ clear the @FIF_CONTROL flag.
 * This rule applies to all other FIF flags as well.
 */

/**
 * DOC: AP support for powersaving clients
 *
 * In order to implement AP and P2P GO modes, mac80211 has support for
 * client powersaving, both "legacy" PS (PS-Poll/null data) and uAPSD.
 * There currently is no support for sAPSD.
 *
 * There is one assumption that mac80211 makes, namely that a client
 * will not poll with PS-Poll and trigger with uAPSD at the same time.
 * Both are supported, and both can be used by the same client, but
 * they can't be used concurrently by the same client. This simplifies
 * the driver code.
 *
 * The first thing to keep in mind is that there is a flag for complete
 * driver implementation: %IEEE80211_HW_AP_LINK_PS. If this flag is set,
 * mac80211 expects the driver to handle most of the state machine for
 * powersaving clients and will ignore the PM bit in incoming frames.
 * Drivers then use ieee80211_sta_ps_transition() to inform mac80211 of
 * stations' powersave transitions. In this mode, mac80211 also doesn't
 * handle PS-Poll/uAPSD.
 *
 * In the mode without %IEEE80211_HW_AP_LINK_PS, mac80211 will check the
 * PM bit in incoming frames for client powersave transitions. When a
 * station goes to sleep, we will stop transmitting to it. There is,
 * however, a race condition: a station might go to sleep while there is
 * data buffered on hardware queues. If the device has support for this
 * it will reject frames, and the driver should give the frames back to
 * mac80211 with the %IEEE80211_TX_STAT_TX_FILTERED flag set which will
 * cause mac80211 to retry the frame when the station wakes up. The
 * driver is also notified of powersave transitions by calling its
 * @sta_notify callback.
 *
 * When the station is asleep, it has three choices: it can wake up,
 * it can PS-Poll, or it can possibly start a uAPSD service period.
 * Waking up is implemented by simply transmitting all buffered (and
 * filtered) frames to the station. This is the easiest case. When
 * the station sends a PS-Poll or a uAPSD trigger frame, mac80211
 * will inform the driver of this with the @allow_buffered_frames
 * callback; this callback is optional. mac80211 will then transmit
 * the frames as usual and set the %IEEE80211_TX_CTL_POLL_RESPONSE
 * on each frame. The last frame in the service period (or the only
 * response to a PS-Poll) also has %IEEE80211_TX_STATUS_EOSP set to
 * indicate that it ends the service period; as this frame must have
 * TX status report it also sets %IEEE80211_TX_CTL_REQ_TX_STATUS.
 * When TX status is reported for this frame, the service period is
 * marked has having ended and a new one can be started by the peer.
 *
 * Another race condition can happen on some devices like iwlwifi
 * when there are frames queued for the station and it wakes up
 * or polls; the frames that are already queued could end up being
 * transmitted first instead, causing reordering and/or wrong
 * processing of the EOSP. The cause is that allowing frames to be
 * transmitted to a certain station is out-of-band communication to
 * the device. To allow this problem to be solved, the driver can
 * call ieee80211_sta_block_awake() if frames are buffered when it
 * is notified that the station went to sleep. When all these frames
 * have been filtered (see above), it must call the function again
 * to indicate that the station is no longer blocked.
 *
 * If the driver buffers frames in the driver for aggregation in any
 * way, it must use the ieee80211_sta_set_buffered() call when it is
 * notified of the station going to sleep to inform mac80211 of any
 * TIDs that have frames buffered. Note that when a station wakes up
 * this information is reset (hence the requirement to call it when
 * informed of the station going to sleep). Then, when a service
 * period starts for any reason, @release_buffered_frames is called
 * with the number of frames to be released and which TIDs they are
 * to come from. In this case, the driver is responsible for setting
 * the EOSP (for uAPSD) and MORE_DATA bits in the released frames,
 * to help the @more_data paramter is passed to tell the driver if
 * there is more data on other TIDs -- the TIDs to release frames
 * from are ignored since mac80211 doesn't know how many frames the
 * buffers for those TIDs contain.
 *
 * If the driver also implement GO mode, where absence periods may
 * shorten service periods (or abort PS-Poll responses), it must
 * filter those response frames except in the case of frames that
 * are buffered in the driver -- those must remain buffered to avoid
 * reordering. Because it is possible that no frames are released
 * in this case, the driver must call ieee80211_sta_eosp_irqsafe()
 * to indicate to mac80211 that the service period ended anyway.
 *
 * Finally, if frames from multiple TIDs are released from mac80211
 * but the driver might reorder them, it must clear & set the flags
 * appropriately (only the last frame may have %IEEE80211_TX_STATUS_EOSP)
 * and also take care of the EOSP and MORE_DATA bits in the frame.
 * The driver may also use ieee80211_sta_eosp_irqsafe() in this case.
 */

/**
 * DOC: HW queue control
 *
 * Before HW queue control was introduced, mac80211 only had a single static
 * assignment of per-interface AC software queues to hardware queues. This
 * was problematic for a few reasons:
 * 1) off-channel transmissions might get stuck behind other frames
 * 2) multiple virtual interfaces couldn't be handled correctly
 * 3) after-DTIM frames could get stuck behind other frames
 *
 * To solve this, hardware typically uses multiple different queues for all
 * the different usages, and this needs to be propagated into mac80211 so it
 * won't have the same problem with the software queues.
 *
 * Therefore, mac80211 now offers the %IEEE80211_HW_QUEUE_CONTROL capability
 * flag that tells it that the driver implements its own queue control. To do
 * so, the driver will set up the various queues in each &struct ieee80211_vif
 * and the offchannel queue in &struct ieee80211_hw. In response, mac80211 will
 * use those queue IDs in the hw_queue field of &struct ieee80211_tx_info and
 * if necessary will queue the frame on the right software queue that mirrors
 * the hardware queue.
 * Additionally, the driver has to then use these HW queue IDs for the queue
 * management functions (ieee80211_stop_queue() et al.)
 *
 * The driver is free to set up the queue mappings as needed, multiple virtual
 * interfaces may map to the same hardware queues if needed. The setup has to
 * happen during add_interface or change_interface callbacks. For example, a
 * driver supporting station+station and station+AP modes might decide to have
 * 10 hardware queues to handle different scenarios:
 *
 * 4 AC HW queues for 1st vif: 0, 1, 2, 3
 * 4 AC HW queues for 2nd vif: 4, 5, 6, 7
 * after-DTIM queue for AP:   8
 * off-channel queue:         9
 *
 * It would then set up the hardware like this:
 *   hw.offchannel_tx_hw_queue = 9
 *
 * and the first virtual interface that is added as follows:
 *   vif.hw_queue[IEEE80211_AC_VO] = 0
 *   vif.hw_queue[IEEE80211_AC_VI] = 1
 *   vif.hw_queue[IEEE80211_AC_BE] = 2
 *   vif.hw_queue[IEEE80211_AC_BK] = 3
 *   vif.cab_queue = 8 // if AP mode, otherwise %IEEE80211_INVAL_HW_QUEUE
 * and the second virtual interface with 4-7.
 *
 * If queue 6 gets full, for example, mac80211 would only stop the second
 * virtual interface's BE queue since virtual interface queues are per AC.
 *
 * Note that the vif.cab_queue value should be set to %IEEE80211_INVAL_HW_QUEUE
 * whenever the queue is not used (i.e. the interface is not in AP mode) if the
 * queue could potentially be shared since mac80211 will look at cab_queue when
 * a queue is stopped/woken even if the interface is not in AP mode.
 */

/**
 * enum ieee80211_filter_flags - hardware filter flags
 *
 * These flags determine what the filter in hardware should be
 * programmed to let through and what should not be passed to the
 * stack. It is always safe to pass more frames than requested,
 * but this has negative impact on power consumption.
 *
 * @FIF_PROMISC_IN_BSS: promiscuous mode within your BSS,
 *	think of the BSS as your network segment and then this corresponds
 *	to the regular ethernet device promiscuous mode.
 *
 * @FIF_ALLMULTI: pass all multicast frames, this is used if requested
 *	by the user or if the hardware is not capable of filtering by
 *	multicast address.
 *
 * @FIF_FCSFAIL: pass frames with failed FCS (but you need to set the
 *	%RX_FLAG_FAILED_FCS_CRC for them)
 *
 * @FIF_PLCPFAIL: pass frames with failed PLCP CRC (but you need to set
 *	the %RX_FLAG_FAILED_PLCP_CRC for them
 *
 * @FIF_BCN_PRBRESP_PROMISC: This flag is set during scanning to indicate
 *	to the hardware that it should not filter beacons or probe responses
 *	by BSSID. Filtering them can greatly reduce the amount of processing
 *	mac80211 needs to do and the amount of CPU wakeups, so you should
 *	honour this flag if possible.
 *
 * @FIF_CONTROL: pass control frames (except for PS Poll), if PROMISC_IN_BSS
 * 	is not set then only those addressed to this station.
 *
 * @FIF_OTHER_BSS: pass frames destined to other BSSes
 *
 * @FIF_PSPOLL: pass PS Poll frames, if PROMISC_IN_BSS is not set then only
 * 	those addressed to this station.
 *
 * @FIF_PROBE_REQ: pass probe request frames
 */
enum ieee80211_filter_flags {
	FIF_PROMISC_IN_BSS	= 1<<0,
	FIF_ALLMULTI		= 1<<1,
	FIF_FCSFAIL		= 1<<2,
	FIF_PLCPFAIL		= 1<<3,
	FIF_BCN_PRBRESP_PROMISC	= 1<<4,
	FIF_CONTROL		= 1<<5,
	FIF_OTHER_BSS		= 1<<6,
	FIF_PSPOLL		= 1<<7,
	FIF_PROBE_REQ		= 1<<8,
};

/**
 * enum ieee80211_ampdu_mlme_action - A-MPDU actions
 *
 * These flags are used with the ampdu_action() callback in
 * &struct ieee80211_ops to indicate which action is needed.
 *
 * Note that drivers MUST be able to deal with a TX aggregation
 * session being stopped even before they OK'ed starting it by
 * calling ieee80211_start_tx_ba_cb_irqsafe, because the peer
 * might receive the addBA frame and send a delBA right away!
 *
 * @IEEE80211_AMPDU_RX_START: start Rx aggregation
 * @IEEE80211_AMPDU_RX_STOP: stop Rx aggregation
 * @IEEE80211_AMPDU_TX_START: start Tx aggregation
 * @IEEE80211_AMPDU_TX_STOP: stop Tx aggregation
 * @IEEE80211_AMPDU_TX_OPERATIONAL: TX aggregation has become operational
 */
enum ieee80211_ampdu_mlme_action {
	IEEE80211_AMPDU_RX_START,
	IEEE80211_AMPDU_RX_STOP,
	IEEE80211_AMPDU_TX_START,
	IEEE80211_AMPDU_TX_STOP,
	IEEE80211_AMPDU_TX_OPERATIONAL,
};

/**
 * enum ieee80211_tx_sync_type - TX sync type
 * @IEEE80211_TX_SYNC_AUTH: sync TX for authentication
 *	(and possibly also before direct probe)
 * @IEEE80211_TX_SYNC_ASSOC: sync TX for association
 * @IEEE80211_TX_SYNC_ACTION: sync TX for action frame
 *	(not implemented yet)
 */
enum ieee80211_tx_sync_type {
	IEEE80211_TX_SYNC_AUTH,
	IEEE80211_TX_SYNC_ASSOC,
	IEEE80211_TX_SYNC_ACTION,
};

/**
 * enum ieee80211_frame_release_type - frame release reason
 * @IEEE80211_FRAME_RELEASE_PSPOLL: frame released for PS-Poll
 * @IEEE80211_FRAME_RELEASE_UAPSD: frame(s) released due to
 *	frame received on trigger-enabled AC
 */
enum ieee80211_frame_release_type {
	IEEE80211_FRAME_RELEASE_PSPOLL,
	IEEE80211_FRAME_RELEASE_UAPSD,
};
#ifdef CONFIG_IEEE80211_SPECIAL_FILTER
#define IEEE80211_MAX_FILTERS		(16)
struct ieee80211_special_filter{
	u8   filter_action;
	u8   oui[3];
	u32  flags; 
};
enum ieee80211_special_filter_flags{
	SPECIAL_F_FLAGS_FRAME_TYPE = BIT(0),
	SPECIAL_F_FLAGS_FRAME_OUI  = BIT(1),
	SPECIAL_F_FLAGS_FRAME_IE   = BIT(2),
};
struct ieee80211_special_filter_list{
	struct list_head list;
	struct ieee80211_special_filter filter;
};
struct ieee80211_special_filter_table{
	u32 n_filters;
	struct ieee80211_special_filter table[IEEE80211_MAX_FILTERS];
};
#define IEEE80211_SPECIAL_FILTER_MASK	(SPECIAL_F_FLAGS_FRAME_TYPE | SPECIAL_F_FLAGS_FRAME_OUI | SPECIAL_F_FLAGS_FRAME_IE)
#endif
/**
 * struct ieee80211_ops - callbacks from mac80211 to the driver
 *
 * This structure contains various callbacks that the driver may
 * handle or, in some cases, must handle, for example to configure
 * the hardware to a new channel or to transmit a frame.
 *
 * @tx: Handler that 802.11 module calls for each transmitted frame.
 *	skb contains the buffer starting from the IEEE 802.11 header.
 *	The low-level driver should send the frame out based on
 *	configuration in the TX control data. This handler should,
 *	preferably, never fail and stop queues appropriately, more
 *	importantly, however, it must never fail for A-MPDU-queues.
 *	This function should return NETDEV_TX_OK except in very
 *	limited cases.
 *	Must be implemented and atomic.
 *
 * @start: Called before the first netdevice attached to the hardware
 *	is enabled. This should turn on the hardware and must turn on
 *	frame reception (for possibly enabled monitor interfaces.)
 *	Returns negative error codes, these may be seen in userspace,
 *	or zero.
 *	When the device is started it should not have a MAC address
 *	to avoid acknowledging frames before a non-monitor device
 *	is added.
 *	Must be implemented and can sleep.
 *
 * @stop: Called after last netdevice attached to the hardware
 *	is disabled. This should turn off the hardware (at least
 *	it must turn off frame reception.)
 *	May be called right after add_interface if that rejects
 *	an interface. If you added any work onto the mac80211 workqueue
 *	you should ensure to cancel it on this callback.
 *	Must be implemented and can sleep.
 *
 * @suspend: Suspend the device; mac80211 itself will quiesce before and
 *	stop transmitting and doing any other configuration, and then
 *	ask the device to suspend. This is only invoked when WoWLAN is
 *	configured, otherwise the device is deconfigured completely and
 *	reconfigured at resume time.
 *	The driver may also impose special conditions under which it
 *	wants to use the "normal" suspend (deconfigure), say if it only
 *	supports WoWLAN when the device is associated. In this case, it
 *	must return 1 from this function.
 *
 * @resume: If WoWLAN was configured, this indicates that mac80211 is
 *	now resuming its operation, after this the device must be fully
 *	functional again. If this returns an error, the only way out is
 *	to also unregister the device. If it returns 1, then mac80211
 *	will also go through the regular complete restart on resume.
 *
 * @add_interface: Called when a netdevice attached to the hardware is
 *	enabled. Because it is not called for monitor mode devices, @start
 *	and @stop must be implemented.
 *	The driver should perform any initialization it needs before
 *	the device can be enabled. The initial configuration for the
 *	interface is given in the conf parameter.
 *	The callback may refuse to add an interface by returning a
 *	negative error code (which will be seen in userspace.)
 *	Must be implemented and can sleep.
 *
 * @change_interface: Called when a netdevice changes type. This callback
 *	is optional, but only if it is supported can interface types be
 *	switched while the interface is UP. The callback may sleep.
 *	Note that while an interface is being switched, it will not be
 *	found by the interface iteration callbacks.
 *
 * @remove_interface: Notifies a driver that an interface is going down.
 *	The @stop callback is called after this if it is the last interface
 *	and no monitor interfaces are present.
 *	When all interfaces are removed, the MAC address in the hardware
 *	must be cleared so the device no longer acknowledges packets,
 *	the mac_addr member of the conf structure is, however, set to the
 *	MAC address of the device going away.
 *	Hence, this callback must be implemented. It can sleep.
 *
 * @config: Handler for configuration requests. IEEE 802.11 code calls this
 *	function to change hardware configuration, e.g., channel.
 *	This function should never fail but returns a negative error code
 *	if it does. The callback can sleep.
 *
 * @bss_info_changed: Handler for configuration requests related to BSS
 *	parameters that may vary during BSS's lifespan, and may affect low
 *	level driver (e.g. assoc/disassoc status, erp parameters).
 *	This function should not be used if no BSS has been set, unless
 *	for association indication. The @changed parameter indicates which
 *	of the bss parameters has changed when a call is made. The callback
 *	can sleep.
 *
 * @tx_sync: Called before a frame is sent to an AP/GO. In the GO case, the
 *	driver should sync with the GO's powersaving so the device doesn't
 *	transmit the frame while the GO is asleep. In the regular AP case
 *	it may be used by drivers for devices implementing other restrictions
 *	on talking to APs, e.g. due to regulatory enforcement or just HW
 *	restrictions.
 *	This function is called for every authentication, association and
 *	action frame separately since applications might attempt to auth
 *	with multiple APs before chosing one to associate to. If it returns
 *	an error, the corresponding authentication, association or frame
 *	transmission is aborted and reported as having failed. It is always
 *	called after tuning to the correct channel.
 *	The callback might be called multiple times before @finish_tx_sync
 *	(but @finish_tx_sync will be called once for each) but in practice
 *	this is unlikely to happen. It can also refuse in that case if the
 *	driver cannot handle that situation.
 *	This callback can sleep.
 * @finish_tx_sync: Called as a counterpart to @tx_sync, unless that returned
 *	an error. This callback can sleep.
 *
 * @prepare_multicast: Prepare for multicast filter configuration.
 *	This callback is optional, and its return value is passed
 *	to configure_filter(). This callback must be atomic.
 *
 * @configure_filter: Configure the device's RX filter.
 *	See the section "Frame filtering" for more information.
 *	This callback must be implemented and can sleep.
 *
 * @set_tim: Set TIM bit. mac80211 calls this function when a TIM bit
 * 	must be set or cleared for a given STA. Must be atomic.
 *
 * @set_key: See the section "Hardware crypto acceleration"
 *	This callback is only called between add_interface and
 *	remove_interface calls, i.e. while the given virtual interface
 *	is enabled.
 *	Returns a negative error code if the key can't be added.
 *	The callback can sleep.
 *
 * @update_tkip_key: See the section "Hardware crypto acceleration"
 * 	This callback will be called in the context of Rx. Called for drivers
 * 	which set IEEE80211_KEY_FLAG_TKIP_REQ_RX_P1_KEY.
 *	The callback must be atomic.
 *
 * @set_rekey_data: If the device supports GTK rekeying, for example while the
 *	host is suspended, it can assign this callback to retrieve the data
 *	necessary to do GTK rekeying, this is the KEK, KCK and replay counter.
 *	After rekeying was done it should (for example during resume) notify
 *	userspace of the new replay counter using ieee80211_gtk_rekey_notify().
 *
 * @hw_scan: Ask the hardware to service the scan request, no need to start
 *	the scan state machine in stack. The scan must honour the channel
 *	configuration done by the regulatory agent in the wiphy's
 *	registered bands. The hardware (or the driver) needs to make sure
 *	that power save is disabled.
 *	The @req ie/ie_len members are rewritten by mac80211 to contain the
 *	entire IEs after the SSID, so that drivers need not look at these
 *	at all but just send them after the SSID -- mac80211 includes the
 *	(extended) supported rates and HT information (where applicable).
 *	When the scan finishes, ieee80211_scan_completed() must be called;
 *	note that it also must be called when the scan cannot finish due to
 *	any error unless this callback returned a negative error code.
 *	The callback can sleep.
 *
 * @cancel_hw_scan: Ask the low-level tp cancel the active hw scan.
 *	The driver should ask the hardware to cancel the scan (if possible),
 *	but the scan will be completed only after the driver will call
 *	ieee80211_scan_completed().
 *	This callback is needed for wowlan, to prevent enqueueing a new
 *	scan_work after the low-level driver was already suspended.
 *	The callback can sleep.
 *
 * @sched_scan_start: Ask the hardware to start scanning repeatedly at
 *	specific intervals.  The driver must call the
 *	ieee80211_sched_scan_results() function whenever it finds results.
 *	This process will continue until sched_scan_stop is called.
 *
 * @sched_scan_stop: Tell the hardware to stop an ongoing scheduled scan.
 *
 * @sw_scan_start: Notifier function that is called just before a software scan
 *	is started. Can be NULL, if the driver doesn't need this notification.
 *	The callback can sleep.
 *
 * @sw_scan_complete: Notifier function that is called just after a
 *	software scan finished. Can be NULL, if the driver doesn't need
 *	this notification.
 *	The callback can sleep.
 *
 * @get_stats: Return low-level statistics.
 * 	Returns zero if statistics are available.
 *	The callback can sleep.
 *
 * @get_tkip_seq: If your device implements TKIP encryption in hardware this
 *	callback should be provided to read the TKIP transmit IVs (both IV32
 *	and IV16) for the given key from hardware.
 *	The callback must be atomic.
 *
 * @set_frag_threshold: Configuration of fragmentation threshold. Assign this
 *	if the device does fragmentation by itself; if this callback is
 *	implemented then the stack will not do fragmentation.
 *	The callback can sleep.
 *
 * @set_rts_threshold: Configuration of RTS threshold (if device needs it)
 *	The callback can sleep.
 *
 * @sta_add: Notifies low level driver about addition of an associated station,
 *	AP, IBSS/WDS/mesh peer etc. This callback can sleep.
 *
 * @sta_remove: Notifies low level driver about removal of an associated
 *	station, AP, IBSS/WDS/mesh peer etc. This callback can sleep.
 *
 * @sta_notify: Notifies low level driver about power state transition of an
 *	associated station, AP,  IBSS/WDS/mesh peer etc. For a VIF operating
 *	in AP mode, this callback will not be called when the flag
 *	%IEEE80211_HW_AP_LINK_PS is set. Must be atomic.
 *
 * @conf_tx: Configure TX queue parameters (EDCF (aifs, cw_min, cw_max),
 *	bursting) for a hardware TX queue.
 *	Returns a negative error code on failure.
 *	The callback can sleep.
 *
 * @get_tsf: Get the current TSF timer value from firmware/hardware. Currently,
 *	this is only used for IBSS mode BSSID merging and debugging. Is not a
 *	required function.
 *	The callback can sleep.
 *
 * @set_tsf: Set the TSF timer to the specified value in the firmware/hardware.
 *      Currently, this is only used for IBSS mode debugging. Is not a
 *	required function.
 *	The callback can sleep.
 *
 * @reset_tsf: Reset the TSF timer and allow firmware/hardware to synchronize
 *	with other STAs in the IBSS. This is only used in IBSS mode. This
 *	function is optional if the firmware/hardware takes full care of
 *	TSF synchronization.
 *	The callback can sleep.
 *
 * @tx_last_beacon: Determine whether the last IBSS beacon was sent by us.
 *	This is needed only for IBSS mode and the result of this function is
 *	used to determine whether to reply to Probe Requests.
 *	Returns non-zero if this device sent the last beacon.
 *	The callback can sleep.
 *
 * @ampdu_action: Perform a certain A-MPDU action
 * 	The RA/TID combination determines the destination and TID we want
 * 	the ampdu action to be performed for. The action is defined through
 * 	ieee80211_ampdu_mlme_action. Starting sequence number (@ssn)
 * 	is the first frame we expect to perform the action on. Notice
 * 	that TX/RX_STOP can pass NULL for this parameter.
 *	The @buf_size parameter is only valid when the action is set to
 *	%IEEE80211_AMPDU_TX_OPERATIONAL and indicates the peer's reorder
 *	buffer size (number of subframes) for this session -- the driver
 *	may neither send aggregates containing more subframes than this
 *	nor send aggregates in a way that lost frames would exceed the
 *	buffer size. If just limiting the aggregate size, this would be
 *	possible with a buf_size of 8:
 *	 - TX: 1.....7
 *	 - RX:  2....7 (lost frame #1)
 *	 - TX:        8..1...
 *	which is invalid since #1 was now re-transmitted well past the
 *	buffer size of 8. Correct ways to retransmit #1 would be:
 *	 - TX:       1 or 18 or 81
 *	Even "189" would be wrong since 1 could be lost again.
 *
 *	Returns a negative error code on failure.
 *	The callback can sleep.
 *
 * @get_survey: Return per-channel survey information
 *
 * @rfkill_poll: Poll rfkill hardware state. If you need this, you also
 *	need to set wiphy->rfkill_poll to %true before registration,
 *	and need to call wiphy_rfkill_set_hw_state() in the callback.
 *	The callback can sleep.
 *
 * @set_coverage_class: Set slot time for given coverage class as specified
 *	in IEEE 802.11-2007 section 17.3.8.6 and modify ACK timeout
 *	accordingly. This callback is not required and may sleep.
 *
 * @testmode_cmd: Implement a cfg80211 test mode command.
 *	The callback can sleep.
 * @testmode_dump: Implement a cfg80211 test mode dump. The callback can sleep.
 *
 * @flush: Flush all pending frames from the hardware queue, making sure
 *	that the hardware queues are empty. If the parameter @drop is set
 *	to %true, pending frames may be dropped. The callback can sleep.
 *
 * @channel_switch: Drivers that need (or want) to offload the channel
 *	switch operation for CSAs received from the AP may implement this
 *	callback. They must then call ieee80211_chswitch_done() to indicate
 *	completion of the channel switch.
 *
 * @napi_poll: Poll Rx queue for incoming data frames.
 *
 * @set_antenna: Set antenna configuration (tx_ant, rx_ant) on the device.
 *	Parameters are bitmaps of allowed antennas to use for TX/RX. Drivers may
 *	reject TX/RX mask combinations they cannot support by returning -EINVAL
 *	(also see nl80211.h @NL80211_ATTR_WIPHY_ANTENNA_TX).
 *
 * @get_antenna: Get current antenna configuration from device (tx_ant, rx_ant).
 *
 * @remain_on_channel: Starts an off-channel period on the given channel, must
 *	call back to ieee80211_ready_on_channel() when on that channel. Note
 *	that normal channel traffic is not stopped as this is intended for hw
 *	offload. Frames to transmit on the off-channel channel are transmitted
 *	normally except for the %IEEE80211_TX_CTL_TX_OFFCHAN flag. When the
 *	duration (which will always be non-zero) expires, the driver must call
 *	ieee80211_remain_on_channel_expired(). This callback may sleep.
 * @cancel_remain_on_channel: Requests that an ongoing off-channel period is
 *	aborted before it expires. This callback may sleep.
 *
 * @set_ringparam: Set tx and rx ring sizes.
 *
 * @get_ringparam: Get tx and rx ring current and maximum sizes.
 *
 * @tx_frames_pending: Check if there is any pending frame in the hardware
 *	queues before entering power save.
 *
 * @set_bitrate_mask: Set a mask of rates to be used for rate control selection
 *	when transmitting a frame. Currently only legacy rates are handled.
 *	The callback can sleep.
 * @rssi_callback: Notify driver when the average RSSI goes above/below
 *	thresholds that were registered previously. The callback can sleep.
 *
 * @release_buffered_frames: Release buffered frames according to the given
 *	parameters. In the case where the driver buffers some frames for
 *	sleeping stations mac80211 will use this callback to tell the driver
 *	to release some frames, either for PS-poll or uAPSD.
 *	Note that if the @more_data paramter is %false the driver must check
 *	if there are more frames on the given TIDs, and if there are more than
 *	the frames being released then it must still set the more-data bit in
 *	the frame. If the @more_data parameter is %true, then of course the
 *	more-data bit must always be set.
 *	The @tids parameter tells the driver which TIDs to release frames
 *	from, for PS-poll it will always have only a single bit set.
 *	In the case this is used for a PS-poll initiated release, the
 *	@num_frames parameter will always be 1 so code can be shared. In
 *	this case the driver must also set %IEEE80211_TX_STATUS_EOSP flag
 *	on the TX status (and must report TX status) so that the PS-poll
 *	period is properly ended. This is used to avoid sending multiple
 *	responses for a retried PS-poll frame.
 *	In the case this is used for uAPSD, the @num_frames parameter may be
 *	bigger than one, but the driver may send fewer frames (it must send
 *	at least one, however). In this case it is also responsible for
 *	setting the EOSP flag in the QoS header of the frames. Also, when the
 *	service period ends, the driver must set %IEEE80211_TX_STATUS_EOSP
 *	on the last frame in the SP. Alternatively, it may call the function
 *	ieee80211_sta_eosp_irqsafe() to inform mac80211 of the end of the SP.
 *	This callback must be atomic.
 * @allow_buffered_frames: Prepare device to allow the given number of frames
 *	to go out to the given station. The frames will be sent by mac80211
 *	via the usual TX path after this call. The TX information for frames
 *	released will also have the %IEEE80211_TX_CTL_POLL_RESPONSE flag set
 *	and the last one will also have %IEEE80211_TX_STATUS_EOSP set. In case
 *	frames from multiple TIDs are released and the driver might reorder
 *	them between the TIDs, it must set the %IEEE80211_TX_STATUS_EOSP flag
 *	on the last frame and clear it on all others and also handle the EOSP
 *	bit in the QoS header correctly. Alternatively, it can also call the
 *	ieee80211_sta_eosp_irqsafe() function.
 *	The @tids parameter is a bitmap and tells the driver which TIDs the
 *	frames will be on; it will at most have two bits set.
 *	This callback must be atomic.
 * @set_data_filter: Prepare device to configure data packet filter and filter
 *	data packetsi accoding to its type. The filter can be configured
 *	as multicast, ipv4, ipv6i, mac address, dhcp & ssdp
 */
struct ieee80211_ops {
	void (*tx)(struct ieee80211_hw *hw, struct sk_buff *skb);
	int (*start)(struct ieee80211_hw *hw);
	void (*stop)(struct ieee80211_hw *hw);
#ifdef CONFIG_PM
	int (*suspend)(struct ieee80211_hw *hw, struct cfg80211_wowlan *wowlan);
	int (*resume)(struct ieee80211_hw *hw);
#endif
	int (*add_interface)(struct ieee80211_hw *hw,
			     struct ieee80211_vif *vif);
	int (*change_interface)(struct ieee80211_hw *hw,
				struct ieee80211_vif *vif,
				enum nl80211_iftype new_type, bool p2p);
	void (*remove_interface)(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif);
	int (*config)(struct ieee80211_hw *hw, u32 changed);
	void (*bss_info_changed)(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
				 struct ieee80211_bss_conf *info,
				 u32 changed);

	int (*tx_sync)(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		       const u8 *bssid, enum ieee80211_tx_sync_type type);
	void (*finish_tx_sync)(struct ieee80211_hw *hw,
			       struct ieee80211_vif *vif,
			       const u8 *bssid,
			       enum ieee80211_tx_sync_type type);

	u64 (*prepare_multicast)(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
				 struct netdev_hw_addr_list *mc_list);
#else
				 int mc_count, struct dev_addr_list *mc_list);
#endif
	void (*configure_filter)(struct ieee80211_hw *hw,
				 struct ieee80211_vif *vif,
				 unsigned int changed_flags,
				 unsigned int *total_flags,
				 u64 multicast);
	int (*set_tim)(struct ieee80211_hw *hw, struct ieee80211_sta *sta,
		       bool set);
	int (*set_key)(struct ieee80211_hw *hw, enum set_key_cmd cmd,
		       struct ieee80211_vif *vif, struct ieee80211_sta *sta,
		       struct ieee80211_key_conf *key);
	void (*update_tkip_key)(struct ieee80211_hw *hw,
				struct ieee80211_vif *vif,
				struct ieee80211_key_conf *conf,
				struct ieee80211_sta *sta,
				u32 iv32, u16 *phase1key);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 0, 8))
	void (*set_rekey_data)(struct ieee80211_hw *hw,
			       struct ieee80211_vif *vif,
			       int enable);
#endif
	int (*hw_scan)(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		       struct ieee80211_scan_req_wrap *req);
	void (*cancel_hw_scan)(struct ieee80211_hw *hw,
			       struct ieee80211_vif *vif);
	int (*sched_scan_start)(struct ieee80211_hw *hw,
				struct ieee80211_vif *vif,
				struct cfg80211_sched_scan_request *req,
				struct ieee80211_sched_scan_ies *ies);
	void (*sched_scan_stop)(struct ieee80211_hw *hw,
			       struct ieee80211_vif *vif);
	void (*sw_scan_start)(struct ieee80211_hw *hw);
	void (*sw_scan_complete)(struct ieee80211_hw *hw);
	int (*get_stats)(struct ieee80211_hw *hw,
			 struct ieee80211_low_level_stats *stats);
	void (*get_tkip_seq)(struct ieee80211_hw *hw, u8 hw_key_idx,
			     u32 *iv32, u16 *iv16);
	int (*set_frag_threshold)(struct ieee80211_hw *hw, u32 value);
	int (*set_rts_threshold)(struct ieee80211_hw *hw,
			struct ieee80211_vif *vif, u32 value);
	int (*sta_add)(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		       struct ieee80211_sta *sta);
	int (*sta_remove)(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			  struct ieee80211_sta *sta);
	void (*sta_notify)(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			enum sta_notify_cmd, struct ieee80211_sta *sta);
	int (*conf_tx)(struct ieee80211_hw *hw,
		       struct ieee80211_vif *vif, u16 ac,
		       const struct ieee80211_tx_queue_params *params);
	u64 (*get_tsf)(struct ieee80211_hw *hw, struct ieee80211_vif *vif);
	void (*set_tsf)(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			u64 tsf);
	void (*reset_tsf)(struct ieee80211_hw *hw, struct ieee80211_vif *vif);
	int (*tx_last_beacon)(struct ieee80211_hw *hw);
	int (*ampdu_action)(struct ieee80211_hw *hw,
			    struct ieee80211_vif *vif,
			    enum ieee80211_ampdu_mlme_action action,
			    struct ieee80211_sta *sta, u16 tid, u16 *ssn,
			    u8 buf_size, u8 hw_token);
	int (*get_survey)(struct ieee80211_hw *hw, int idx,
		struct survey_info *survey);
	void (*rfkill_poll)(struct ieee80211_hw *hw);
	void (*set_coverage_class)(struct ieee80211_hw *hw, u8 coverage_class);
#if defined (CONFIG_NL80211_TESTMODE) && defined (CONFIG_ATBM_TEST_TOOL)
	int (*testmode_cmd)(struct ieee80211_hw *hw, void *data, int len);
	int (*testmode_dump)(struct ieee80211_hw *hw, struct sk_buff *skb,
			     struct netlink_callback *cb,
			     void *data, int len);
#endif
	int (*altmtest_cmd)(struct ieee80211_hw *hw, void *data, int len);

	void (*flush)(struct ieee80211_hw *hw,
		      struct ieee80211_vif *vif,
		      bool drop);
	void (*channel_switch)(struct ieee80211_hw *hw,
			       struct ieee80211_channel_switch *ch_switch);
#ifdef IEEE80211_SUPPORT_NAPI
	int (*napi_poll)(struct ieee80211_hw *hw, int budget);
#endif
	int (*set_antenna)(struct ieee80211_hw *hw, u32 tx_ant, u32 rx_ant);
	int (*get_antenna)(struct ieee80211_hw *hw, u32 *tx_ant, u32 *rx_ant);

	int (*remain_on_channel)(struct ieee80211_hw *hw,
				struct ieee80211_vif *vif,
				 struct ieee80211_channel *chan,
				 enum nl80211_channel_type channel_type,
				 int duration, u64 cookie);
	int (*cancel_remain_on_channel)(struct ieee80211_hw *hw);
	int (*set_ringparam)(struct ieee80211_hw *hw, u32 tx, u32 rx);
	void (*get_ringparam)(struct ieee80211_hw *hw,
			      u32 *tx, u32 *tx_max, u32 *rx, u32 *rx_max);
	bool (*tx_frames_pending)(struct ieee80211_hw *hw);
	int (*set_bitrate_mask)(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
				const struct cfg80211_bitrate_mask *mask);
	void (*rssi_callback)(struct ieee80211_hw *hw,
			      enum ieee80211_rssi_event rssi_event);

	void (*allow_buffered_frames)(struct ieee80211_hw *hw,
				      struct ieee80211_sta *sta,
				      u16 tids, int num_frames,
				      enum ieee80211_frame_release_type reason,
				      bool more_data);
	void (*release_buffered_frames)(struct ieee80211_hw *hw,
					struct ieee80211_sta *sta,
					u16 tids, int num_frames,
					enum ieee80211_frame_release_type reason,
					bool more_data);
	void (*set_data_filter)(struct ieee80211_hw *hw,
				struct ieee80211_vif *vif,
				void *data,
				int len);
	int (*set_mac_addr2efuse)(struct ieee80211_hw *hw, u8 *macAddr);
	int (*sta_triger_listen)(struct ieee80211_hw *hw,struct ieee80211_vif *vif,struct ieee80211_channel *chan);
	int (*sta_stop_listen)(struct ieee80211_hw *hw,struct ieee80211_vif *vif);
#ifdef CONFIG_IEEE80211_SPECIAL_FILTER
	int (*set_frame_filter)(struct ieee80211_hw *hw,struct ieee80211_vif *vif,u32 n_filters,
				struct ieee80211_special_filter *filter_table,bool enable);
#endif
	int (*do_set_defualt_wep_key)(struct ieee80211_vif* vif, struct ieee80211_key_conf* key);
#ifdef CONFIG_ATBM_BLE
	int (*do_ble_xmit)(struct ieee80211_hw *hw,u8* xmit,size_t len);
#endif
};

enum ieee80211_ble_msg_type {
	BLE_MSG_TYPE_UNKOWN,
	BLE_MSG_TYPE_ACK,
	BLE_MSG_TYPE_EVT,
};
enum ieee80211_ble_dev_type {
	BLE_DEV_TYPE_NBLE,
	BLE_DEV_TYPE_BROADCAST,
	BLE_DEV_TYPE_BLUEDROID,
	BLE_DEV_TYPE_BLUEZ,
};
struct ieee80211_ble_status {
	size_t size;
	size_t hw_hdr_size;
	size_t nr;

	void   *context1;
	void   *context2;
};
static inline struct ieee80211_ble_status *IEEE80211_BLE_SKB_CB(struct sk_buff *skb)
{
	return (struct ieee80211_ble_status *)skb->cb;
}

#ifdef CONFIG_ATBM_BLE
void ieee80211_ble_dev_recv(struct ieee80211_hw *hw,u8 *event_buffer, u16 event_len);
void  ieee80211_ble_recv(struct ieee80211_hw *hw,struct sk_buff *skb);
int  ieee80211_ble_platform_init(void);
void  ieee80211_ble_platform_exit(void);
#ifdef CONFIG_ATBM_BLE_ADV_COEXIST

void atbm_ioctl_ble_adv_rpt_async(struct ieee80211_hw *hw,u8 *event_buffer, u16 event_len);
void atbm_ioctl_ble_conn_rpt_async(struct ieee80211_hw *hw,u8 *event_buffer, u16 event_len);

#endif
#endif
/**
 * ieee80211_alloc_hw -  Allocate a new hardware device
 *
 * This must be called once for each hardware device. The returned pointer
 * must be used to refer to this device when calling other functions.
 * mac80211 allocates a private data area for the driver pointed to by
 * @priv in &struct ieee80211_hw, the size of this area is given as
 * @priv_data_len.
 *
 * @priv_data_len: length of private data
 * @ops: callbacks for this device
 */
struct ieee80211_hw *ieee80211_alloc_hw(size_t priv_data_len,
					const struct ieee80211_ops *ops);

/**
 * ieee80211_register_hw - Register hardware device
 *
 * You must call this function before any other functions in
 * mac80211. Note that before a hardware can be registered, you
 * need to fill the contained wiphy's information.
 *
 * @hw: the device to register as returned by ieee80211_alloc_hw()
 */
int ieee80211_register_hw(struct ieee80211_hw *hw);

/**
 * struct ieee80211_tpt_blink - throughput blink description
 * @throughput: throughput in Kbit/sec
 * @blink_time: blink time in milliseconds
 *	(full cycle, ie. one off + one on period)
 */
struct ieee80211_tpt_blink {
	int throughput;
	int blink_time;
};

/**
 * enum ieee80211_tpt_led_trigger_flags - throughput trigger flags
 * @IEEE80211_TPT_LEDTRIG_FL_RADIO: enable blinking with radio
 * @IEEE80211_TPT_LEDTRIG_FL_WORK: enable blinking when working
 * @IEEE80211_TPT_LEDTRIG_FL_CONNECTED: enable blinking when at least one
 *	interface is connected in some way, including being an AP
 */
enum ieee80211_tpt_led_trigger_flags {
	IEEE80211_TPT_LEDTRIG_FL_RADIO		= BIT(0),
	IEEE80211_TPT_LEDTRIG_FL_WORK		= BIT(1),
	IEEE80211_TPT_LEDTRIG_FL_CONNECTED	= BIT(2),
};

#ifdef CONFIG_MAC80211_ATBM_LEDS
extern char *__ieee80211_get_tx_led_name(struct ieee80211_hw *hw);
extern char *__ieee80211_get_rx_led_name(struct ieee80211_hw *hw);
extern char *__ieee80211_get_assoc_led_name(struct ieee80211_hw *hw);
extern char *__ieee80211_get_radio_led_name(struct ieee80211_hw *hw);
extern char *__ieee80211_create_tpt_led_trigger(
				struct ieee80211_hw *hw, unsigned int flags,
				const struct ieee80211_tpt_blink *blink_table,
				unsigned int blink_table_len);
#endif
/**
 * ieee80211_get_tx_led_name - get name of TX LED
 *
 * mac80211 creates a transmit LED trigger for each wireless hardware
 * that can be used to drive LEDs if your driver registers a LED device.
 * This function returns the name (or %NULL if not configured for LEDs)
 * of the trigger so you can automatically link the LED device.
 *
 * @hw: the hardware to get the LED trigger name for
 */
static inline char *ieee80211_get_tx_led_name(struct ieee80211_hw *hw)
{
#ifdef CONFIG_MAC80211_ATBM_LEDS
	return __ieee80211_get_tx_led_name(hw);
#else
	return NULL;
#endif
}

/**
 * ieee80211_get_rx_led_name - get name of RX LED
 *
 * mac80211 creates a receive LED trigger for each wireless hardware
 * that can be used to drive LEDs if your driver registers a LED device.
 * This function returns the name (or %NULL if not configured for LEDs)
 * of the trigger so you can automatically link the LED device.
 *
 * @hw: the hardware to get the LED trigger name for
 */
static inline char *ieee80211_get_rx_led_name(struct ieee80211_hw *hw)
{
#ifdef CONFIG_MAC80211_ATBM_LEDS
	return __ieee80211_get_rx_led_name(hw);
#else
	return NULL;
#endif
}

/**
 * ieee80211_get_assoc_led_name - get name of association LED
 *
 * mac80211 creates a association LED trigger for each wireless hardware
 * that can be used to drive LEDs if your driver registers a LED device.
 * This function returns the name (or %NULL if not configured for LEDs)
 * of the trigger so you can automatically link the LED device.
 *
 * @hw: the hardware to get the LED trigger name for
 */
static inline char *ieee80211_get_assoc_led_name(struct ieee80211_hw *hw)
{
#ifdef CONFIG_MAC80211_ATBM_LEDS
	return __ieee80211_get_assoc_led_name(hw);
#else
	return NULL;
#endif
}

/**
 * ieee80211_get_radio_led_name - get name of radio LED
 *
 * mac80211 creates a radio change LED trigger for each wireless hardware
 * that can be used to drive LEDs if your driver registers a LED device.
 * This function returns the name (or %NULL if not configured for LEDs)
 * of the trigger so you can automatically link the LED device.
 *
 * @hw: the hardware to get the LED trigger name for
 */
static inline char *ieee80211_get_radio_led_name(struct ieee80211_hw *hw)
{
#ifdef CONFIG_MAC80211_ATBM_LEDS
	return __ieee80211_get_radio_led_name(hw);
#else
	return NULL;
#endif
}

/**
 * ieee80211_create_tpt_led_trigger - create throughput LED trigger
 * @hw: the hardware to create the trigger for
 * @flags: trigger flags, see &enum ieee80211_tpt_led_trigger_flags
 * @blink_table: the blink table -- needs to be ordered by throughput
 * @blink_table_len: size of the blink table
 *
 * This function returns %NULL (in case of error, or if no LED
 * triggers are configured) or the name of the new trigger.
 * This function must be called before ieee80211_register_hw().
 */
static inline char *
ieee80211_create_tpt_led_trigger(struct ieee80211_hw *hw, unsigned int flags,
				 const struct ieee80211_tpt_blink *blink_table,
				 unsigned int blink_table_len)
{
#ifdef CONFIG_MAC80211_ATBM_LEDS
	return __ieee80211_create_tpt_led_trigger(hw, flags, blink_table,
						  blink_table_len);
#else
	return NULL;
#endif
}

/**
 * ieee80211_unregister_hw - Unregister a hardware device
 *
 * This function instructs mac80211 to free allocated resources
 * and unregister netdevices from the networking subsystem.
 *
 * @hw: the hardware to unregister
 */
void ieee80211_unregister_hw(struct ieee80211_hw *hw);

/**
 * ieee80211_free_hw - free hardware descriptor
 *
 * This function frees everything that was allocated, including the
 * private data for the driver. You must call ieee80211_unregister_hw()
 * before calling this function.
 *
 * @hw: the hardware to free
 */
void ieee80211_free_hw(struct ieee80211_hw *hw);

/**
 * ieee80211_restart_hw - restart hardware completely
 *
 * Call this function when the hardware was restarted for some reason
 * (hardware error, ...) and the driver is unable to restore its state
 * by itself. mac80211 assumes that at this point the driver/hardware
 * is completely uninitialised and stopped, it starts the process by
 * calling the ->start() operation. The driver will need to reset all
 * internal state that it has prior to calling this function.
 *
 * @hw: the hardware to restart
 */
#ifdef CONFIG_ATBM_MAC80211_NO_USE
void ieee80211_restart_hw(struct ieee80211_hw *hw);
#endif
int  ieee80211_restart_hw_sync(struct ieee80211_hw *hw);
int  ieee80211_pre_restart_hw_sync(struct ieee80211_hw *hw);

#ifdef IEEE80211_SUPPORT_NAPI
/** ieee80211_napi_schedule - schedule NAPI poll
 *
 * Use this function to schedule NAPI polling on a device.
 *
 * @hw: the hardware to start polling
 */
void ieee80211_napi_schedule(struct ieee80211_hw *hw);

/** ieee80211_napi_complete - complete NAPI polling
 *
 * Use this function to finish NAPI polling on a device.
 *
 * @hw: the hardware to stop polling
 */
void ieee80211_napi_complete(struct ieee80211_hw *hw);
#endif
/**
 * ieee80211_rx - receive frame
 *
 * Use this function to hand received frames to mac80211. The receive
 * buffer in @skb must start with an IEEE 802.11 header. In case of a
 * paged @skb is used, the driver is recommended to put the ieee80211
 * header of the frame on the linear part of the @skb to avoid memory
 * allocation and/or memcpy by the stack.
 *
 * This function may not be called in IRQ context. Calls to this function
 * for a single hardware must be synchronized against each other. Calls to
 * this function, ieee80211_rx_ni() and ieee80211_rx_irqsafe() may not be
 * mixed for a single hardware.
 *
 * In process context use instead ieee80211_rx_ni().
 *
 * @hw: the hardware this frame came in on
 * @skb: the buffer to receive, owned by mac80211 after this call
 */
void ieee80211_rx(struct ieee80211_hw *hw, struct sk_buff *skb);

/**
 * ieee80211_rx_irqsafe - receive frame
 *
 * Like ieee80211_rx() but can be called in IRQ context
 * (internally defers to a tasklet.)
 *
 * Calls to this function, ieee80211_rx() or ieee80211_rx_ni() may not
 * be mixed for a single hardware.
 *
 * @hw: the hardware this frame came in on
 * @skb: the buffer to receive, owned by mac80211 after this call
 */
void ieee80211_rx_irqsafe(struct ieee80211_hw *hw, struct sk_buff *skb);

/**
 * ieee80211_rx_ni - receive frame (in process context)
 *
 * Like ieee80211_rx() but can be called in process context
 * (internally disables bottom halves).
 *
 * Calls to this function, ieee80211_rx() and ieee80211_rx_irqsafe() may
 * not be mixed for a single hardware.
 *
 * @hw: the hardware this frame came in on
 * @skb: the buffer to receive, owned by mac80211 after this call
 */
static inline void ieee80211_rx_ni(struct ieee80211_hw *hw,
				   struct sk_buff *skb)
{
	local_bh_disable();
	ieee80211_rx(hw, skb);
	local_bh_enable();
}

/**
 * ieee80211_sta_ps_transition - PS transition for connected sta
 *
 * When operating in AP mode with the %IEEE80211_HW_AP_LINK_PS
 * flag set, use this function to inform mac80211 about a connected station
 * entering/leaving PS mode.
 *
 * This function may not be called in IRQ context or with softirqs enabled.
 *
 * Calls to this function for a single hardware must be synchronized against
 * each other.
 *
 * The function returns -EINVAL when the requested PS mode is already set.
 *
 * @sta: currently connected sta
 * @start: start or stop PS
 */
int ieee80211_sta_ps_transition(struct ieee80211_sta *sta, bool start);

/**
 * ieee80211_sta_ps_transition_ni - PS transition for connected sta
 *                                  (in process context)
 *
 * Like ieee80211_sta_ps_transition() but can be called in process context
 * (internally disables bottom halves). Concurrent call restriction still
 * applies.
 *
 * @sta: currently connected sta
 * @start: start or stop PS
 */
static inline int ieee80211_sta_ps_transition_ni(struct ieee80211_sta *sta,
						  bool start)
{
	int ret;

	local_bh_disable();
	ret = ieee80211_sta_ps_transition(sta, start);
	local_bh_enable();

	return ret;
}

/*
 * The TX headroom reserved by mac80211 for its own tx_status functions.
 * This is enough for the radiotap header.
 */
#define IEEE80211_TX_STATUS_HEADROOM	14

/**
 * ieee80211_sta_set_buffered - inform mac80211 about driver-buffered frames
 * @sta: &struct ieee80211_sta pointer for the sleeping station
 * @tid: the TID that has buffered frames
 * @buffered: indicates whether or not frames are buffered for this TID
 *
 * If a driver buffers frames for a powersave station instead of passing
 * them back to mac80211 for retransmission, the station may still need
 * to be told that there are buffered frames via the TIM bit.
 *
 * This function informs mac80211 whether or not there are frames that are
 * buffered in the driver for a given TID; mac80211 can then use this data
 * to set the TIM bit (NOTE: This may call back into the driver's set_tim
 * call! Beware of the locking!)
 *
 * If all frames are released to the station (due to PS-poll or uAPSD)
 * then the driver needs to inform mac80211 that there no longer are
 * frames buffered. However, when the station wakes up mac80211 assumes
 * that all buffered frames will be transmitted and clears this data,
 * drivers need to make sure they inform mac80211 about all buffered
 * frames on the sleep transition (sta_notify() with %STA_NOTIFY_SLEEP).
 *
 * Note that technically mac80211 only needs to know this per AC, not per
 * TID, but since driver buffering will inevitably happen per TID (since
 * it is related to aggregation) it is easier to make mac80211 map the
 * TID to the AC as required instead of keeping track in all drivers that
 * use this API.
 */
void ieee80211_sta_set_buffered(struct ieee80211_sta *sta,
				u8 tid, bool buffered);
bool ieee80211_sta_support_40w(struct ieee80211_sta *pubsta);

/**
 * ieee80211_tx_status - transmit status callback
 *
 * Call this function for all transmitted frames after they have been
 * transmitted. It is permissible to not call this function for
 * multicast frames but this can affect statistics.
 *
 * This function may not be called in IRQ context. Calls to this function
 * for a single hardware must be synchronized against each other. Calls
 * to this function, ieee80211_tx_status_ni() and ieee80211_tx_status_irqsafe()
 * may not be mixed for a single hardware.
 *
 * @hw: the hardware the frame was transmitted by
 * @skb: the frame that was transmitted, owned by mac80211 after this call
 */
void ieee80211_tx_status(struct ieee80211_hw *hw,
			 struct sk_buff *skb);

/**
 * ieee80211_tx_status_ni - transmit status callback (in process context)
 *
 * Like ieee80211_tx_status() but can be called in process context.
 *
 * Calls to this function, ieee80211_tx_status() and
 * ieee80211_tx_status_irqsafe() may not be mixed
 * for a single hardware.
 *
 * @hw: the hardware the frame was transmitted by
 * @skb: the frame that was transmitted, owned by mac80211 after this call
 */
static inline void ieee80211_tx_status_ni(struct ieee80211_hw *hw,
					  struct sk_buff *skb)
{
	local_bh_disable();
	ieee80211_tx_status(hw, skb);
	local_bh_enable();
}

/**
 * ieee80211_tx_status_irqsafe - IRQ-safe transmit status callback
 *
 * Like ieee80211_tx_status() but can be called in IRQ context
 * (internally defers to a tasklet.)
 *
 * Calls to this function, ieee80211_tx_status() and
 * ieee80211_tx_status_ni() may not be mixed for a single hardware.
 *
 * @hw: the hardware the frame was transmitted by
 * @skb: the frame that was transmitted, owned by mac80211 after this call
 */
 #if 0
void ieee80211_tx_status_irqsafe(struct ieee80211_hw *hw,
				 struct sk_buff *skb);
 #endif
#if defined (CONFIG_ATBM_MAC80211_NO_USE)
/**
 * ieee80211_report_low_ack - report non-responding station
 *
 * When operating in AP-mode, call this function to report a non-responding
 * connected STA.
 *
 * @sta: the non-responding connected sta
 * @num_packets: number of packets sent to @sta without a response
 */
void ieee80211_report_low_ack(struct ieee80211_sta *sta, u32 num_packets);
char * ieee80211_vif_name_get(struct ieee80211_vif * vif);
#endif
/**
 * ieee80211_beacon_get_tim - beacon generation function
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @tim_offset: pointer to variable that will receive the TIM IE offset.
 *	Set to 0 if invalid (in non-AP modes).
 * @tim_length: pointer to variable that will receive the TIM IE length,
 *	(including the ID and length bytes!).
 *	Set to 0 if invalid (in non-AP modes).
 *
 * If the driver implements beaconing modes, it must use this function to
 * obtain the beacon frame/template.
 *
 * If the beacon frames are generated by the host system (i.e., not in
 * hardware/firmware), the driver uses this function to get each beacon
 * frame from mac80211 -- it is responsible for calling this function
 * before the beacon is needed (e.g. based on hardware interrupt).
 *
 * If the beacon frames are generated by the device, then the driver
 * must use the returned beacon as the template and change the TIM IE
 * according to the current DTIM parameters/TIM bitmap.
 *
 * The driver is responsible for freeing the returned skb.
 */
struct sk_buff *ieee80211_beacon_get_tim(struct ieee80211_hw *hw,
					 struct ieee80211_vif *vif,
					 u16 *tim_offset, u16 *tim_length);

/**
 * ieee80211_beacon_get - beacon generation function
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * See ieee80211_beacon_get_tim().
 */
static inline struct sk_buff *ieee80211_beacon_get(struct ieee80211_hw *hw,
						   struct ieee80211_vif *vif)
{
	return ieee80211_beacon_get_tim(hw, vif, NULL, NULL);
}

#ifdef ATBM_PROBE_RESP_EXTRA_IE
/**
 * ieee80211_proberesp_get - probe response generation function
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 */
struct sk_buff *ieee80211_proberesp_get(struct ieee80211_hw *hw,
						   struct ieee80211_vif *vif);
#endif
#if defined (CONFIG_ATBM_MAC80211_NO_USE) || defined (CONFIG_ATBM_BT_COMB)

/**
 * ieee80211_pspoll_get - retrieve a PS Poll template
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * Creates a PS Poll a template which can, for example, uploaded to
 * hardware. The template must be updated after association so that correct
 * AID, BSSID and MAC address is used.
 *
 * Note: Caller (or hardware) is responsible for setting the
 * &IEEE80211_FCTL_PM bit.
 */
struct sk_buff *ieee80211_pspoll_get(struct ieee80211_hw *hw,
				     struct ieee80211_vif *vif);

/**
 * ieee80211_nullfunc_get - retrieve a nullfunc template
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * Creates a Nullfunc template which can, for example, uploaded to
 * hardware. The template must be updated after association so that correct
 * BSSID and address is used.
 *
 * Note: Caller (or hardware) is responsible for setting the
 * &IEEE80211_FCTL_PM bit as well as Duration and Sequence Control fields.
 */
struct sk_buff *ieee80211_nullfunc_get(struct ieee80211_hw *hw,
				       struct ieee80211_vif *vif);
#endif

/**
 * ieee80211_qosnullfunc_get - retrieve a qos nullfunc template
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * Creates a QOS Nullfunc template which can, for example, be uploaded to
 * hardware. The template must be updated after association so that correct
 * BSSID and address is used.
 *
 * Note: Caller (or hardware) is responsible for setting the
 * &IEEE80211_FCTL_PM bit as well as Duration and Sequence Control fields.
 * The qos ctl field is set to zero by the function. Caller must
 * fill up proper value.
 */
struct sk_buff *ieee80211_qosnullfunc_get(struct ieee80211_hw *hw,
					  struct ieee80211_vif *vif);
/**
 * ieee80211_probereq_get - retrieve a Probe Request template
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @ssid: SSID buffer
 * @ssid_len: length of SSID
 * @ie: buffer containing all IEs except SSID for the template
 * @ie_len: length of the IE buffer
 *
 * Creates a Probe Request template which can, for example, be uploaded to
 * hardware.
 */
struct sk_buff *ieee80211_probereq_get(struct ieee80211_hw *hw,
				       struct ieee80211_vif *vif,
				       const u8 *ssid, size_t ssid_len,
				       const u8 *ie, size_t ie_len,u8 *bssid);
#ifdef CONFIG_ATBM_MAC80211_NO_USE
/**
 * ieee80211_rts_get - RTS frame generation function
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @frame: pointer to the frame that is going to be protected by the RTS.
 * @frame_len: the frame length (in octets).
 * @frame_txctl: &struct ieee80211_tx_info of the frame.
 * @rts: The buffer where to store the RTS frame.
 *
 * If the RTS frames are generated by the host system (i.e., not in
 * hardware/firmware), the low-level driver uses this function to receive
 * the next RTS frame from the 802.11 code. The low-level is responsible
 * for calling this function before and RTS frame is needed.
 */
void ieee80211_rts_get(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
		       const void *frame, size_t frame_len,
		       const struct ieee80211_tx_info *frame_txctl,
		       struct ieee80211_rts *rts);
/**
 * ieee80211_ctstoself_get - CTS-to-self frame generation function
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @frame: pointer to the frame that is going to be protected by the CTS-to-self.
 * @frame_len: the frame length (in octets).
 * @frame_txctl: &struct ieee80211_tx_info of the frame.
 * @cts: The buffer where to store the CTS-to-self frame.
 *
 * If the CTS-to-self frames are generated by the host system (i.e., not in
 * hardware/firmware), the low-level driver uses this function to receive
 * the next CTS-to-self frame from the 802.11 code. The low-level is responsible
 * for calling this function before and CTS-to-self frame is needed.
 */
void ieee80211_ctstoself_get(struct ieee80211_hw *hw,
			     struct ieee80211_vif *vif,
			     const void *frame, size_t frame_len,
			     const struct ieee80211_tx_info *frame_txctl,
			     struct ieee80211_cts *cts);

/**
 * ieee80211_rts_duration - Get the duration field for an RTS frame
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @frame_len: the length of the frame that is going to be protected by the RTS.
 * @frame_txctl: &struct ieee80211_tx_info of the frame.
 *
 * If the RTS is generated in firmware, but the host system must provide
 * the duration field, the low-level driver uses this function to receive
 * the duration field value in little-endian byteorder.
 */
__le16 ieee80211_rts_duration(struct ieee80211_hw *hw,
			      struct ieee80211_vif *vif, size_t frame_len,
			      const struct ieee80211_tx_info *frame_txctl);
/**
 * ieee80211_ctstoself_duration - Get the duration field for a CTS-to-self frame
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @frame_len: the length of the frame that is going to be protected by the CTS-to-self.
 * @frame_txctl: &struct ieee80211_tx_info of the frame.
 *
 * If the CTS-to-self is generated in firmware, but the host system must provide
 * the duration field, the low-level driver uses this function to receive
 * the duration field value in little-endian byteorder.
 */
__le16 ieee80211_ctstoself_duration(struct ieee80211_hw *hw,
				    struct ieee80211_vif *vif,
				    size_t frame_len,
				    const struct ieee80211_tx_info *frame_txctl);

/**
 * ieee80211_generic_frame_duration - Calculate the duration field for a frame
 * @hw: pointer obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @frame_len: the length of the frame.
 * @rate: the rate at which the frame is going to be transmitted.
 *
 * Calculate the duration field of some generic frame, given its
 * length and transmission rate (in 100kbps).
 */
__le16 ieee80211_generic_frame_duration(struct ieee80211_hw *hw,
					struct ieee80211_vif *vif,
					enum ieee80211_band band,
					size_t frame_len,
					struct ieee80211_rate *rate);
#endif
/**
 * ieee80211_get_buffered_bc - accessing buffered broadcast and multicast frames
 * @hw: pointer as obtained from ieee80211_alloc_hw().
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * Function for accessing buffered broadcast and multicast frames. If
 * hardware/firmware does not implement buffering of broadcast/multicast
 * frames when power saving is used, 802.11 code buffers them in the host
 * memory. The low-level driver uses this function to fetch next buffered
 * frame. In most cases, this is used when generating beacon frame. This
 * function returns a pointer to the next buffered skb or NULL if no more
 * buffered frames are available.
 *
 * Note: buffered frames are returned only after DTIM beacon frame was
 * generated with ieee80211_beacon_get() and the low-level driver must thus
 * call ieee80211_beacon_get() first. ieee80211_get_buffered_bc() returns
 * NULL if the previous generated beacon was not DTIM, so the low-level driver
 * does not need to check for DTIM beacons separately and should be able to
 * use common code for all beacons.
 */
struct sk_buff *
ieee80211_get_buffered_bc(struct ieee80211_hw *hw, struct ieee80211_vif *vif);

#ifdef CONFIG_ATBM_USE_SW_ENC
/**
 * ieee80211_get_tkip_p1k_iv - get a TKIP phase 1 key for IV32
 *
 * This function returns the TKIP phase 1 key for the given IV32.
 *
 * @keyconf: the parameter passed with the set key
 * @iv32: IV32 to get the P1K for
 * @p1k: a buffer to which the key will be written, as 5 u16 values
 */
void ieee80211_get_tkip_p1k_iv(struct ieee80211_key_conf *keyconf,
			       u32 iv32, u16 *p1k);

/**
 * ieee80211_get_tkip_p1k - get a TKIP phase 1 key
 *
 * This function returns the TKIP phase 1 key for the IV32 taken
 * from the given packet.
 *
 * @keyconf: the parameter passed with the set key
 * @skb: the packet to take the IV32 value from that will be encrypted
 *	with this P1K
 * @p1k: a buffer to which the key will be written, as 5 u16 values
 */
static inline void ieee80211_get_tkip_p1k(struct ieee80211_key_conf *keyconf,
					  struct sk_buff *skb, u16 *p1k)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	const u8 *data = (u8 *)hdr + ieee80211_hdrlen(hdr->frame_control);
	u32 iv32 = get_unaligned_le32(&data[4]);

	ieee80211_get_tkip_p1k_iv(keyconf, iv32, p1k);
}

/**
 * ieee80211_get_tkip_rx_p1k - get a TKIP phase 1 key for RX
 *
 * This function returns the TKIP phase 1 key for the given IV32
 * and transmitter address.
 *
 * @keyconf: the parameter passed with the set key
 * @ta: TA that will be used with the key
 * @iv32: IV32 to get the P1K for
 * @p1k: a buffer to which the key will be written, as 5 u16 values
 */
void ieee80211_get_tkip_rx_p1k(struct ieee80211_key_conf *keyconf,
			       const u8 *ta, u32 iv32, u16 *p1k);

/**
 * ieee80211_get_tkip_p2k - get a TKIP phase 2 key
 *
 * This function computes the TKIP RC4 key for the IV values
 * in the packet.
 *
 * @keyconf: the parameter passed with the set key
 * @skb: the packet to take the IV32/IV16 values from that will be
 *	encrypted with this key
 * @p2k: a buffer to which the key will be written, 16 bytes
 */
void ieee80211_get_tkip_p2k(struct ieee80211_key_conf *keyconf,
			    struct sk_buff *skb, u8 *p2k);
#endif
/**
 * struct ieee80211_key_seq - key sequence counter
 *
 * @tkip: TKIP data, containing IV32 and IV16 in host byte order
 * @ccmp: PN data, most significant byte first (big endian,
 *	reverse order than in packet)
 * @aes_cmac: PN data, most significant byte first (big endian,
 *	reverse order than in packet)
 */
struct ieee80211_key_seq {
	union {
		struct {
			u32 iv32;
			u16 iv16;
		} tkip;
		struct {
			u8 pn[6];
		} ccmp;
		struct {
			u8 pn[6];
		} aes_cmac;
	};
};
#ifdef CONFIG_ATBM_USE_SW_ENC
/**
 * ieee80211_get_key_tx_seq - get key TX sequence counter
 *
 * @keyconf: the parameter passed with the set key
 * @seq: buffer to receive the sequence data
 *
 * This function allows a driver to retrieve the current TX IV/PN
 * for the given key. It must not be called if IV generation is
 * offloaded to the device.
 *
 * Note that this function may only be called when no TX processing
 * can be done concurrently, for example when queues are stopped
 * and the stop has been synchronized.
 */
void ieee80211_get_key_tx_seq(struct ieee80211_key_conf *keyconf,
			      struct ieee80211_key_seq *seq);

/**
 * ieee80211_get_key_rx_seq - get key RX sequence counter
 *
 * @keyconf: the parameter passed with the set key
 * @tid: The TID, or -1 for the management frame value (CCMP only);
 *	the value on TID 0 is also used for non-QoS frames. For
 *	CMAC, only TID 0 is valid.
 * @seq: buffer to receive the sequence data
 *
 * This function allows a driver to retrieve the current RX IV/PNs
 * for the given key. It must not be called if IV checking is done
 * by the device and not by mac80211.
 *
 * Note that this function may only be called when no RX processing
 * can be done concurrently.
 */
void ieee80211_get_key_rx_seq(struct ieee80211_key_conf *keyconf,
			      int tid, struct ieee80211_key_seq *seq);
/**
 * ieee80211_gtk_rekey_notify - notify userspace supplicant of rekeying
 * @vif: virtual interface the rekeying was done on
 * @bssid: The BSSID of the AP, for checking association
 * @replay_ctr: the new replay counter after GTK rekeying
 * @gfp: allocation flags
 */
 #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0))
void ieee80211_gtk_rekey_notify(struct ieee80211_vif *vif, const u8 *bssid,
				const u8 *replay_ctr, gfp_t gfp);
 #endif
#endif
/**
 * ieee80211_wake_queue - wake specific queue
 * @hw: pointer as obtained from ieee80211_alloc_hw().
 * @queue: queue number (counted from zero).
 *
 * Drivers should use this function instead of netif_wake_queue.
 */
void ieee80211_wake_queue(struct ieee80211_hw *hw, int queue);

/**
 * ieee80211_stop_queue - stop specific queue
 * @hw: pointer as obtained from ieee80211_alloc_hw().
 * @queue: queue number (counted from zero).
 *
 * Drivers should use this function instead of netif_stop_queue.
 */
void ieee80211_stop_queue(struct ieee80211_hw *hw, int queue);

/**
 * ieee80211_queue_stopped - test status of the queue
 * @hw: pointer as obtained from ieee80211_alloc_hw().
 * @queue: queue number (counted from zero).
 *
 * Drivers should use this function instead of netif_stop_queue.
 */

int ieee80211_queue_stopped(struct ieee80211_hw *hw, int queue);

/**
 * ieee80211_stop_queues - stop all queues
 * @hw: pointer as obtained from ieee80211_alloc_hw().
 *
 * Drivers should use this function instead of netif_stop_queue.
 */
void ieee80211_stop_queues(struct ieee80211_hw *hw);

/**
 * ieee80211_wake_queues - wake all queues
 * @hw: pointer as obtained from ieee80211_alloc_hw().
 *
 * Drivers should use this function instead of netif_wake_queue.
 */
void ieee80211_wake_queues(struct ieee80211_hw *hw);

/**
 * ieee80211_scan_completed - completed hardware scan
 *
 * When hardware scan offload is used (i.e. the hw_scan() callback is
 * assigned) this function needs to be called by the driver to notify
 * mac80211 that the scan finished. This function can be called from
 * any context, including hardirq context.
 *
 * @hw: the hardware that finished the scan
 * @aborted: set to true if scan was aborted
 */
void ieee80211_scan_completed(struct ieee80211_hw *hw, bool aborted);
#ifdef CONFIG_ATBM_SUPPORT_SCHED_SCAN
/**
 * ieee80211_sched_scan_results - got results from scheduled scan
 *
 * When a scheduled scan is running, this function needs to be called by the
 * driver whenever there are new scan results available.
 *
 * @hw: the hardware that is performing scheduled scans
 */
void ieee80211_sched_scan_results(struct ieee80211_hw *hw);

/**
 * ieee80211_sched_scan_stopped - inform that the scheduled scan has stopped
 *
 * When a scheduled scan is running, this function can be called by
 * the driver if it needs to stop the scan to perform another task.
 * Usual scenarios are drivers that cannot continue the scheduled scan
 * while associating, for instance.
 *
 * @hw: the hardware that is performing scheduled scans
 */
void ieee80211_sched_scan_stopped(struct ieee80211_hw *hw);
#endif
#ifdef CONFIG_ATBM_MAC80211_NO_USE
/**
 * ieee80211_iterate_active_interfaces - iterate active interfaces
 *
 * This function iterates over the interfaces associated with a given
 * hardware that are currently active and calls the callback for them.
 * This function allows the iterator function to sleep, when the iterator
 * function is atomic @ieee80211_iterate_active_interfaces_atomic can
 * be used.
 * Does not iterate over a new interface during add_interface()
 *
 * @hw: the hardware struct of which the interfaces should be iterated over
 * @iterator: the iterator function to call
 * @data: first argument of the iterator function
 */
void ieee80211_iterate_active_interfaces(struct ieee80211_hw *hw,
					 void (*iterator)(void *data, u8 *mac,
						struct ieee80211_vif *vif),
					 void *data);
#endif

/**
 * ieee80211_iterate_active_interfaces_atomic - iterate active interfaces
 *
 * This function iterates over the interfaces associated with a given
 * hardware that are currently active and calls the callback for them.
 * This function requires the iterator callback function to be atomic,
 * if that is not desired, use @ieee80211_iterate_active_interfaces instead.
 * Does not iterate over a new interface during add_interface()
 *
 * @hw: the hardware struct of which the interfaces should be iterated over
 * @iterator: the iterator function to call, cannot sleep
 * @data: first argument of the iterator function
 */
void ieee80211_iterate_active_interfaces_atomic(struct ieee80211_hw *hw,
						void (*iterator)(void *data,
						    u8 *mac,
						    struct ieee80211_vif *vif),
						void *data);

/**
 * ieee80211_queue_work - add work onto the mac80211 workqueue
 *
 * Drivers and mac80211 use this to add work onto the mac80211 workqueue.
 * This helper ensures drivers are not queueing work when they should not be.
 *
 * @hw: the hardware struct for the interface we are adding work for
 * @work: the work we want to add onto the mac80211 workqueue
 */
void ieee80211_queue_work(struct ieee80211_hw *hw, struct atbm_work_struct *work);

/**
 * ieee80211_queue_delayed_work - add work onto the mac80211 workqueue
 *
 * Drivers and mac80211 use this to queue delayed work onto the mac80211
 * workqueue.
 *
 * @hw: the hardware struct for the interface we are adding work for
 * @dwork: delayable work to queue onto the mac80211 workqueue
 * @delay: number of jiffies to wait before queueing
 */
void ieee80211_queue_delayed_work(struct ieee80211_hw *hw,
				  struct atbm_delayed_work *dwork,
				  unsigned long delay);
#ifdef CONFIG_ATBM_DRIVER_PROCESS_BA

/**
 * ieee80211_start_tx_ba_session - Start a tx Block Ack session.
 * @sta: the station for which to start a BA session
 * @tid: the TID to BA on.
 * @timeout: session timeout value (in TUs)
 *
 * Return: success if addBA request was sent, failure otherwise
 *
 * Although mac80211/low level driver/user space application can estimate
 * the need to start aggregation on a certain RA/TID, the session level
 * will be managed by the mac80211.
 */
int ieee80211_start_tx_ba_session(struct ieee80211_sta *sta, u16 tid,
				  u16 timeout);
/**
 * ieee80211_stop_tx_ba_session - Stop a Block Ack session.
 * @sta: the station whose BA session to stop
 * @tid: the TID to stop BA.
 *
 * Return: negative error if the TID is invalid, or no aggregation active
 *
 * Although mac80211/low level driver/user space application can estimate
 * the need to stop aggregation on a certain RA/TID, the session level
 * will be managed by the mac80211.
 */
int ieee80211_stop_tx_ba_session(struct ieee80211_sta *sta, u16 tid);
#endif
/**
 * ieee80211_find_sta - find a station
 *
 * @vif: virtual interface to look for station on
 * @addr: station's address
 *
 * This function must be called under RCU lock and the
 * resulting pointer is only valid under RCU lock as well.
 */
struct ieee80211_sta *ieee80211_find_sta(struct ieee80211_vif *vif,
					 const u8 *addr);

/**
 * ieee80211_iter_keys - iterate keys programmed into the device
 * @hw: pointer obtained from ieee80211_alloc_hw()
 * @vif: virtual interface to iterate, may be %NULL for all
 * @iter: iterator function that will be called for each key
 * @iter_data: custom data to pass to the iterator function
 *
 * This function can be used to iterate all the keys known to
 * mac80211, even those that weren't previously programmed into
 * the device. This is intended for use in WoWLAN if the device
 * needs reprogramming of the keys during suspend. Note that due
 * to locking reasons, it is also only safe to call this at few
 * spots since it must hold the RTNL and be able to sleep.
 *
 * The order in which the keys are iterated matches the order
 * in which they were originally installed and handed to the
 * set_key callback.
 */
void ieee80211_iter_keys(struct ieee80211_hw *hw,
			 struct ieee80211_vif *vif,
			 void (*iter)(struct ieee80211_hw *hw,
				      struct ieee80211_vif *vif,
				      struct ieee80211_sta *sta,
				      struct ieee80211_key_conf *key,
				      void *data),
			 void *iter_data);

/**
 * ieee80211_connection_loss - inform hardware has lost connection to the AP
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * When beacon filtering is enabled with %IEEE80211_HW_BEACON_FILTER, and
 * %IEEE80211_CONF_PS and %IEEE80211_HW_CONNECTION_MONITOR are set, the driver
 * needs to inform if the connection to the AP has been lost.
 *
 * This function will cause immediate change to disassociated state,
 * without connection recovery attempts.
 */
void ieee80211_connection_loss(struct ieee80211_vif *vif);
#ifdef CONFIG_ATBM_MAC80211_NO_USE

/**
 * ieee80211_cqm_rssi_notify - inform a configured connection quality monitoring
 *	rssi threshold triggered
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @rssi_event: the RSSI trigger event type
 * @gfp: context flags
 *
 * When the %IEEE80211_HW_SUPPORTS_CQM_RSSI is set, and a connection quality
 * monitoring is configured with an rssi threshold, the driver will inform
 * whenever the rssi level reaches the threshold.
 */
void ieee80211_cqm_rssi_notify(struct ieee80211_vif *vif,
			       enum nl80211_cqm_rssi_threshold_event rssi_event,
			       gfp_t gfp);

/**
 * ieee80211_beacon_loss - inform hardware does not receive beacons
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * When beacon filtering is enabled with %IEEE80211_HW_BEACON_FILTER and
 * %IEEE80211_CONF_PS is set, the driver needs to inform whenever the
 * hardware is not receiving beacons with this function.
 */
void ieee80211_beacon_loss(struct ieee80211_vif *vif);

/**
 * ieee80211_sta_eosp - notify mac80211 about end of SP
 * @pubsta: the station
 *
 * When a device transmits frames in a way that it can't tell
 * mac80211 in the TX status about the EOSP, it must clear the
 * %IEEE80211_TX_STATUS_EOSP bit and call this function instead.
 * This applies for PS-Poll as well as uAPSD.
 *
 * Note that there is no non-_irqsafe version right now as
 * it wasn't needed, but just like _tx_status() and _rx()
 * must not be mixed in irqsafe/non-irqsafe versions, this
 * function must not be mixed with those either. Use the
 * all irqsafe, or all non-irqsafe, don't mix! If you need
 * the non-irqsafe version of this, you need to add it.
 */
void ieee80211_sta_eosp_irqsafe(struct ieee80211_sta *pubsta);

/**
 * ieee80211_sta_block_awake - block station from waking up
 * @hw: the hardware
 * @pubsta: the station
 * @block: whether to block or unblock
 *
 * Some devices require that all frames that are on the queues
 * for a specific station that went to sleep are flushed before
 * a poll response or frames after the station woke up can be
 * delivered to that it. Note that such frames must be rejected
 * by the driver as filtered, with the appropriate status flag.
 *
 * This function allows implementing this mode in a race-free
 * manner.
 *
 * To do this, a driver must keep track of the number of frames
 * still enqueued for a specific station. If this number is not
 * zero when the station goes to sleep, the driver must call
 * this function to force mac80211 to consider the station to
 * be asleep regardless of the station's actual state. Once the
 * number of outstanding frames reaches zero, the driver must
 * call this function again to unblock the station. That will
 * cause mac80211 to be able to send ps-poll responses, and if
 * the station queried in the meantime then frames will also
 * be sent out as a result of this. Additionally, the driver
 * will be notified that the station woke up some time after
 * it is unblocked, regardless of whether the station actually
 * woke up while blocked or not.
 */
void ieee80211_sta_block_awake(struct ieee80211_hw *hw,
			       struct ieee80211_sta *pubsta, bool block);

/**
 * ieee80211_find_sta_by_ifaddr - find a station on hardware
 *
 * @hw: pointer as obtained from ieee80211_alloc_hw()
 * @addr: remote station's address
 * @localaddr: local address (vif->sdata->vif.addr). Use NULL for 'any'.
 *
 * This function must be called under RCU lock and the
 * resulting pointer is only valid under RCU lock as well.
 *
 * NOTE: You may pass NULL for localaddr, but then you will just get
 *      the first STA that matches the remote address 'addr'.
 *      We can have multiple STA associated with multiple
 *      logical stations (e.g. consider a station connecting to another
 *      BSSID on the same AP hardware without disconnecting first).
 *      In this case, the result of this method with localaddr NULL
 *      is not reliable.
 *
 * DO NOT USE THIS FUNCTION with localaddr NULL if at all possible.
 */
struct ieee80211_sta *ieee80211_find_sta_by_ifaddr(struct ieee80211_hw *hw,
					       const u8 *addr,
					       const u8 *localaddr);

/**
 * ieee80211_resume_disconnect - disconnect from AP after resume
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * Instructs mac80211 to disconnect from the AP after resume.
 * Drivers can use this after WoWLAN if they know that the
 * connection cannot be kept up, for example because keys were
 * used while the device was asleep but the replay counters or
 * similar cannot be retrieved from the device during resume.
 *
 * Note that due to implementation issues, if the driver uses
 * the reconfiguration functionality during resume the interface
 * will still be added as associated first during resume and then
 * disconnect normally later.
 *
 * This function can only be called from the resume callback and
 * the driver must not be holding any of its own locks while it
 * calls this function, or at least not any locks it needs in the
 * key configuration paths (if it supports HW crypto).
 */
void ieee80211_resume_disconnect(struct ieee80211_vif *vif);

/**
 * ieee80211_disable_dyn_ps - force mac80211 to temporarily disable dynamic psm
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * Some hardware require full power save to manage simultaneous BT traffic
 * on the WLAN frequency. Full PSM is required periodically, whenever there are
 * burst of BT traffic. The hardware gets information of BT traffic via
 * hardware co-existence lines, and consequentially requests mac80211 to
 * (temporarily) enter full psm.
 * This function will only temporarily disable dynamic PS, not enable PSM if
 * it was not already enabled.
 * The driver must make sure to re-enable dynamic PS using
 * ieee80211_enable_dyn_ps() if the driver has disabled it.
 *
 */

void ieee80211_disable_dyn_ps(struct ieee80211_vif *vif);

/**
 * ieee80211_enable_dyn_ps - restore dynamic psm after being disabled
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * This function restores dynamic PS after being temporarily disabled via
 * ieee80211_disable_dyn_ps(). Each ieee80211_disable_dyn_ps() call must
 * be coupled with an eventual call to this function.
 *
 */
void ieee80211_enable_dyn_ps(struct ieee80211_vif *vif);

/**
 * ieee80211_get_operstate - get the operstate of the vif
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 *
 * The driver might need to know the operstate of the net_device
 * (specifically, whether the link is IF_OPER_UP after resume)
 */
unsigned char ieee80211_get_operstate(struct ieee80211_vif *vif);

/**
 * ieee80211_cqm_beacon_miss_notify - inform a configured connection quality
 *	monitoring beacon miss threshold triggered
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @gfp: context flag
 *
 * When the %IEEE80211_HW_SUPPORTS_CQM_BEACON_MISS is set, and a connection
 * quality monitoring is configured with an beacon miss threshold, the driver
 * will inform whenever the desired amount of consecutive beacons is missed.
 */
 /*
void ieee80211_cqm_beacon_miss_notify(struct ieee80211_vif *vif,
				      gfp_t gfp);
*/
/**
 * ieee80211_cqm_tx_fail_notify - inform a configured connection quality
 *	monitoring beacon miss threshold triggered
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @gfp: context flag
 *
 * When the %IEEE80211_HW_SUPPORTS_CQM_TX_FAIL is set, and a connection
 * quality monitoring is configured with an tx failure threshold, the driver
 * whenever the desired amount of consecutive TX attempts is failed.
 */
 #if 0
void ieee80211_cqm_tx_fail_notify(struct ieee80211_vif *vif,
				  gfp_t gfp);

/**
 * ieee80211_p2p_noa_notify - driver set/change NOA params
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @p2p_ps: new NOA params set/change by driver
 * @gfp: context flag
 *
 * Inform upper layer about NOA change
 */
void ieee80211_p2p_noa_notify(struct ieee80211_vif *vif,
			      struct cfg80211_p2p_ps *p2p_ps,
			      gfp_t gfp);

/**
 * ieee80211_driver_hang_notify - inform upper layer about driver hang
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @gfp: context flag
 *
 * In case of FW/driver failure, send a notification to upper layers
 * to take a recovery actions.
 */
void ieee80211_driver_hang_notify(struct ieee80211_vif *vif, gfp_t gfp);
#endif


/**
 * ieee80211_chswitch_done - Complete channel switch process
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @success: make the channel switch successful or not
 *
 * Complete the channel switch post-process: set the new operational channel
 * and wake up the suspended queues.
 */
void ieee80211_chswitch_done(struct ieee80211_vif *vif, bool success);

/**
 * ieee80211_request_smps - request SM PS transition
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @smps_mode: new SM PS mode
 *
 * This allows the driver to request an SM PS transition in managed
 * mode. This is useful when the driver has more information than
 * the stack about possible interference, for example by bluetooth.
 */
void ieee80211_request_smps(struct ieee80211_vif *vif,
			    enum ieee80211_smps_mode smps_mode);

/**
 * ieee80211_key_removed - disable hw acceleration for key
 * @key_conf: The key hw acceleration should be disabled for
 *
 * This allows drivers to indicate that the given key has been
 * removed from hardware acceleration, due to a new key that
 * was added. Don't use this if the key can continue to be used
 * for TX, if the key restriction is on RX only it is permitted
 * to keep the key for TX only and not call this function.
 *
 * Due to locking constraints, it may only be called during
 * @set_key. This function must be allowed to sleep, and the
 * key it tries to disable may still be used until it returns.
 */
void ieee80211_key_removed(struct ieee80211_key_conf *key_conf);
#endif
#ifdef CONFIG_ATBM_SUPPORT_P2P

/**
 * ieee80211_ready_on_channel - notification of remain-on-channel start
 * @hw: pointer as obtained from ieee80211_alloc_hw()
 */
void ieee80211_ready_on_channel(struct ieee80211_hw *hw,unsigned long roc_extend);

/**
 * ieee80211_remain_on_channel_expired - remain_on_channel duration expired
 * @hw: pointer as obtained from ieee80211_alloc_hw()
 */
void ieee80211_remain_on_channel_expired(struct ieee80211_hw *hw, u64 cookie);
#endif
#ifdef CONFIG_ATBM_MAC80211_NO_USE
/**
 * ieee80211_stop_rx_ba_session - callback to stop existing BA sessions
 *
 * in order not to harm the system performance and user experience, the device
 * may request not to allow any rx ba session and tear down existing rx ba
 * sessions based on system constraints such as periodic BT activity that needs
 * to limit wlan activity (eg.sco or a2dp)."
 * in such cases, the intention is to limit the duration of the rx ppdu and
 * therefore prevent the peer device to use a-mpdu aggregation.
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @ba_rx_bitmap: Bit map of open rx ba per tid
 * @addr: & to bssid mac address
 */
void ieee80211_stop_rx_ba_session(struct ieee80211_vif *vif, u16 ba_rx_bitmap,
				  const u8 *addr);
#endif
#if 0

/**
 * ieee80211_send_bar - send a BlockAckReq frame
 *
 * can be used to flush pending frames from the peer's aggregation reorder
 * buffer.
 *
 * @vif: &struct ieee80211_vif pointer from the add_interface callback.
 * @ra: the peer's destination address
 * @tid: the TID of the aggregation session
 * @ssn: the new starting sequence number for the receiver
 */
void ieee80211_send_bar(struct ieee80211_vif *vif, u8 *ra, u16 tid, u16 ssn);
#endif
#ifndef CONFIG_RATE_HW_CONTROL
/* Rate control API */

/**
 * enum rate_control_changed - flags to indicate which parameter changed
 *
 * @IEEE80211_RC_HT_CHANGED: The HT parameters of the operating channel have
 *	changed, rate control algorithm can update its internal state if needed.
 */
enum rate_control_changed {
	IEEE80211_RC_HT_CHANGED = BIT(0)
};

/**
 * struct ieee80211_tx_rate_control - rate control information for/from RC algo
 *
 * @hw: The hardware the algorithm is invoked for.
 * @sband: The band this frame is being transmitted on.
 * @bss_conf: the current BSS configuration
 * @reported_rate: The rate control algorithm can fill this in to indicate
 *	which rate should be reported to userspace as the current rate and
 *	used for rate calculations in the mesh network.
 * @rts: whether RTS will be used for this frame because it is longer than the
 *	RTS threshold
 * @short_preamble: whether mac80211 will request short-preamble transmission
 *	if the selected rate supports it
 * @max_rate_idx: user-requested maximum rate (not MCS for now)
 *	(deprecated; this will be removed once drivers get updated to use
 *	rate_idx_mask)
 * @rate_idx_mask: user-requested rate mask (not MCS for now)
 * @skb: the skb that will be transmitted, the control information in it needs
 *	to be filled in
 * @bss: whether this frame is sent out in AP or IBSS mode
 */
struct ieee80211_tx_rate_control {
	struct ieee80211_hw *hw;
	struct ieee80211_supported_band *sband;
	struct ieee80211_bss_conf *bss_conf;
	struct sk_buff *skb;
	struct ieee80211_tx_rate reported_rate;
	bool rts, short_preamble;
	u8 max_rate_idx;
	u32 rate_idx_mask;
	bool bss;
};

struct rate_control_ops {
	struct module *module;
	const char *name;
	void *(*alloc)(struct ieee80211_hw *hw, struct dentry *debugfsdir);
	void (*free)(void *priv);

	void *(*alloc_sta)(void *priv, struct ieee80211_sta *sta, gfp_t gfp);
	void (*rate_init)(void *priv, struct ieee80211_supported_band *sband,
			  struct ieee80211_sta *sta, void *priv_sta);
	void (*rate_update)(void *priv, struct ieee80211_supported_band *sband,
			    struct ieee80211_sta *sta,
			    void *priv_sta, u32 changed,
			    enum nl80211_channel_type oper_chan_type);
	void (*free_sta)(void *priv, struct ieee80211_sta *sta,
			 void *priv_sta);

	void (*tx_status)(void *priv, struct ieee80211_supported_band *sband,
			  struct ieee80211_sta *sta, void *priv_sta,
			  struct sk_buff *skb);
	void (*get_rate)(void *priv, struct ieee80211_sta *sta, void *priv_sta,
			 struct ieee80211_tx_rate_control *txrc);

	void (*add_sta_debugfs)(void *priv, void *priv_sta,
				struct dentry *dir);
	void (*remove_sta_debugfs)(void *priv, void *priv_sta);
};

static inline int rate_supported(struct ieee80211_sta *sta,
				 enum ieee80211_band band,
				 int index)
{
	return (sta == NULL || sta->supp_rates[band] & BIT(index));
}

/**
 * rate_control_send_low - helper for drivers for management/no-ack frames
 *
 * Rate control algorithms that agree to use the lowest rate to
 * send management frames and NO_ACK data with the respective hw
 * retries should use this in the beginning of their mac80211 get_rate
 * callback. If true is returned the rate control can simply return.
 * If false is returned we guarantee that sta and sta and priv_sta is
 * not null.
 *
 * Rate control algorithms wishing to do more intelligent selection of
 * rate for multicast/broadcast frames may choose to not use this.
 *
 * @sta: &struct ieee80211_sta pointer to the target destination. Note
 * 	that this may be null.
 * @priv_sta: private rate control structure. This may be null.
 * @txrc: rate control information we sholud populate for mac80211.
 */
bool rate_control_send_low(struct ieee80211_sta *sta,
			   void *priv_sta,
			   struct ieee80211_tx_rate_control *txrc);
static inline s8
rate_lowest_index(struct ieee80211_supported_band *sband,
		  struct ieee80211_sta *sta)
{
	int i;

	for (i = 0; i < sband->n_bitrates; i++)
		if (rate_supported(sta, sband->band, i))
			return i;

	/* warn when we cannot find a rate. */
	WARN_ON_ONCE(1);

	/* and return 0 (the lowest index) */
	return 0;
}

static inline
bool rate_usable_index_exists(struct ieee80211_supported_band *sband,
			      struct ieee80211_sta *sta)
{
	unsigned int i;

	for (i = 0; i < sband->n_bitrates; i++)
		if (rate_supported(sta, sband->band, i))
			return true;
	return false;
}

int ieee80211_rate_control_register(struct rate_control_ops *ops);
void ieee80211_rate_control_unregister(struct rate_control_ops *ops);
#endif

static inline bool
conf_is_ht(struct ieee80211_channel_conf *conf)
{
	return conf->channel_type != NL80211_CHAN_NO_HT;
}

static inline enum nl80211_iftype
ieee80211_iftype_p2p(enum nl80211_iftype type, bool p2p)
{
	if (p2p) {
		switch (type) {
		case NL80211_IFTYPE_STATION:
			return NL80211_IFTYPE_P2P_CLIENT;
		case NL80211_IFTYPE_AP:
			return NL80211_IFTYPE_P2P_GO;
		default:
			break;
		}
	}
	return type;
}

static inline enum nl80211_iftype
ieee80211_vif_type_p2p(struct ieee80211_vif *vif)
{
	return ieee80211_iftype_p2p(vif->type, vif->p2p);
}
static inline void atbm_wdev_lock(struct wireless_dev *wdev)
	__acquires(wdev)
{
	mutex_lock(&wdev->mtx);
	__acquire(wdev->mtx);
}

static inline void atbm_wdev_unlock(struct wireless_dev *wdev)
	__releases(wdev)
{
	__release(wdev->mtx);
	mutex_unlock(&wdev->mtx);
}
#ifdef CONFIG_ATBM_MAC80211_NO_USE
void ieee80211_enable_rssi_reports(struct ieee80211_vif *vif,
				   int rssi_min_thold,
				   int rssi_max_thold);

void ieee80211_disable_rssi_reports(struct ieee80211_vif *vif);
#endif
#if defined(CONFIG_MAC80211_ATBM_MESH) || defined(ATBM_SURPORT_TDLS)
int ieee80211_add_srates_ie(struct ieee80211_vif *vif, struct sk_buff *skb);

int ieee80211_add_ext_srates_ie(struct ieee80211_vif *vif,
				struct sk_buff *skb);
#endif
struct sk_buff *ieee80211_special_queue_package(struct ieee80211_vif *vif,struct sk_buff *skb);
struct atbm_ewma {
	unsigned long internal;
	unsigned long factor;
	unsigned long weight;
};
static inline int atbm_ilog2(unsigned long v)
{
	int l = 0;
	while ((1UL << l) < v)
		l++;
	return l;
}

static inline void atbm_ewma_init(struct atbm_ewma *avg, unsigned long factor, unsigned long weight)
{

	avg->weight = atbm_ilog2(weight);
	avg->factor = atbm_ilog2(factor);
	avg->internal = 0;
}
/**
 * ewma_add() - Exponentially weighted moving average (EWMA)
 * @avg: Average structure
 * @val: Current value
 *
 * Add a sample to the average.
 */
static inline struct atbm_ewma *atbm_ewma_add(struct atbm_ewma *avg, unsigned long val)
{
	avg->internal = avg->internal  ?
		(((avg->internal << avg->weight) - avg->internal) +
			(val << avg->factor)) >> avg->weight :
		(val << avg->factor);
	return avg;
}
#define ATBM_DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(divisor) __divisor = divisor;		\
	(((x) + ((__divisor) / 2)) / (__divisor));	\
}							\
)

static inline unsigned long atbm_ewma_read(const struct atbm_ewma *avg)
{
	return avg->internal >> avg->factor;
}
static inline const u8 *atbm_ieee80211_find_ie(u8 eid, const u8 *ies, int len)
{
	while (len > 2 && ies[0] != eid) {
		len -= ies[1] + 2;
		ies += ies[1] + 2;
	}
	if (len < 2)
		return NULL;
	if (len < 2 + ies[1])
		return NULL;
	return ies;
}

static inline const u8 *atbm_ieee80211_find_vendor_ie(unsigned int oui, u8 oui_type,
				  const u8 *ies, int len)
{
	struct atbm_ieee80211_vendor_ie *ie;
	const u8 *pos = ies, *end = ies + len;
	int ie_oui;

	while (pos < end) {
		pos = atbm_ieee80211_find_ie(ATBM_WLAN_EID_VENDOR_SPECIFIC, pos,
				       end - pos);
		if (!pos)
			return NULL;

		ie = (struct atbm_ieee80211_vendor_ie *)pos;

		/* make sure we can access ie->len */
		BUILD_BUG_ON(offsetof(struct atbm_ieee80211_vendor_ie, len) != 1);

		if (ie->len < sizeof(*ie))
			goto cont;

		ie_oui = ie->oui[0] << 16 | ie->oui[1] << 8 | ie->oui[2];
		if (ie_oui == oui && ie->oui_type == oui_type)
			return pos;
cont:
		pos += 2 + ie->len;
	}
	return NULL;
}
#ifdef CONFIG_ATBM_SUPPORT_P2P
static inline u8* atbm_ieee80211_find_p2p_ie(const u8 *ie_start,size_t ie_len)
{
	if((ie_start == NULL)||(ie_len == 0))
		return NULL;
	return (u8*)atbm_ieee80211_find_vendor_ie(ATBM_OUI_WFA,ATBM_P2P_OUI_TYPE,ie_start,ie_len);
}

static inline u8* atbm_ieee80211_find_p2p_attr(u8* attr_start,ssize_t attr_len,u8 attr_id)
{
	if((attr_start == NULL)||(attr_len == 0))
		return NULL;
	// 3 = 1(Attribute ID) + 2(Length)
	while (attr_len > 2 && attr_start[0] != attr_id) {
		attr_len -= ATBM_WPA_GET_LE16((const u8*)(&attr_start[1])) + 3;
		attr_start += ATBM_WPA_GET_LE16((const u8*)(&attr_start[1])) + 3;
	}

	if(attr_len<3)
		return NULL;
	if (attr_len < 3 + ATBM_WPA_GET_LE16((const u8*)(&attr_start[1])))
		return NULL;

	return attr_start;
}

static inline int ieee80211_p2p_action_check(u8* data,ssize_t data_len)
{
	#define RETURN_ERR do{res = -1;goto action_check_end;}while(0)
	int res = 0;
	u8* p2p_data = data;
	ssize_t p2p_data_len = data_len;
	u32 oui_wfa = 0;
	
	if(p2p_data[0] != 9/*vendor spec*/)
		RETURN_ERR;

	if(p2p_data_len<3)
		RETURN_ERR;
	
	p2p_data++;
	p2p_data_len--;

	oui_wfa = (p2p_data[0] << 16) | (p2p_data[1] << 8) | p2p_data[2];

	if(oui_wfa != ATBM_OUI_WFA)
		RETURN_ERR;

	p2p_data += 3;
	p2p_data_len -= 3;
	if (p2p_data_len < 1)
		RETURN_ERR;

	if(p2p_data[0] != ATBM_P2P_OUI_TYPE)
		RETURN_ERR;

	p2p_data++;
	p2p_data_len--;

	if(p2p_data_len<1)
		RETURN_ERR;

	res = data_len-p2p_data_len;
action_check_end:
	return res;
}
#endif
#ifndef do_posix_clock_monotonic_gettime
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
#define ktime_get_ts ktime_get_ts64
#define timespec timespec64
#endif
#define do_posix_clock_monotonic_gettime(ts) ktime_get_ts(ts)
#endif

/*
*atbm timer function
*/
struct atbm_timer_list {
	struct timer_list timer;
	void (*function)(unsigned long data);
	unsigned long data;
	unsigned long expires;
};
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
static inline void atbm_timer_handle(struct timer_list *in_timer)
#else
static inline void atbm_timer_handle(unsigned long data)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	struct atbm_timer_list *atbm_timer = from_timer(atbm_timer, in_timer, timer);
#else
	struct atbm_timer_list *atbm_timer = (struct atbm_timer_list *)data;
#endif
	BUG_ON(atbm_timer->function == NULL);
	atbm_timer->function(atbm_timer->data);
}

static inline void atbm_init_timer(struct atbm_timer_list *atbm_timer)
{
	atbm_timer->expires = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	timer_setup(&atbm_timer->timer, atbm_timer_handle, 0);
#else
	/* setup_timer(ptimer, pfunc,(u32)cntx);	 */
	atbm_timer->timer.function = atbm_timer_handle;
	atbm_timer->timer.data = (unsigned long)atbm_timer;
	init_timer(&atbm_timer->timer);
#endif
}

static inline void atbm_setup_timer(struct atbm_timer_list *atbm_timer,
				   void (*function)(unsigned long data),unsigned long data)
{
	atbm_timer->expires = 0;
	atbm_timer->function = function;
	atbm_timer->data = data;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	timer_setup(&atbm_timer->timer, atbm_timer_handle, 0);
#else
	
	atbm_timer->timer.function = atbm_timer_handle;
	atbm_timer->timer.data = (unsigned long)atbm_timer;
	init_timer(&atbm_timer->timer);
#endif
}
static inline int atbm_del_timer_sync(struct atbm_timer_list *atbm_timer)
{
	return del_timer_sync(&atbm_timer->timer);
}
static inline int atbm_mod_timer(struct atbm_timer_list *atbm_timer, unsigned long expires)
{
	int ret = 0;
	ret =  mod_timer(&atbm_timer->timer,expires);
	atbm_timer->expires = atbm_timer->timer.expires;

	return ret;
}

static inline void atbm_add_timer(struct atbm_timer_list *atbm_timer)
{
	atbm_timer->timer.expires = atbm_timer->expires;
	add_timer(&atbm_timer->timer);
}

static inline int atbm_del_timer(struct atbm_timer_list *atbm_timer)
{
	return del_timer(&atbm_timer->timer);
}
static inline int atbm_timer_pending(struct atbm_timer_list *atbm_timer)
{
	return timer_pending(&atbm_timer->timer);
}
extern struct cfg80211_bss *ieee80211_atbm_get_bss(struct wiphy *wiphy,
				      struct ieee80211_channel *channel,
				      const u8 *bssid,
				      const u8 *ssid, size_t ssid_len,
				      u16 capa_mask, u16 capa_val);
extern void ieee80211_atbm_put_authen_bss(struct ieee80211_vif *vif,struct cfg80211_bss *bss);
extern void __ieee80211_atbm_put_authen_bss(struct ieee80211_vif *vif,struct cfg80211_bss *bss);

extern void ieee80211_atbm_put_bss(struct wiphy *wiphy, struct cfg80211_bss *pub);
extern int ieee80211_atbm_handle_bss(struct wiphy *wiphy, struct cfg80211_bss *pub);
extern void ieee80211_atbm_release_bss(struct wiphy *wiphy, struct cfg80211_bss *pub);
extern struct cfg80211_bss *ieee80211_atbm_get_authen_bss(struct ieee80211_vif *vif,
					  struct ieee80211_channel *channel,
				      const u8 *bssid,
				      const u8 *ssid, size_t ssid_len);
extern struct cfg80211_bss *__ieee80211_atbm_get_authen_bss(struct ieee80211_vif *vif,
					  struct ieee80211_channel *channel,
				      const u8 *bssid,
				      const u8 *ssid, size_t ssid_len);
extern char *ieee80211_alloc_name(struct ieee80211_hw *hw,const char *name);
extern bool atbm_ieee80211_is_robust_mgmt_frame(struct sk_buff *skb);
extern void ieee80211_event_work(struct ieee80211_vif *vif,u32 id,u32 data);


#define IEEE80211_CHANNLE_SPECIAL_FREQ_FLAG				BIT(15)
#define IEEE80211_CHANNLE_CCA_RUNNING_FLAG				BIT(14)
#define IEEE80211_CHANNLE_NO_IR							BIT(13)

#define IEEE80211_CHANNLE_HW_VALUE_MASK (IEEE80211_CHANNLE_SPECIAL_FREQ_FLAG|IEEE80211_CHANNLE_CCA_RUNNING_FLAG|IEEE80211_CHANNLE_NO_IR)

static inline u16 channel_hw_value(struct ieee80211_channel *channel)
{
	return (channel->hw_value)&(~IEEE80211_CHANNLE_HW_VALUE_MASK);
}
static inline void channel_mask_no_ir(struct ieee80211_channel *channel)
{
	channel->hw_value |= IEEE80211_CHANNLE_NO_IR;
}
static inline void channel_clear_no_ir(struct ieee80211_channel *channel)
{
	channel->hw_value &= ~IEEE80211_CHANNLE_NO_IR;
}

static inline void channel_mask_special(struct ieee80211_channel *channel)
{
	channel->hw_value |= IEEE80211_CHANNLE_SPECIAL_FREQ_FLAG;
}
static inline void channel_mask_cca(struct ieee80211_channel *channel)
{
	channel->hw_value |= IEEE80211_CHANNLE_CCA_RUNNING_FLAG;
}
static inline void channel_clear_special(struct ieee80211_channel *channel)
{
	channel->hw_value &= ~IEEE80211_CHANNLE_SPECIAL_FREQ_FLAG;
}
static inline void channel_clear_cca(struct ieee80211_channel *channel)
{
	channel->hw_value &= ~IEEE80211_CHANNLE_CCA_RUNNING_FLAG;
}
static inline int channel_center_freq(struct ieee80211_channel *channel)
{
	return channel->center_freq;
}
static inline bool channel_in_special(struct ieee80211_channel *channel)
{
	return (channel->hw_value&IEEE80211_CHANNLE_SPECIAL_FREQ_FLAG) ? true:false;
}
static inline bool channel_in_cca(struct ieee80211_channel *channel)
{
	return (channel->hw_value&IEEE80211_CHANNLE_CCA_RUNNING_FLAG) ? true:false;
}
static inline bool channel_is_no_ir(struct ieee80211_channel *channel)
{
	return (channel->hw_value&IEEE80211_CHANNLE_NO_IR) ? true:false;
}

static inline bool atbm_accsii_to_hex(char pos,char *res)
{
	if((pos>='0')&&(pos<='9')){
		*res = pos-'0';
		return true;
	}else if((pos>='a')&&(pos<='f')){
		*res = pos - 'a'+0x0a;
		return true;
	}else if((pos>='A')&&(pos<='F')){
		*res = pos - 'A'+0x0a;
		return true;
	}

	return false;
}

static inline bool atbm_accsii_to_hex_V2(const char *pos,int len,u32 *res)
{
    int index = 0;
    int     got_hex = false;
    unsigned int  d = 0;
	const char *accsii = pos;

    for(;index<len;index++)
    {
        char    ch;

        ch = accsii[index];
        if (ch == 0)
        {
            break;
        }
        if (ch >= '0' && ch <= '9')
        {
            d = (d<<4) | (ch - '0');
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            d = (d<<4) | (ch - 'a' + 10);
        }
        else if (ch >= 'A' && ch <= 'F')
        {
            d = (d<<4) | (ch - 'A' + 10);
        }
        else
        {
            got_hex = 0;
            break;
        }
        got_hex = true;
    }
    if (got_hex)
    {
        *res = d;
    }

    return got_hex;
}


static inline bool atbm_accsii_to_int(const char *pos,int len,int *res)
{
	int res_val = 0;
	int index = 0;
	bool neg = false;
	const char *accsii = pos;

	if((len == 0) || (len>10) || (len<0))
		return false;
	
	neg   = pos[0] == '-'  ? true:false;
	index = neg    == true ? 1:0;

	if((neg==true)&&(len == 1))
		return false;
	
	for(;index<len;index++){
		
		if((accsii[index] < '0') || (accsii[index] > '9')){
			return false;
		}

		res_val = res_val*10;

		res_val += accsii[index]-'0';
	}

	res_val = neg == true?0-res_val:res_val;

	*res = res_val;

	return true;
}

static inline bool atbm_accsii_to_int_10x(const char *pos,int len,int *res)
{
	int res_val = 0;
	int index = 0;
	bool neg = false;
	const char *accsii = pos;
	bool bfloat = false;

	if((len == 0) || (len>10) || (len<0))
		return false;
	
	neg   = pos[0] == '-'  ? true:false;
	index = neg    == true ? 1:0;

	if((neg==true)&&(len == 1))
		return false;
	
	for(;index<len;index++){
		
		if((accsii[index] != '.') && ((accsii[index] < '0') || (accsii[index] > '9'))){
			return false;
		}

		if(accsii[index] == '.')
		{
			bfloat = true;
			continue;
		}

		res_val = res_val*10;

		res_val += accsii[index]-'0';
	}

	res_val = neg == true?0-res_val:res_val;

	if(bfloat == false)
		res_val = res_val*10;

	*res = res_val;

	return true;
}

struct atbm_decimal_fraction{
	int interger;
	int decimal;
	int digits;
};
static inline bool atbm_accsii_to_decimal(const char *pos,int len,struct atbm_decimal_fraction *res)
{
	const char *accsii = pos;
	const char *decimal = memchr(accsii,'.',len);
	int interger_len = len;

	memset(res,0,sizeof(struct atbm_decimal_fraction));
	
	if(decimal){
		
		res->digits = len - (decimal + 1 - pos);
		
		if(atbm_accsii_to_int(decimal + 1,res->digits,&res->decimal) == false){
			return false;
		}
		
		interger_len = len - res->digits - 1;
	}

	return atbm_accsii_to_int(accsii,interger_len,&res->interger);
}
static inline void atbm_common_hash_list_init(struct hlist_head *hlist,size_t size)
{
	size_t index = 0;
	
	for(index = 0;index<size;index++){
		INIT_HLIST_HEAD(&hlist[index]);
	}
}

static inline unsigned int atbm_hash_index(const char *string,unsigned int len,unsigned int hash_bit)
{
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
	unsigned int hash = full_name_hash(NULL,string,len);
	#else
	unsigned int hash = full_name_hash(string,len);
	#endif

	return hash_32(hash, hash_bit);
}

static inline struct hlist_head *atbm_hash_list(const char *string,unsigned int len,struct hlist_head *hlist_table,unsigned int hash_bit)
{
	unsigned int hash = atbm_hash_index(string,len,hash_bit);

	return &hlist_table[hash];
}
static inline bool ieee8011_channel_valid(struct ieee80211_hw *hw,int channel)
{
	int freq;
	struct ieee80211_channel *ch;
	if(channel<=0)
		return false;

	if(channel<14){
		freq = 2412+(channel-1)*5;
	}else if(channel == 14){
		freq = 2484;
	}else{
		freq = 5000 + 5 * channel;
	}

	ch = ieee80211_get_channel(hw->wiphy,freq);

	return ch == NULL ? false:true;
}

static inline struct ieee80211_channel *ieee8011_chnum_to_channel(struct ieee80211_hw *hw,int channel)
{
	int freq;
	if(channel<=0)
		return NULL;

	if(channel<14){
		freq = 2412+(channel-1)*5;
	}else if(channel == 14){
		freq = 2484;
	}else{
		freq = 5000 + 5 * channel;
	}

	return ieee80211_get_channel(hw->wiphy,freq);
}
static inline u8 ieee80211_rssi_weight(s8 signal)
{
	u8 weight = 0;

	if(signal <= -75)
		weight = 0;
	else if((signal>-75)&&(signal<= -65))
		weight = 1;
	else if((signal>-65)&&(signal<= -50))
		weight = 2;
	else if(signal > -50)
		weight = 4;
	else 
		weight = 0;

	return weight;
}
int ieee80211_ratelimit(void);
struct ieee80211_vif *ieee80211_get_monitor_vif(struct ieee80211_hw *hw);

#define ATBM_IEEE80211_MAX_RX_AGGR			128
#define ATBM_IEEE80211_MAX_TX_AGGR			128

#define ATBM_SPACE	' '
#define ATBM_LINEF	'\n'
#define ATBM_ENTER	'\r'
#define ATBM_TAIL	'\0'
#define ATBM_EQUAL	'='
#define ATBM_UNUSED	'#'
#define ATBM_SOH	1
#define ATBM_EOT	4
#define ATBM_STX	2
#define ATBM_ETX	3
#define ATBM_SPACE_STR	" "
#define ATBM_LINEF_STR	"\n"
#define ATBM_DECIMAL '.'

#define ATBM_COMMON_HASHBITS    4
#define ATBM_COMMON_HASHENTRIES (1 << ATBM_COMMON_HASHBITS)

#define ATBM_MAX_SCAN_IE							1
#define ATBM_MAX_SCAN_SSID							2
#define ATBM_MAX_SCAN_MAC_FILTER					8
#define ATBM_MAX_SCAN_PRIVATE_IE_LEN				(255-4)
#define ATBM_MAX_SCAN_CHANNEL						(14+4)

#define ATBM_PRINTK_MASK_ERR			BIT(0)
#define ATBM_PRINTK_MASK_WARN			BIT(1)
#define ATBM_PRINTK_MASK_INIT			BIT(2)
#define ATBM_PRINTK_MASK_EXIT			BIT(3)
#define ATBM_PRINTK_MASK_BUS			BIT(4)
#define ATBM_PRINTK_MASK_SCAN			BIT(5)
#define ATBM_PRINTK_MASK_P2P			BIT(6)
#define ATBM_PRINTK_MASK_MGMT			BIT(7)
#define ATBM_PRINTK_MASK_LMAC			BIT(8)
#define ATBM_PRINTK_MASK_AGG			BIT(9)
#define ATBM_PRINTK_MASK_AP				BIT(10)
#define ATBM_PRINTK_MASK_STA			BIT(11)
#define ATBM_PRINTK_MASK_SMARTCONFIG	BIT(12)
#define ATBM_PRINTK_MASK_WEXT			BIT(13)
#define ATBM_PRINTK_MASK_TX				BIT(14)
#define ATBM_PRINTK_MASK_RX				BIT(15)
#define ATBM_PRINTK_MASK_PM				BIT(16)
#define ATBM_PRINTK_MASK_PLATFROM		BIT(17)
#define ATBM_PRINTK_MASK_BH				BIT(18)
#define ATBM_PRINTK_MASK_CFG80211		BIT(19)
#define ATBM_PRINTK_MASK_DEBUG			BIT(20)
#define ATBM_PRINTK_MASK_BLE			BIT(21)

#define ATBM_PRINTK_DEFAULT_MASK	(ATBM_PRINTK_MASK_ERR|ATBM_PRINTK_MASK_WARN|ATBM_PRINTK_MASK_INIT|ATBM_PRINTK_MASK_AGG| \
									ATBM_PRINTK_MASK_LMAC|ATBM_PRINTK_MASK_EXIT|ATBM_PRINTK_MASK_SCAN|ATBM_PRINTK_MASK_PM | ATBM_PRINTK_MASK_WEXT)
extern const char *atbm_log;

#ifdef CONFIG_ATBM_MOULE_FS
extern u32 atbm_printk_mask;
#else
#define atbm_printk_mask	ATBM_PRINTK_DEFAULT_MASK
#endif

#define ATBM_PRINTK_ALL ((u32)(-1))
#define ATBM_PRINTK_CLEAR		(0)
#define ATBM_TAG "[atbm_log]:"
#define atbm_printk(_level,fmt,arg...) 	do {if(atbm_printk_mask&(_level)) printk(KERN_ERR "%s" fmt,atbm_log,##arg);}while(0)


/*
*atbm printk
*/
#define atbm_printk_err(...) 		atbm_printk(ATBM_PRINTK_MASK_ERR,__VA_ARGS__)
#define atbm_printk_warn(...)		atbm_printk(ATBM_PRINTK_MASK_WARN,__VA_ARGS__)
#define atbm_printk_init(...)		atbm_printk(ATBM_PRINTK_MASK_INIT,__VA_ARGS__)
#define atbm_printk_exit(...)		atbm_printk(ATBM_PRINTK_MASK_EXIT,__VA_ARGS__)
#define atbm_printk_bus(...)		atbm_printk(ATBM_PRINTK_MASK_BUS,__VA_ARGS__)
#define atbm_printk_scan(...)		atbm_printk(ATBM_PRINTK_MASK_SCAN,__VA_ARGS__)
#define atbm_printk_p2p(...)		atbm_printk(ATBM_PRINTK_MASK_P2P,__VA_ARGS__)
#define atbm_printk_mgmt(...)		atbm_printk(ATBM_PRINTK_MASK_MGMT,__VA_ARGS__)
#define atbm_printk_lmac(...)		atbm_printk(ATBM_PRINTK_MASK_LMAC,__VA_ARGS__)
#define atbm_printk_agg(...)		atbm_printk(ATBM_PRINTK_MASK_AGG,__VA_ARGS__)
#define atbm_printk_ap(...)			atbm_printk(ATBM_PRINTK_MASK_AP,__VA_ARGS__)
#define atbm_printk_sta(...)		atbm_printk(ATBM_PRINTK_MASK_STA,__VA_ARGS__)
#define atbm_printk_smt(...)		atbm_printk(ATBM_PRINTK_MASK_SMARTCONFIG,__VA_ARGS__)
#define atbm_printk_wext(...)		atbm_printk(ATBM_PRINTK_MASK_WEXT,__VA_ARGS__)
#define atbm_printk_tx(...)			atbm_printk(ATBM_PRINTK_MASK_TX,__VA_ARGS__)
#define atbm_printk_rx(...)			atbm_printk(ATBM_PRINTK_MASK_RX,__VA_ARGS__)
#define atbm_printk_pm(...)			atbm_printk(ATBM_PRINTK_MASK_PM,__VA_ARGS__)
#define atbm_printk_platform(...)	atbm_printk(ATBM_PRINTK_MASK_PLATFROM,__VA_ARGS__)
#define atbm_printk_bh(...)			atbm_printk(ATBM_PRINTK_MASK_BH,__VA_ARGS__)
#define atbm_printk_cfg(...)		atbm_printk(ATBM_PRINTK_MASK_CFG80211,__VA_ARGS__)
#define atbm_printk_debug(...)		atbm_printk(ATBM_PRINTK_MASK_DEBUG,__VA_ARGS__)
#define atbm_printk_ble(...)		atbm_printk(ATBM_PRINTK_MASK_BLE,__VA_ARGS__)
#define atbm_printk_always(fmt,arg...)		printk(KERN_ERR "%s" fmt,atbm_log,##arg)
#define atbm_printk_limit(fmt,arg...)	do{if(ieee80211_ratelimit()) printk(KERN_ERR "%s" fmt,atbm_log,##arg);}while(0)


#define ATBM_MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define ATBM_MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0))
#ifdef fallthrough

#define atbm_fallthrough fallthrough
#else

#define atbm_fallthrough
#endif


static inline const char* atbm_skip_space(const char *target,ssize_t len)
{
	const char* pos_end = target+len;

	if((len <= 0)||(target == NULL)){
		return NULL;
	}
	while((*target == ATBM_SPACE)||(*target == ATBM_LINEF)||(*target == ATBM_ENTER)){
		target++;
		if(target == pos_end){
			break;
		}
	}
	return (target == pos_end? NULL:target);
}

#define ieee80211_chw_is_ht40(chtype) (((chtype) == NL80211_CHAN_HT40PLUS) || ((chtype) == NL80211_CHAN_HT40MINUS))
#define LIGHT	"\e[1m"
#define NORMAL	"\e[0m"
#define ENTER	"\n"
#define highlight_debug(fmt,arg...)			\
	printk(KERN_ERR LIGHT fmt NORMAL ENTER,##arg)

#ifdef MODULE
#define atbm_module_parent			(&THIS_MODULE->mkobj.kobj)
#else
#define atbm_module_parent			(NULL)
#endif

#endif /* MAC80211_H */
