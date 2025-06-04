// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 Semenets V. Pavel
 */

#include <stdint.h>
#include <env.h>
#include <compiler.h>
#include <tlv_eeprom.h>
#include <linux/kernel.h>
#include "tlv_data.h"

/*
#define TLV_CODE_PRODUCT_NAME   0x21
#define TLV_CODE_PART_NUMBER    0x22
#define TLV_CODE_SERIAL_NUMBER  0x23
#define TLV_CODE_MAC_BASE       0x24
#define TLV_CODE_MANUF_DATE     0x25
#define TLV_CODE_DEVICE_VERSION 0x26
#define TLV_CODE_LABEL_REVISION 0x27
#define TLV_CODE_PLATFORM_NAME  0x28
#define TLV_CODE_ONIE_VERSION   0x29
#define TLV_CODE_MAC_SIZE       0x2A
#define TLV_CODE_MANUF_NAME     0x2B
#define TLV_CODE_MANUF_COUNTRY  0x2C
#define TLV_CODE_VENDOR_NAME    0x2D
#define TLV_CODE_DIAG_VERSION   0x2E
#define TLV_CODE_SERVICE_TAG    0x2F
#define TLV_CODE_VENDOR_EXT     0xFD
#define TLV_CODE_CRC_32         0xFE
*/

static void set_mac_len(struct tlvinfo_tlv *tlv_entry,
                   struct tlv_data *td)
{
    uint16_t t_maclen;

    memcpy(&t_maclen, tlv_entry->value, sizeof(t_maclen));
    td->maclen = be16_to_cpu(t_maclen);
}

static void set_mac_address(struct tlvinfo_tlv *tlv_entry,
                   struct tlv_data *td)
{
    u8 *mac;
    mac = td->macbase;

    memcpy(mac, tlv_entry->value, sizeof(mac));
}

static void set_mac_addr(struct tlv_data *td) {
    int     i;
    char    t_mac[18];
    char    enetvar[11];

    for (i = 0; i < td->maclen; i++) {
        snprintf(enetvar, sizeof(enetvar), i >= 1 ? "eth%daddr" : "ethaddr", i);

        if (i < 1) {
            snprintf(t_mac, sizeof(t_mac), "%02X:%02X:%02X:%02X:%02X:%02X", td->macbase[0], td->macbase[1], td->macbase[2], td->macbase[3], td->macbase[4], td->macbase[5]);
        } else {
            td->macbase[5]++;
			if (td->macbase[5] == 0) {
				td->macbase[4]++;
				if (td->macbase[4] == 0) {
					td->macbase[3]++;
					if (td->macbase[3] == 0) {
						td->macbase[0] = 0;
						td->macbase[1] = 0;
						td->macbase[2] = 0;
					}
				}
			}
            
            snprintf(t_mac, sizeof(t_mac), "%02X:%02X:%02X:%02X:%02X:%02X", td->macbase[0], td->macbase[1], td->macbase[2], td->macbase[3], td->macbase[4], td->macbase[5]);
        }

        env_set(enetvar, t_mac);
    }
}

static void parse_tlv_data(u8 *eeprom, struct tlvinfo_header *hdr,
               struct tlvinfo_tlv *entry, struct tlv_data *td)
{
    unsigned int tlv_offset, tlv_len;

    tlv_offset = sizeof(struct tlvinfo_header);
    tlv_len = sizeof(struct tlvinfo_header) + be16_to_cpu(hdr->totallen);
    while (tlv_offset < tlv_len) {
        entry = (struct tlvinfo_tlv *)&eeprom[tlv_offset];

        switch (entry->type) {
            case TLV_CODE_MAC_BASE:
                set_mac_address(entry, td);
                break;
            case TLV_CODE_MAC_SIZE:
                set_mac_len(entry, td);
                break;
            default:
                break;
        }

        tlv_offset += sizeof(struct tlvinfo_tlv) + entry->length;
    }
}

void read_tlv_data(struct tlv_data *td)
{
    u8 eeprom_data[TLV_TOTAL_LEN_MAX];
    struct tlvinfo_header *tlv_hdr;
    struct tlvinfo_tlv *tlv_entry;
    int ret, i;

    for (i = 0; i < 2; i++) {
        ret = read_tlvinfo_tlv_eeprom(eeprom_data, &tlv_hdr,
                          &tlv_entry, i);
        if (ret < 0)
            continue;

        parse_tlv_data(eeprom_data, tlv_hdr, tlv_entry, td);

        set_mac_addr(td);
    }
}
