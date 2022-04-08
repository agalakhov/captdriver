#include "../src/std.h"
#include "../src/hiscoa-compress.h"
#include "hiscoa-decompress.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static const unsigned band_size = 70;

static const struct hiscoa_params params = {
	.origin_3 = 1,
	.origin_5 = 4,
	.origin_0 = 0,
	.origin_2 = -12,
	.origin_4 = 340,
};

int main(int argc, char **argv)
{
	FILE *input = stdin;
	unsigned width, height;
	char header[1024];
	unsigned bsize;
	uint8_t *buf = NULL;
	uint8_t *dcbuf = NULL;
	uint8_t *bandbuf = NULL;
	unsigned errors = 0;

	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (! input)
			abort();
	}

	fscanf(input, "%s\n", header);
	if (strcmp(header, "P4"))
		abort();
	fscanf(input, "%s\n", header);
	fscanf(input, "%u %u\n", &width, &height);
	fprintf(stderr, "Input image dimensions: %ux%u\n", width, height);

	bsize = (width / 8) * band_size;
	buf = calloc(1, bsize);
	if (! buf)
		abort();
	dcbuf = calloc(1, bsize);
	if (! dcbuf)
		abort();
	bandbuf = calloc(2, bsize);
	if (! bandbuf)
		abort();

	while (1) {
		bool bad = false;
		ssize_t s = fread(buf, 1, bsize, input);
		if (s < 0)
			abort();
		if (s > 0) {
			uint8_t *tmpb = bandbuf;
			size_t tmps;
			size_t dcsize = bsize;
			unsigned code;
			size_t bl = s / (width / 8);
			size_t bs = hiscoa_compress_band(bandbuf, 2 * bsize,
					buf, width / 8, bl,
					(s < (int)bsize) ? HISCOA_EOB_LAST : HISCOA_EOB_NORMAL,
					&params);
			memset(dcbuf, 0xAA, bsize);
			tmps = bs;
			code = hiscoa_decompress_band((const void **)&tmpb, &tmps,
					dcbuf, &dcsize,
					width / 8,
					&params);
			if (tmps) {
				fprintf(stderr, "! band not fully decompressed: %u bytes left\n",
						(unsigned) tmps);
				bad = true;
			}
			if (memcmp(dcbuf, buf, dcsize)) {
				fprintf(stderr, "! band compressed INCORRECTLY\n");
				bad = true;
			}
			if ((int)dcsize != s) {
				fprintf(stderr, "! band size incorrect: %u instead of %u\n",
						(unsigned) dcsize, (unsigned) s);
				bad = true;
			}
			if (bad)
				errors += 1;
		}
		if (s < (int)bsize)
			break;
	}

	fprintf(stderr, "FINISHED - %u errors\n", errors);

	if (bandbuf)
		free(bandbuf);
	if (dcbuf)
		free(dcbuf);
	if (buf)
		free(buf);

	if (argc > 1)
		fclose(input);
	return errors ? 1 : 0;
}
