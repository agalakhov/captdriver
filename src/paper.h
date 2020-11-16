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

struct cups_page_header2_s;

struct page_dims_s {
	/* set by CUPS */
	unsigned media_type;
	unsigned paper_width_pts;
	unsigned paper_height_pts;
	unsigned paper_width;
	unsigned paper_height;
	unsigned toner_save;
	unsigned h_dpi;
	unsigned w_dpi;
	bool pause;
	/* set by printer ops */
	unsigned line_size;
	unsigned band_size;
	unsigned num_lines;
	uint16_t bound_a;
	uint16_t bound_b;
	uint8_t capt_size_id;
};

void page_set_dims(struct page_dims_s *dims, const struct cups_page_header2_s *header);
