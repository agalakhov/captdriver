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

#include "generic-ops.h"

#include "std.h"
#include "hiscoa-common.h"
#include "hiscoa-compress.h"
#include "capt-command.h"
#include "capt-status.h"
#include "printer.h"

#include <stdlib.h>

void ops_send_band_hiscoa(struct printer_state_s *state,
		const void *data, unsigned line_size, unsigned num_lines)
{
	const uint8_t *band_ptr;
	size_t band_size;
	void *band_buf = calloc(2 * line_size, num_lines);
	if (! band_buf)
		abort();

	// FIXME
	band_size = hiscoa_compress_band(band_buf, 2 * line_size * num_lines,
						data, line_size, num_lines,
						0, &hiscoa_default_params);

	band_ptr = band_buf;
	while (band_size) {
		size_t send = 0xFF00;
		if (send > band_size)
			send = band_size;
		state->isend += 1;
		if (state->isend % 16 == 0)
			capt_wait_ready();
		capt_send(CAPT_PRINT_DATA, band_ptr, band_size);
		band_ptr += send;
		band_size -= send;
	}

	free(band_buf);
}
