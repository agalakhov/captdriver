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

enum hiscoa_eob_type {
	HISCOA_EOB_NORMAL = 0x0,
	HISCOA_EOB_LAST = 0x01,
};

struct hiscoa_params {
	int origin_3;
	int origin_5;
	int origin_0;
	int origin_2;
	int origin_4;
};

extern const struct hiscoa_params hiscoa_default_params;
