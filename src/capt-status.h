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

struct capt_status_s {
	uint16_t status[7];

	uint16_t page_decoding;
	uint16_t page_printing;
	uint16_t page_out;
	uint16_t page_completed;
};

#define _FL(s, b) ((s << 16) | (1 << b))
enum capt_flags
{
	/* status[0] */
	CAPT_FL_READY1       = _FL(0, 15), /* ? */
	CAPT_FL_READY2       = _FL(0, 12), /* ? */
	CAPT_FL_JOBSTAT_CHNG = _FL(0, 9),
	CAPT_FL_XSTATUS_CHNG = _FL(0, 8),
	CAPT_FL_BUSY         = _FL(0, 7),
	CAPT_FL_UNINIT1      = _FL(0, 5),
	CAPT_FL_UNINIT2      = _FL(0, 4),
	CAPT_FL_BUFFERFULL   = _FL(0, 2),
	CAPT_FL_NOPAPER1     = _FL(0, 1),
	CAPT_FL_PROCESSING   = _FL(0, 0),
	/* status[1] */
	CAPT_FL_NOPAPER2     = _FL(1, 14),
	CAPT_FL_PROCESSING1  = _FL(1, 7),
	CAPT_FL_BUTTON       = _FL(1, 5),
	CAPT_FL_PRINTING     = _FL(1, 2),
	CAPT_FL_POWERUP      = _FL(1, 0),
	/* status[2] */
	CAPT_FL_nERROR       = _FL(2, 7),
	CAPT_FL_BUTTON1      = _FL(2, 8),
	/* status[3] */
	CAPT_FL_POWERUP1     = _FL(3, 12),
	/* status[4] */
	/* status[5] */
	/* status[6] */
};
#undef _FL

static inline bool FLAG(const struct capt_status_s *status, enum capt_flags flag)
{
	return !! (status->status[flag >> 16] & (flag & 0xFFFF));
}

void capt_init_status(void);
const struct capt_status_s *capt_get_status(void);
const struct capt_status_s *capt_get_xstatus(void);
void capt_wait_ready(void);
