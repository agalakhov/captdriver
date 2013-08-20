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

enum capt_status_flags
{
	/* [R1][0][0][0]-[R2][0][R3][XS]-[BU][0][NR1][NR2]-[?][PR][NP][NR] */

	CAPT_ST_NR  = (1 << 0),
	CAPT_ST_NR3 = (1 << 1),
	CAPT_ST_PR  = (1 << 2),
	/* ? = (1 << 3), */

	CAPT_ST_NR2 = (1 << 4),
	CAPT_ST_NR1 = (1 << 5),
	/* 0 = (1 << 6), */
	CAPT_ST_BU  = (1 << 7),

	CAPT_ST_XS  = (1 << 8),
	CAPT_ST_R3  = (1 << 9),
	/* 0 = (1 << 10), */
	CAPT_ST_R2  = (1 << 11),

	/* 0 = (1 << 12), */
	/* 0 = (1 << 13), */
	/* 0 = (1 << 14), */
	CAPT_ST_R1  = (1 << 15)
};

uint16_t capt_get_status(void);
uint16_t capt_get_xstatus(void);
void capt_wait_ready(void);
const uint8_t *capt_get_status_rec(void);
uint16_t capt_get_status_size(void);
