#include "hiscoa-decompress.h"

#include "std.h"
#include "hiscoa-common.h"

struct state {
	const uint8_t xorval;
	const uint8_t *const input_buf;
	const size_t input_size;
	unsigned bitpos;

	uint8_t *const output_buf;
	const size_t output_size;
	size_t output_pos;

	unsigned prefix;

	uint8_t bytes[16];

	unsigned origin_3;
	unsigned origin_5;
	unsigned origin_0;
	unsigned origin_2;
	unsigned origin_4;
};

static bool read_bit(struct state *state)
{
	bool ret;
	if (state->bitpos / 8 >= state->input_size)
		return true;
	ret = !!((state->input_buf[state->bitpos / 8] ^ state->xorval) &
		(0x80 >> (state->bitpos % 8)));
	state->bitpos += 1;
	return ret;
}

static unsigned read_n10(struct state *state, unsigned limit)
{
	unsigned ret = 0;
	while (ret < limit && read_bit(state))
		++ret;
	return ret;
}

static unsigned read_number(struct state *state, unsigned bits)
{
	unsigned i;
	unsigned ret = 0;
	for (i = 0; i < bits; ++i)
		ret = (ret << 1) | (read_bit(state) ? 1 : 0);
	return ret;
}

static unsigned read_long_number(struct state *state)
{
	unsigned order = read_n10(state, 6);
	if (order == 6)
		return 0;
	if (order == 0) {
		if (! read_bit(state))
			return 1;
		return read_bit(state) ? 2 : 3;
	}
	return (1 << (order + 2)) - 1 - read_number(state, order + 1);
}

static unsigned read_prefix(struct state *state)
{
	unsigned nbits = read_number(state, 2);
	unsigned val = read_number(state, nbits);
	return 128 * ((1 << (nbits + 1)) - 1 - val);
}

static uint8_t save_byte(struct state *state, uint8_t byte)
{
	unsigned i;
	for (i = 15; i > 0; --i)
		state->bytes[i] = state->bytes[i - 1];
	state->bytes[0] = byte;
	return byte;
}

static uint8_t get_byte(struct state *state, unsigned pos)
{
	unsigned i;
	uint8_t ret = state->bytes[pos];
	for (i = pos; i > 0; --i)
		state->bytes[i] = state->bytes[i - 1];
	state->bytes[0] = ret;
	return ret;
}

static void swap(unsigned *a, unsigned *b)
{
	unsigned t = *a;
	*a = *b;
	*b = t;
}

static inline void output_byte(struct state *state, uint8_t byte)
{
	if (state->output_buf && state->output_pos < state->output_size)
		state->output_buf[state->output_pos] = byte;
	state->output_pos += 1;
}

static void copy_block(struct state *state, unsigned origin)
{
	unsigned len = state->prefix + read_long_number(state);
	unsigned i;
	for (i = 0; i < len; ++i) {
		uint8_t byte = (state->output_buf
			&& state->output_pos >= origin
			&& state->output_pos - origin < state->output_size)
			? state->output_buf[state->output_pos - origin] : 0xCC;
		output_byte(state, byte);
	}
}

unsigned hiscoa_decompress_band(
	const void **band, size_t *size,
	void *output, size_t *output_size,
	unsigned line_size,
	const struct hiscoa_params *params
)
{
	struct state state = {
		.xorval = 0x43,
		.input_buf = *(uint8_t **) band,
		.input_size = size ? *size : (size_t) -1,
		.bitpos = 0,
		.output_buf = output,
		.output_size = output_size ? *output_size : (size_t) -1,
		.output_pos = 0,
		.prefix = 0,
		.bytes = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xFF,
			   0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xFF },

		.origin_3 = params->origin_3,
		.origin_5 = params->origin_5,
		.origin_0 = params->origin_0 + line_size,
		.origin_2 = params->origin_2 + line_size,
		.origin_4 = params->origin_4,
	};

	unsigned ret = (unsigned) -1;
	bool end = false;
	while (! end && state.bitpos < 8 * state.input_size) {
		bool no_reset_prefix = false;
		unsigned group = read_n10(&state, 8);
		switch (group) {
		case 0:
			copy_block(&state, state.origin_0);
			break;
		case 1:
			output_byte(&state, get_byte(&state, 15 - read_number(&state, 4)));
			break;
		case 2:
			if (read_bit(&state)) {
				output_byte(&state, save_byte(&state, read_number(&state, 8)));
			} else {
				copy_block(&state, state.origin_2);
				swap(&state.origin_2, &state.origin_0);
			}
			break;
		case 3:
			copy_block(&state, state.origin_3);
			break;
		case 4:
			copy_block(&state, state.origin_4);
			break;
		case 5:
			copy_block(&state, state.origin_5);
			swap(&state.origin_5, &state.origin_3);
			break;
		case 6:
			if (read_bit(&state)) {
				output_byte(&state, save_byte(&state, 0));
			} else {
				state.prefix = read_prefix(&state);
				no_reset_prefix = true;
			}
			break;
		case 7:
			ret = read_number(&state, 2);
			end = true;
			break;
		case 8:
			break;
		}
		if (! no_reset_prefix)
			state.prefix = 0;
	}

	while (state.bitpos % 32)
		read_bit(&state);

	if (state.bitpos > 8 * state.input_size)
		state.bitpos = 8 * state.input_size;
	*(uint8_t **) band += state.bitpos / 8;
	if (size)
		*size -= state.bitpos / 8;
	if (output_size)
		*output_size = state.output_pos;

	return ret;
}
