/*
 * Copyright (C) 2013 Alexey Galakhov <agalakhov@gmail.com>
 *
 * Licensed under the GNU General Public License Version 3
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "std.h"

enum capt_command {
	CAPT_NOP        = 0xA0A0, /* ? */
	CAPT_CHKJOBSTAT = 0xA0A1,
	CAPT_CHKXSTATUS = 0xA0A8,

	CAPT_IEEE_IDENT = 0xA1A0, /* raw reply */
	CAPT_IDENT      = 0xA1A1,

	CAPT_JOB_BEGIN  = 0xA2A0,

	CAPT_START_0    = 0xA3A2,

	CAPT_PRINT_DATA     = 0xC0A0,
	CAPT_PRINT_DATA_END = 0xC0A4,

	CAPT_SET_PARM_PAGE   = 0xD0A0,
	CAPT_SET_PARM_1      = 0xD0A1,
	CAPT_SET_PARM_2      = 0xD0A2,
	CAPT_SET_PARM_HISCOA = 0xD0A4,
	CAPT_SET_PARMS  = 0xD0A9,

	CAPT_CHKSTATUS  = 0xE0A0,
	CAPT_START_2    = 0xE0A2,
	CAPT_START_1    = 0xE0A3,
	CAPT_START_3    = 0xE0A4,
	CAPT_UPLOAD_2   = 0xE0A5,
	CAPT_FIRE       = 0xE0A7,
	CAPT_JOB_END    = 0xE0A9,

	CAPT_JOB_SETUP  = 0xE1A1,
	CAPT_GPIO       = 0xE1A2,
	CAPT3_UNK_0     = 0xE0BA,
};


const char *capt_identify(void);

void capt_send(uint16_t cmd, const void *data, size_t size);
void capt_sendrecv(uint16_t cmd, const void *buf, size_t size, void *reply, size_t *reply_size);

void capt_multi_begin(uint16_t cmd);
void capt_multi_add(uint16_t cmd, const void *data, size_t size);
void capt_multi_send(void);
void capt_cleanup(void);
