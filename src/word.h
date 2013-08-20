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

static inline uint8_t HI(uint16_t x)
{
	return (x >> 8) & 0xFF;
}

static inline uint8_t LO(uint16_t x)
{
	return (x >> 0) & 0xFF;
}

static inline uint16_t WORD(uint8_t lo, uint8_t hi)
{
	return ((hi & 0xFF) << 8) | ((lo & 0xFF) << 0);
}

static inline uint16_t BCD(uint8_t lo, uint8_t hi)
{
	uint16_t a, b, c, d;
	a = (hi >> 8) & 0x0F;
	b = (hi >> 0) & 0x0F;
	c = (lo >> 8) & 0x0F;
	d = (lo >> 0) & 0x0F;
	if (a > 9 || b > 9 || c > 9 || d > 9)
		return WORD(lo, hi);
	return a * 1000 + b * 100 + c * 10 + d * 1;
}
