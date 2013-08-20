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

#include "capt-status.h"

#include "std.h"
#include "word.h"
#include "capt-command.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static size_t  status_size = 0;
static uint8_t status_rec[0x10000];

uint16_t capt_get_status(void)
{
	status_size = sizeof(status_rec);
	capt_sendrecv(CAPT_CHKSTATUS, NULL, 0, status_rec, &status_size);
	return WORD(status_rec[0], status_rec[1]);
}

uint16_t capt_get_xstatus(void)
{
	uint16_t retval = capt_get_status();
	if (retval & CAPT_ST_XS) {
		status_size = sizeof(status_rec);
		capt_sendrecv(CAPT_CHKXSTATUS, NULL, 0, status_rec, &status_size);
		retval = WORD(status_rec[0], status_rec[1]);
	}
	return retval;
}

void capt_wait_ready(void)
{
	while (capt_get_status() & CAPT_ST_BU)
		sleep(1);
}

const uint8_t *capt_get_status_rec(void)
{
	return status_rec + 2;
}

uint16_t capt_get_status_size(void)
{
	return status_size - 2;
}
