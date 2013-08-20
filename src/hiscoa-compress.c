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

#include "hiscoa-compress.h"

#include "std.h"
#include "word.h"
#include "hiscoa-common.h"

#include <string.h>

struct state {
	const uint8_t xorval;

	const uint8_t *const input_buf;
	const size_t input_size;
	size_t input_pos;

	uint8_t *const output_buf;
	const size_t output_bitsize;
	unsigned bitpos;

	unsigned line_size;

	unsigned nbytes;
	uint8_t bytes[16];

	unsigned origin[6];
};

static void push_bits(struct state *state, uint32_t bits, unsigned count)
{
	while (count && state->bitpos < state->output_bitsize) {
		unsigned word = state->bitpos / 8;
		unsigned spc = 8 - (state->bitpos % 8);
		unsigned cnt = (count > spc) ? spc : count;
		uint8_t mask = (0xFFu >> (8 - cnt)) << (spc - cnt);
		uint32_t val = (bits >> (count - cnt)) << (spc - cnt);
		uint8_t x = state->output_buf[word];
		x ^= state->xorval;
		x &= ~mask;
		x |= mask & val;
		x ^= state->xorval;
		state->output_buf[word] = x;
		state->bitpos += cnt;
		count -= cnt;
	}
}

static unsigned try_match(const struct state *state, unsigned diff)
{
	unsigned pos = state->input_pos;
	if (pos < diff)
		return 0;
	while (state->input_buf[pos] == state->input_buf[pos - diff]) {
		++pos;
		if (pos >= state->input_size)
			break;
		if (pos >= state->input_pos + 512 + 127)
			break;
		if (pos % state->line_size == 0)
			break;
	}
	return pos - state->input_pos;
}

static void swap(unsigned *a, unsigned *b)
{
	unsigned t = *a;
	*a = *b;
	*b = t;
}

static unsigned find_msb(unsigned val)
{
	/* FIXME do this faster */
	unsigned nbits;
	if (val == 0)
		return 0;
	for (nbits = 8 * sizeof(val) - 1; val < (1u << nbits); --nbits)
		;
	return nbits + 1;
}

static bool try_write_longrepeat(struct state *state)
{
	unsigned cmd;
	unsigned best_cmd;
	unsigned best_len = 0;

	for (cmd = 0; cmd < 6; ++cmd) {
		if (state->origin[cmd]) {
			unsigned len = try_match(state, state->origin[cmd]);
			if (len > best_len) {
				best_cmd = cmd;
				best_len = len;
			}
		}
	}

	if (best_len > 1) {
		unsigned nbits;
		unsigned len = best_len;

		if (len > 127) {
			unsigned val = len / 128;
			nbits = find_msb(val) - 1;
			push_bits(state, 0xFC, 8);
			push_bits(state, nbits, 2);
			push_bits(state, ~val, nbits);
			len %= 128;
		}

		push_bits(state, 0xFFFFFFFE, best_cmd + 1); /* longrep */
		if (best_cmd == 2)
			push_bits(state, 0x0, 1); /* subcommand 0 */

		switch (len) {
		case 0:
			push_bits(state, 0x3F, 6);
			break;
		case 1:
			push_bits(state, 0x0, 2);
			break;
		case 2:
			push_bits(state, 0x3, 3);
			break;
		case 3:
			push_bits(state, 0x2, 3);
			break;
		default:
			nbits = find_msb(len) - 1;
			push_bits(state, 0xFFFFFFFE, nbits);
			push_bits(state, ~len, nbits);
			break;
		}

		if (best_cmd == 2)
			swap(&state->origin[2], &state->origin[0]);

		if (best_cmd == 5)
			swap(&state->origin[5], &state->origin[3]);

		state->input_pos += best_len;
		return true;
	}

	return false;
}

static bool try_write_byterepeat(struct state *state)
{
	unsigned i;
	for (i = 0; i < state->nbytes; ++i) {
		uint8_t byte = state->input_buf[state->input_pos];
		if (byte == state->bytes[i]) {
			unsigned j;
			for (j = i; j > 0; --j)
				state->bytes[j] = state->bytes[j - 1];
			state->bytes[0] = byte;
			push_bits(state, 0x20 | ((15 - i) & 0xF), 6);
			state->input_pos += 1;
			return true;
		}
	}
	return false;
}

static void write_simple_byte(struct state *state)
{
	unsigned i;
	uint8_t byte = state->input_buf[state->input_pos];

	if (state->nbytes < 16)
		state->nbytes += 1;
	for (i = state->nbytes - 1; i > 0; --i)
		state->bytes[i] = state->bytes[i - 1];
	state->bytes[0] = byte;

	if (byte == 0) {
		push_bits(state, 0xFD, 8); /* zero */
	} else {
		push_bits(state, 0xD00 | byte, 12); /* byte */
	}
	state->input_pos += 1;
}

size_t hiscoa_compress_band(void *buf, size_t size,
	const void *band, unsigned line_size, unsigned nlines,
	enum hiscoa_eob_type eob_type,
	const struct hiscoa_params *params)
{
	struct state state = {
		.xorval = 0x43,

		.input_buf = (const uint8_t *) band,
		.input_size = line_size * nlines,
		.input_pos = 0,

		.output_buf = (uint8_t *) buf,
		.output_bitsize = 8 * size,
		.bitpos = 0,

		.line_size = line_size,

		.nbytes = 0,

		.origin[1] = 0,
		.origin[3] = params->origin_3,
		.origin[5] = params->origin_5,
		.origin[0] = params->origin_0 + line_size,
		.origin[2] = params->origin_2 + line_size,
		.origin[4] = params->origin_4,
	};

	while (state.input_pos < state.input_size) {
		if (try_write_longrepeat(&state))
			continue;
		if (try_write_byterepeat(&state))
			continue;
		write_simple_byte(&state);
	}

	push_bits(&state, 0xFE, 8); /* end */
	push_bits(&state, (unsigned) eob_type, 2);
	//if (state.bitpos % 32)
		push_bits(&state, 0xFFFFFFFF, 32 - (state.bitpos % 32));

	return state.bitpos / 8;
}


size_t hiscoa_format_params(void *buf, size_t size, const struct hiscoa_params *params)
{
	uint8_t b[8] = {
		(uint8_t) (int8_t) params->origin_3,
		(uint8_t) (int8_t) params->origin_5,
		0x01,
		0x01,
		(uint8_t) (int8_t) params->origin_0,
		(uint8_t) (int8_t) params->origin_2,
		LO((uint16_t) (int16_t) params->origin_4),
		HI((uint16_t) (int16_t) params->origin_4),
	};

	if (size < sizeof(b))
		return 0;
	memcpy(buf, b, sizeof(b));
	return sizeof(b);
}
