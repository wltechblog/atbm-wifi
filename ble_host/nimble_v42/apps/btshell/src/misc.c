/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "syscfg/syscfg.h"

#if MYNEWT_VAL_SHELL_CMD

#include "host/ble_uuid.h"
#include "host/ble_gap.h"

#include "btshell.h"



/**
 * Utility function to log an array of bytes.
 */
SRAM_CODE void
print_bytes(const uint8_t *bytes, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        iot_printf("%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

SRAM_CODE void
print_mbuf(const struct os_mbuf *om)
{
    int colon;

    colon = 0;
    while (om != NULL) {
        if (colon) {
            iot_printf(":");
        } else {
            colon = 1;
        }
        print_bytes(om->om_data, om->om_len);
        om = SLIST_NEXT(om, om_next);
    }
}

SRAM_CODE void
print_addr(const void *addr)
{
    const uint8_t *u8p;

    u8p = addr;
    iot_printf("%02x:%02x:%02x:%02x:%02x:%02x",
                   u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}

SRAM_CODE void
print_uuid(const ble_uuid_t *uuid)
{
    char buf[BLE_UUID_STR_LEN];

    ble_uuid_to_str(uuid, buf);

    iot_printf("%s", buf);
}

SRAM_CODE int
svc_is_empty(const struct btshell_svc *svc)
{
    return svc->svc.end_handle <= svc->svc.start_handle;
}

SRAM_CODE uint16_t
chr_end_handle(const struct btshell_svc *svc, const struct btshell_chr *chr)
{
    const struct btshell_chr *next_chr;

    next_chr = SLIST_NEXT(chr, next);
    if (next_chr != NULL) {
        return next_chr->chr.def_handle - 1;
    } else {
        return svc->svc.end_handle;
    }
}

SRAM_CODE int
chr_is_empty(const struct btshell_svc *svc, const struct btshell_chr *chr)
{
    return chr_end_handle(svc, chr) <= chr->chr.val_handle;
}

SRAM_CODE void
print_conn_desc(const struct ble_gap_conn_desc *desc)
{
    iot_printf("handle=%d our_ota_addr_type=%d our_ota_addr=",
                   desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);
    iot_printf(" our_id_addr_type=%d our_id_addr=",
                   desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);
    iot_printf(" peer_ota_addr_type=%d peer_ota_addr=",
                   desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);
    iot_printf(" peer_id_addr_type=%d peer_id_addr=",
                   desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);
    iot_printf(" conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                   "encrypted=%d authenticated=%d bonded=%d\n",
                   desc->conn_itvl, desc->conn_latency,
                   desc->supervision_timeout,
                   desc->sec_state.encrypted,
                   desc->sec_state.authenticated,
                   desc->sec_state.bonded);
}

SRAM_CODE static void
print_dsc(struct btshell_dsc *dsc)
{
    iot_printf("            dsc_handle=%d uuid=", dsc->dsc.handle);
    print_uuid(&dsc->dsc.uuid.u);
    iot_printf("\n");
}

SRAM_CODE static void
print_chr(struct btshell_chr *chr)
{
    struct btshell_dsc *dsc;

    iot_printf("        def_handle=%d val_handle=%d properties=0x%02x "
                   "uuid=", chr->chr.def_handle, chr->chr.val_handle,
                   chr->chr.properties);
    print_uuid(&chr->chr.uuid.u);
    iot_printf("\n");

    SLIST_FOREACH(dsc, &chr->dscs, next) {
        print_dsc(dsc);
    }
}

SRAM_CODE void
print_svc(struct btshell_svc *svc)
{
    struct btshell_chr *chr;

    iot_printf("    start=%d end=%d uuid=", svc->svc.start_handle,
                   svc->svc.end_handle);
    print_uuid(&svc->svc.uuid.u);
    iot_printf("\n");

    SLIST_FOREACH(chr, &svc->chrs, next) {
        print_chr(chr);
    }
}
#endif  //MYNEWT_VAL_SHELL_CMD