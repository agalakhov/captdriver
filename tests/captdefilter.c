#include "word.h"
#include "hiscoa-decompress.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct page {
	size_t reserve;
	size_t size;
	uint8_t *data;
};

static struct page page;

static bool hi_mode = false;
static unsigned line_size;
static struct hiscoa_params hiscoa_params;

static uint8_t *page_reserve(size_t size)
{
	size_t reserve = page.reserve;
	while (page.size + size > reserve)
		reserve *= 2;
	if (reserve != page.reserve) {
		page.reserve = reserve;
		page.data = realloc(page.data, page.reserve);
		if (! page.data)
			abort();
	}
	return page.data + page.size;
}

static void page_add(const uint8_t *buf, size_t size)
{
	memcpy(page_reserve(size), buf, size);
	page.size += size;
}

static void page_output(void)
{
	unsigned w = line_size * 8;
	unsigned h = page.size / line_size;
	printf("P4\n#\n%u %u\n", w, h);
	fwrite(page.data, line_size, h, stdout);
	fprintf(stderr, "output page %ux%u px\n", w, h);
	page.size = 0;
}

static void dump(const uint8_t *buf, size_t size)
{
	size_t i;
	for (i = 1; i <= size; ++i) {
		fprintf(stderr, " %02x", buf[i - 1]);
		if (i % 16 == 0 || i == size)
			fprintf(stderr, "\n");
	}
}

static void decode_hiscoa_params(const uint8_t *buf, size_t size)
{
	(void) size;

	hiscoa_params.origin_3 = (int8_t)buf[0];
	hiscoa_params.origin_5 = (int8_t)buf[1];
	/* buf[2] - ??? */
	/* buf[3] - ??? */
	hiscoa_params.origin_0 = (int8_t) buf[4]; /* ??? */
	hiscoa_params.origin_2 = (int8_t) buf[5];
	hiscoa_params.origin_4 = (int16_t) WORD(buf[6], buf[7]);
}

static void decode_hiscoa_band_data(const uint8_t *buf, size_t size, unsigned lines)
{
	const uint8_t *src;
	size_t srcsize;
	uint8_t *dest;
	size_t destsize = 0;
	unsigned type;
	src = buf;
	srcsize = size;
	hiscoa_decompress_band((const void **)&src, &srcsize,
			NULL, &destsize, line_size, &hiscoa_params);
	dest = page_reserve(destsize);
	src = buf;
	srcsize = size;
	type = hiscoa_decompress_band((const void **)&src, &srcsize,
			dest, &destsize, line_size, &hiscoa_params);
	page.size += destsize;
	fprintf(stderr, "  eob = %u, lines = %u, expected bytes = %u\n",
		type, lines, lines * line_size);
	fprintf(stderr, "  unconsumed size = %u, decompressed size = %u\n",
		(unsigned) srcsize, (unsigned) destsize);
}

static void decode_hiscoa_band(const uint8_t *buf, size_t size)
{
	unsigned lines = WORD(buf[2], buf[3]);
	decode_hiscoa_band_data(buf + 4, size - 4, lines);
}

static void decode_hiscoa_band2(const uint8_t *buf, size_t size)
{
	unsigned lines = WORD(buf[2], buf[3]);
	decode_hiscoa_band_data(buf + 6, size - 6, lines);
}

static void decode_hiscoa_band3(const uint8_t *buf, size_t size)
{
	unsigned lines = WORD(buf[2], buf[3]);
	decode_hiscoa_band_data(buf + 8, size - 8, lines);
}

static void dispatch(uint16_t cmd, const uint8_t *buf, size_t size)
{
	switch (cmd) {
	case 0xD0A9:
		fprintf(stderr, "  --(multi-command)--\n");
		while (size) {
			uint16_t cc = WORD(buf[0], buf[1]);
			unsigned cs = WORD(buf[2], buf[3]);
			dispatch(cc, buf + 4, cs - 4);
			buf += cs;
			size -= cs;
		}
		break;
	case 0xD0A0:
		fprintf(stderr, "  -(compression parameters)-\n");
		dump(buf, size);
		line_size = WORD(buf[26], buf[27]);
		fprintf(stderr, "  decoded: L=%u bytes, %u pixels\n", line_size, line_size * 8);
		break;
	case 0xD0A4:
		fprintf(stderr, "  -(Hi-SCoA parameters)-\n");
		dump(buf, size);
		decode_hiscoa_params(buf, size);
		hi_mode = true;
		fprintf(stderr, "  decoded: [0]=%i+L [2]=%i+L [4]=%i [3]=%i [5]=%i\n",
			hiscoa_params.origin_0, hiscoa_params.origin_2,
			hiscoa_params.origin_4,
			hiscoa_params.origin_3, hiscoa_params.origin_5);
		break;
	case 0xC0A0:
		if (! hi_mode) {
			fprintf(stderr, "  -(SCoA data)-\n");
			dump(buf, 16);
		} else {
			fprintf(stderr, "  -(Hi-SCoA data from printer debug)-\n");
			dump(buf, 16);
			decode_hiscoa_band_data(buf, size, 70);
		}
		break;
	case 0x8000:
		fprintf(stderr, "  -(Hi-SCoA data)-\n");
		dump(buf, 4);
		decode_hiscoa_band2(buf, size);
		break;
	case 0x8200:
		fprintf(stderr, "  -(Hi-SCoA data)-\n");
		dump(buf, 4);
		decode_hiscoa_band3(buf, size);
		break;
	case 0xC0A4:
		fprintf(stderr, "  --end--\n");
		page_output();
		break;
	default:
		dump(buf, size);
		break;
	}
}

int main(int argc, char **argv)
{
	static uint8_t buf[1<<20];

	FILE *input = stdin;

	if (argc > 1) {
		input = fopen(argv[1], "r");
		if (! input)
			abort();
	}

	page.reserve = 1024;
	page.size = 0;
	page.data = malloc(page.reserve);
	if (! page.data)
		abort();

	while (1) {
		int s;
		uint16_t cmd;
		uint32_t len;
		size_t pos;
		s = fread(buf, 1, 4, input);
		if (s == 0)
			break;
		if (s != 4) {
			fprintf(stderr, "! unable to read header\n");
			break;
		}
		pos = 4;
		cmd = WORD(buf[0], buf[1]);
		switch (cmd) {
		case 0x8000:
			fread(buf + pos, 1, 2, input);
			pos += 2;
			len = WORD(buf[4], buf[5]);
			len <<= 16;
			len += WORD(buf[2], buf[3]);
			break;
		case 0x8200:
			fread(buf + pos, 1, 4, input);
			pos += 4;
			len = WORD(buf[6], buf[7]);
			len <<= 16;
			len += WORD(buf[4], buf[5]);
			break;
		default:
			len = WORD(buf[2], buf[3]);
			break;
		}
		fprintf(stderr, "CMD %04X len=%u\n", cmd, len);
		if (fread(buf + pos, 1, len - pos, input) != len - pos) {
			fprintf(stderr, "! unable to read %li bytes\n", len - pos);
			break;
		}
		dispatch(cmd, buf + pos, len - pos);
	}

	if (page.size)
		page_output();

	if (argc > 1)
		fclose(input);
	return 0;
}
