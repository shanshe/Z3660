/*
 * ethernet.h
 *
 *  Created on: 23 sept. 2022
 *      Author: shanshe
 */

/*
 * MNT ZZ9000 Amiga Graphics and Coprocessor Card Operating System (ZZ9000OS)
 *
 * Copyright (C) 2019, Lukas F. Hartmann <lukas@mntre.com>
 *                     MNT Research GmbH, Berlin
 *                     https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 *
*/

#ifndef ETHERNET_H_
#define ETHERNET_H_

int ethernet_init();
uint16_t ethernet_send_frame(uint16_t frame_size);
int ethernet_receive_frame();
uint32_t get_frames_received();
uint8_t* ethernet_get_mac_address_ptr();
void ethernet_update_mac_address();
volatile uint8_t* ethernet_current_receive_ptr();
int ethernet_get_backlog();
void ethernet_task();

#endif
