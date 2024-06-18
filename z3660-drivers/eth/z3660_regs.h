/*
 * Z3660 Graphics Card Driver based on MNT ZZ9000 rev 1.13
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 */

/*
 * MNT ZZ9000 Amiga Graphics Card Driver (ZZ9000.card)
 * Copyright (C) 2016-2020, Lukas F. Hartmann <lukas@mntre.com>
 *                          MNT Research GmbH, Berlin
 *                          https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 */

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned long

#define u16 uint16_t
#define u32 uint32_t

enum zz_reg_offsets {
   REG_ZZ_CONFIG         = 0x104,

   REG_ZZ_ETH_TX         = 0x190,
   REG_ZZ_ETH_RX         = 0x194,
   REG_ZZ_ETH_MAC_HI     = 0x198,
   REG_ZZ_ETH_MAC_LO     = 0x19C,

   REG_ZZ_FW_VERSION     = 0x1A0,
   REG_ZZ_ETH_RX_ADDRESS = 0x1A4,
   REG_ZZ_INT_STATUS     = 0x1A8,
};

#define TX_FRAME_ADDRESS 0x07F00000
