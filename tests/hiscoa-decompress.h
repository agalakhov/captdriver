#pragma once

#include "../src/std.h"
#include "../src/hiscoa-common.h"

unsigned hiscoa_decompress_band(
	const void **band, size_t *size,
	void *output, size_t *output_size,
	unsigned line_size,
	const struct hiscoa_params *params
);
