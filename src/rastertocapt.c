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

#include "std.h"
#include "printer.h"
#include "paper.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <cups/raster.h>


static size_t center_pixels(size_t small, size_t large, unsigned bpp)
{
	unsigned small_p;
	unsigned large_p;
	unsigned bypp;
	while (bpp % 8)
		bpp *= 2;
	bypp = bpp / 8;
	small_p = small / bypp;
	large_p = large / bypp;
	return bypp * ((large_p - small_p) / 2);
}

static void send_page_data(struct printer_state_s *state,
		const struct page_dims_s *dims,
		cups_raster_t *raster,
		const struct cups_page_header2_s *header)
{
	const struct printer_ops_s *ops = state->ops;
	uint8_t *linebuf = NULL;
	uint8_t *bandbuf = NULL;
	unsigned i;

	unsigned shiftb = 0;
	unsigned shiftl = 0;
	unsigned csize = header->cupsBytesPerLine;


	if (header->cupsBytesPerLine < dims->line_size) {
		csize = header->cupsBytesPerLine;
		shiftb = center_pixels(header->cupsBytesPerLine, dims->line_size,
				header->cupsBitsPerPixel);
	}

	if (dims->line_size < header->cupsBytesPerLine) {
		csize = dims->line_size;
		shiftl = center_pixels(dims->line_size, header->cupsBytesPerLine,
				header->cupsBitsPerPixel);
	}

	if (header->cupsBytesPerLine > dims->line_size)
		fprintf(stderr, "INFO: cutting line from %u to %u bytes\n",
				header->cupsBytesPerLine, dims->line_size);

	if (header->cupsHeight > dims->num_lines)
		fprintf(stderr, "INFO: cutting image from %u to %u lines\n",
				header->cupsHeight, dims->num_lines);

	linebuf = calloc(1, header->cupsBytesPerLine);
	if (! linebuf)
		abort();
	bandbuf = calloc(dims->line_size, dims->band_size);
	if (! bandbuf)
		abort();

	state->isend = 0;

	for (state->iband = 0; state->iband * dims->band_size < dims->num_lines; ++state->iband) {
		unsigned start = state->iband * dims->band_size;
		unsigned nlines = dims->band_size;
		unsigned iline;
		memset(bandbuf, 0, dims->line_size * dims->band_size);
		if (nlines + start >= dims->num_lines)
			nlines = dims->num_lines - start;
		for (iline = 0; iline < nlines; ++iline) {
			if (iline + start < header->cupsHeight) {
				if (cupsRasterReadPixels(raster, linebuf, header->cupsBytesPerLine)
								!= header->cupsBytesPerLine) {
					fprintf(stderr, "ERROR: CAPT: broken raster file\n");
					exit(1);
				}
			} else {
				memset(linebuf, 0, header->cupsBytesPerLine);
			}
			memcpy(bandbuf + iline * dims->line_size + shiftb, linebuf + shiftl, csize);
		}
		ops->send_band(state, bandbuf, dims->line_size, nlines);
	}

	/* discard end of image, if any */
	for (i = dims->num_lines; i < header->cupsHeight; ++i)
		cupsRasterReadPixels(raster, linebuf, header->cupsBytesPerLine);

	if (bandbuf)
		free(bandbuf);
	if (linebuf)
		free(linebuf);
}


static void do_print(int fd)
{
	struct cups_page_header2_s header;
	const struct printer_ops_s *ops;
	struct printer_state_s *state = NULL;
	cups_raster_t *raster;

	ops = printer_detect();

	if (ops->alloc_state)
		state = ops->alloc_state();
	else
		state = calloc(1, sizeof(struct printer_state_s));
	state->ops = ops;
	state->ipage = 0;

	raster = cupsRasterOpen(fd, CUPS_RASTER_READ);

	fprintf(stderr, "DEBUG: CAPT: rastertocapt is rendering\n");

	while (cupsRasterReadHeader2(raster, &header)) {
		struct page_dims_s dims;

		page_set_dims(&dims, &header);

		if (! state->ipage) {
			fprintf(stderr, "DEBUG: CAPT: rastertocapt: start job\n");
			if (ops->job_prologue)
				ops->job_prologue(state);
		}

		state->ipage += 1;

		ops->page_setup(state, &dims, header.cupsWidth, header.cupsHeight);

		fprintf(stderr, "DEBUG: CAPT: rastertocapt: start page %u\n", state->ipage);
		if (ops->page_prologue)
			ops->page_prologue(state, &dims);

		fprintf(stderr, "DEBUG: CAPT: rastertocapt: sending page data\n");
		send_page_data(state, &dims, raster, &header);

		fprintf(stderr, "DEBUG: CAPT: rastertocapt: end page %u\n", state->ipage);
		if (ops->page_epilogue)
			ops->page_epilogue(state, &dims);
	}

	if (state->ipage) {
		fprintf(stderr, "DEBUG: CAPT: rastertocapt: end job\n");
		if (ops->job_epilogue)
			ops->job_epilogue(state);
	}

	if (! state->ipage)
		fprintf(stderr, "ERROR: CAPT: no pages in job\n");

	cupsRasterClose(raster);

	if (state) {
		if (ops->free_state)
			ops->free_state(state);
		else
			free(state);
		state = NULL;
	}
}


int main(int argc, char *argv[])
{
	int fd = 0;

	if (argc < 6 || argc > 7) {
		fprintf(stderr, "Usage: %s job-id user title copies options [file]\n", argv[0]);
		return 1;
	}

	if (argc == 7) {
		fd = open(argv[6], O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, "ERROR: unable to open %s: %s\n", argv[6], strerror(errno));
			return 1;
		}
	}

	fprintf(stderr, "DEBUG: CAPT: rastertocapt started\n");
	signal(SIGPIPE, SIG_IGN);
	do_print(fd);
	fprintf(stderr, "DEBUG: CAPT: rastertocapt finished\n");

	if (argc == 7)
		close(fd);

	return 0;
}
