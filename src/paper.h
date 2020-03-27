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
	unsigned paper_width;
	unsigned paper_height;
	unsigned margin_height;
	unsigned margin_width;
	/* set by printer ops */
	unsigned line_size;
	unsigned band_size;
	unsigned num_lines;
};

void page_set_dims(struct page_dims_s *dims, const struct cups_page_header2_s *header);
