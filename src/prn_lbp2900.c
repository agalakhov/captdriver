/*
 * Copyright (C) 2013 Alexey Galakhov <agalakhov@gmail.com>
 * Copyright (C) 2016 Alexei Gordeev <KP1533TM2@gmail.com>
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
#include "word.h"
#include "capt-command.h"
#include "capt-status.h"
#include "generic-ops.h"
#include "hiscoa-common.h"
#include "hiscoa-compress.h"
#include "paper.h"
#include "printer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

uint16_t job;

struct lbp2900_ops_s {
	struct printer_ops_s ops;

	const struct capt_status_s * (*get_status) (void);
	void (*wait_ready) (void);
};

static const struct capt_status_s *lbp2900_get_status(const struct printer_ops_s *ops)
{
	const struct lbp2900_ops_s *lops = container_of(ops, struct lbp2900_ops_s, ops);
	return lops->get_status();
}

static void lbp2900_wait_ready(const struct printer_ops_s *ops)
{
	const struct lbp2900_ops_s *lops = container_of(ops, struct lbp2900_ops_s, ops);
	lops->wait_ready();
}

static void send_job_start(uint8_t fg, uint16_t page)
{
	/* FIXME: this function could use a better name */

	uint8_t ml = 0x00; /* host name lenght */
	uint8_t ul = 0x00; /* user name lenght */
	uint8_t nl = 0x00; /* document name lenght */
	time_t rawtime = time(NULL);
	const struct tm *tm = localtime(&rawtime);
	uint8_t buf[32 + 40 + ml + ul + nl];
	uint8_t head[32] = {
		0x00, 0x00, 0x00, 0x00, LO(page), HI(page), 0x00, 0x00,
		ml, 0x00, ul, 0x00, nl, 0x00, 0x00, 0x00,
		fg, 0x01, LO(job), HI(job),
		/*-60 */ 0xC4, 0xFF,
		/*-120*/ 0x88, 0xFF,
		LO(tm->tm_year), HI(tm->tm_year), (uint8_t) tm->tm_mon, (uint8_t) tm->tm_mday,
		(uint8_t) tm->tm_hour, (uint8_t) tm->tm_min, (uint8_t) tm->tm_sec,
		0x01,
	};
	memcpy(buf, head, sizeof(head));
	memset(buf + 32, 0, 40 + ml + ul + nl);
	capt_sendrecv(CAPT_JOB_SETUP, buf, sizeof(buf), NULL, 0);
}

static const uint8_t magicbuf_0[] = {
	0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t magicbuf_2[] = {
	0xEE, 0xDB, 0xEA, 0xAD, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t lbp2900_gpio_blink[] = {
	0x00, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x00,
};

static const uint8_t lbp2900_gpio_init[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

static const uint8_t lbp3010_gpio_blink[] = {
        /* led */ 0x31, 0x00, 0x00, /* S6 */ 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* S7 */ 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

static const uint8_t lbp3010_gpio_init[] = {
        /* led */ 0x13, 0x00, 0x00, /* S6 */ 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* S7 */ 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};


static void lbp2900_job_prologue(struct printer_state_s *state)
{
	(void) state;
	uint8_t buf[8];
	size_t size;

	capt_sendrecv(CAPT_IDENT, NULL, 0, NULL, 0);
	sleep(1);
	capt_init_status();
	lbp2900_get_status(state->ops);

	capt_sendrecv(CAPT_START_0, NULL, 0, NULL, 0);
	capt_sendrecv(CAPT_GPIO, lbp2900_gpio_init, ARRAY_SIZE(lbp2900_gpio_init), NULL, 0);
	capt_sendrecv(CAPT_JOB_BEGIN, magicbuf_0, ARRAY_SIZE(magicbuf_0), buf, &size);
	job=WORD(buf[2], buf[3]);
	lbp2900_wait_ready(state->ops);
	send_job_start(1, 0);
	lbp2900_wait_ready(state->ops);
}

static void lbp3000_job_prologue(struct printer_state_s *state)
{
	(void) state;
	uint8_t buf[8];
	size_t size;

	capt_sendrecv(CAPT_IDENT, NULL, 0, NULL, 0);
	sleep(1);
	capt_init_status();
	lbp2900_get_status(state->ops);

	capt_sendrecv(CAPT_START_0, NULL, 0, NULL, 0);
	capt_sendrecv(CAPT_GPIO, lbp2900_gpio_init, ARRAY_SIZE(lbp2900_gpio_init), NULL, 0);
	capt_sendrecv(CAPT_JOB_BEGIN, magicbuf_0, ARRAY_SIZE(magicbuf_0), buf, &size); 
	job=WORD(buf[2], buf[3]); 

	/* LBP-3000 prints the very first printjob perfectly
	 * and then proceeds to hang at this (commented out)
	 * spot. That's the difference, or so it seems. */
/*	lbp2900_wait_ready(state->ops); */
	send_job_start(1, 0);
	
	/* There's also that command, that apparently does something, and does something, 
	 * but it's there in the Wireshark logs. Response data == command data. */
	uint8_t dummy[2] = {0, 0};
	capt_sendrecv(0xE0A6, dummy, sizeof(dummy), NULL, 0);

	lbp2900_wait_ready(state->ops);
}

static void lbp3010_job_prologue(struct printer_state_s *state)
{
	(void) state;
	uint8_t buf[8];
	size_t size;

	capt_sendrecv(CAPT_IDENT, NULL, 0, NULL, 0);
	sleep(1);
	capt_init_status();
	lbp2900_get_status(state->ops);

	capt_sendrecv(CAPT_START_0, NULL, 0, NULL, 0);
	capt_sendrecv(CAPT_JOB_BEGIN, magicbuf_0, ARRAY_SIZE(magicbuf_0), buf, &size);
	job=WORD(buf[2], buf[3]);

	capt_sendrecv(CAPT_GPIO, lbp3010_gpio_init, ARRAY_SIZE(lbp3010_gpio_init), NULL, 0);
	lbp2900_wait_ready(state->ops);

	send_job_start(1, 0);
	lbp2900_wait_ready(state->ops);
}

static bool lbp2900_page_prologue(struct printer_state_s *state, const struct page_dims_s *dims)
{
	const struct capt_status_s *status;
	size_t s;
	uint8_t buf[16];
	uint8_t sz = dims->capt_size_id;
	uint8_t save = dims->toner_save;
	uint8_t fm = 0x00; /* fuser mode (temperature?) */
	uint8_t air = 0x02; /* automatic image refinement */

	switch (dims->media_type) {
		case 0x00:
		case 0x02:
			/* Plain Paper & Plain Paper L */
			fm = 0x01;
			break;
		case 0x01:
			/* Heavy Paper */
			fm = 0x01;
			break;
		case 0x03:
			/* Heavy Paper H */
			fm = 0x02;
			break;
		case 0x04:
			/* Transparency */
			fm = 0x13;
			break;
		case 0x05:
			/* Envelope */
			fm = 0x1C;
			break;
		default:
			fm = 0x01;
	}

	fprintf(stderr, "DEBUG: CAPT: media_type=%u, fm=%u\n", dims->media_type, fm);

	uint8_t pageparms[] = {
		/* Bytes 0-21 (0x00 to 0x15) */
		0x00, 0x00, 0x30, 0x2A, sz, 0x00, 0x00, 0x00,
		0x1F, 0x1F, 0x1F, 0x1F, dims->media_type, /* adapt */ 0x11, 0x04, 0x00,
		0x01, 0x01, air, save, 0x00, 0x00,
		/* Bytes 22-33 (0x16 to 0x21) */
		LO(dims->bound_a), HI(dims->bound_a),/* set print area */
		LO(dims->bound_b), HI(dims->bound_b),
		LO(dims->line_size), HI(dims->line_size),
		LO(dims->num_lines), HI(dims->num_lines),
		LO(dims->paper_width), HI(dims->paper_width),
		LO(dims->paper_height), HI(dims->paper_height),
		/* Bytes 34-39 (0x22 to 0x27) */
		0x00, 0x00, fm, 0x00, 0x00, 0x00,
	};

	(void) state;

	status = lbp2900_get_status(state->ops);
	if (FLAG(status, CAPT_FL_UNINIT1) || FLAG(status, CAPT_FL_UNINIT2)) {
		capt_sendrecv(CAPT_START_1, NULL, 0, NULL, 0);
		capt_sendrecv(CAPT_START_2, NULL, 0, NULL, 0);
		capt_sendrecv(CAPT_START_3, NULL, 0, NULL, 0);
		lbp2900_get_status(state->ops); 
		lbp2900_wait_ready(state->ops);
		capt_sendrecv(CAPT_UPLOAD_2, magicbuf_2, ARRAY_SIZE(magicbuf_2), NULL, 0);
		lbp2900_wait_ready(state->ops);
	}

	while (1) {
		if (! FLAG(lbp2900_get_status(state->ops), CAPT_FL_BUFFERFULL))
			break;
		sleep(1);
	}

	capt_multi_begin(CAPT_SET_PARMS);
	capt_multi_add(CAPT_SET_PARM_PAGE, pageparms, sizeof(pageparms));
	s = hiscoa_format_params(buf, sizeof(buf), &hiscoa_default_params);
	capt_multi_add(CAPT_SET_PARM_HISCOA, buf, s);
	capt_multi_add(CAPT_SET_PARM_1, NULL, 0);
	capt_multi_add(CAPT_SET_PARM_2, NULL, 0);
	capt_multi_send();

	return true;
}

static bool lbp3010_page_prologue(struct printer_state_s *state, const struct page_dims_s *dims)
{
	const struct capt_status_s *status;
	size_t s;
	uint8_t buf[16];

	uint8_t mt = 0x00; /* media type, 0x00: Plain Paper */
	uint8_t save = dims->toner_save;
	uint8_t fm = 0x01; /* fuser mode (temperature?) */
	uint8_t air = 0x00; /* image refinement mode */

	/* FIXME: pageparms only support A4 plain paper printing */
	uint8_t pageparms[] = {
		/* Bytes 0-21 (0x00 to 0x15) */
		0x00, 0x00, 0x30, 0x2A, /* sz */ 0x02, 0x00, 0x00, 0x00,
		0x1C, 0x1C, 0x1C, 0x1C, mt, /* adapt */ 0x11, 0x04, 0x00,
		0x01, 0x01, /* img ref */ air, save, 0x00, 0x00,
		/* Bytes 22-33 (0x16 to 0x21) */
		/*Margin height? 118*/ 0x76, 0x00,
		/*Margin width? 78*/ 0x4e, 0x00,
		LO(dims->line_size), HI(dims->line_size),
		LO(dims->num_lines), HI(dims->num_lines),
		LO(dims->paper_width), HI(dims->paper_width),
		LO(dims->paper_height), HI(dims->paper_height),
		/* Bytes 34-39 (0x22 to 0x27) */
		0x00, 0x00, fm, 0x00, 0x00, 0x00,
	};

	(void) state;

	status = lbp2900_get_status(state->ops);
	if (FLAG(status, CAPT_FL_UNINIT1) || FLAG(status, CAPT_FL_UNINIT2)) {
		capt_sendrecv(CAPT_START_1, NULL, 0, NULL, 0);
		capt_sendrecv(CAPT_START_2, NULL, 0, NULL, 0);
		capt_sendrecv(CAPT_START_3, NULL, 0, NULL, 0);

		/* FIXME: wait for printer is free (could it be potentially dangerous or really mandatory?) */
		while ( ! FLAG(lbp2900_get_status(state->ops), ((4 << 16) | (1 << 0)) ) )
		  sleep(1);
		lbp2900_get_status(state->ops);


		lbp2900_wait_ready(state->ops);
		capt_sendrecv(CAPT_UPLOAD_2, magicbuf_2, ARRAY_SIZE(magicbuf_2), NULL, 0);
		lbp2900_wait_ready(state->ops);
	}

	while (1) {
		if (! FLAG(lbp2900_get_status(state->ops), CAPT_FL_BUFFERFULL))
			break;
		sleep(1);
	}

	capt_multi_begin(CAPT_SET_PARMS);
	capt_multi_add(CAPT_SET_PARM_PAGE, pageparms, sizeof(pageparms));
	s = hiscoa_format_params(buf, sizeof(buf), &hiscoa_default_params);
	capt_multi_add(CAPT_SET_PARM_HISCOA, buf, s);
	capt_multi_add(CAPT_SET_PARM_1, NULL, 0);
	capt_multi_add(CAPT_SET_PARM_2, NULL, 0);
	capt_multi_send();

	return true;
}

static bool lbp2900_page_epilogue(struct printer_state_s *state, const struct page_dims_s *dims)
{
	(void) dims;
	const struct capt_status_s *status;

	capt_send(CAPT_PRINT_DATA_END, NULL, 0);

	/* waiting until the page is received */
	while (1) {
	  sleep(1);
	  status = lbp2900_get_status(state->ops);
	  if (status->page_received == status->page_decoding)
	    break;
	}
	send_job_start(2, status->page_decoding);
	lbp2900_wait_ready(state->ops);

	uint8_t buf[2] = { LO(status->page_decoding), HI(status->page_decoding) };
	capt_sendrecv(CAPT_FIRE, buf, 2, NULL, 0);
	lbp2900_wait_ready(state->ops);

	send_job_start(6, status->page_decoding);

	while (1) {
		const struct capt_status_s *status = lbp2900_get_status(state->ops);
		/* Interesting. Using page_printing here results in shifted print */
		if (status->page_out == status->page_decoding)
			return true;
		if (FLAG(status, CAPT_FL_NOPAPER2) || FLAG(status, CAPT_FL_NOPAPER1)) {
			fprintf(stderr, "DEBUG: CAPT: no paper\n");
			if (FLAG(status, CAPT_FL_PRINTING) || FLAG(status, CAPT_FL_PROCESSING1))
				continue;
			return false;
		}
		sleep(1);
	}
}

static void lbp2900_job_epilogue(struct printer_state_s *state)
{
	uint8_t jbuf[2] = { LO(job), HI(job) };

	while (1) {
		const struct capt_status_s *status = lbp2900_get_status(state->ops);
		if (status->page_completed == status->page_decoding) {
			send_job_start(4, status->page_completed);
			break;
		}
		sleep(1);
	}
	capt_sendrecv(CAPT_JOB_END, jbuf, 2, NULL, 0);
}

static void lbp2900_page_setup(struct printer_state_s *state,
		struct page_dims_s *dims,
		unsigned width, unsigned height)
{ 
	(void) state;
	(void) width;
	(void) height;

	/* Override dims for standard sizes to maintain consistency
       with the original drivers
    */
	if ((dims->paper_width_pts == 595) && (dims->paper_height_pts == 842)) {
		/* A4 */
		dims->capt_size_id = 0x02; 
		dims->paper_width = 4960;
		dims->paper_height = 7014;
	}
	else if ((dims->paper_width_pts == 420) && (dims->paper_height_pts == 595)) {
		/* A5 */
		dims->capt_size_id = 0x03; 
		dims->paper_width = 3496;
		dims->paper_height = 4960;
	}
	else if ((dims->paper_width_pts == 516) && (dims->paper_height_pts == 729)) {
		/* B5 */
		dims->capt_size_id = 0x07;
		dims->paper_width = 4298;
		dims->paper_height = 6070;
	}
	else if ((dims->paper_width_pts == 522) && (dims->paper_height_pts == 756)) {
		/* Executive */
		dims->capt_size_id = 0x0A;
		dims->paper_width = 4350;
		dims->paper_height = 6300;
	}
	else if ((dims->paper_width_pts == 612) && (dims->paper_height_pts == 1008)) {
		/* Legal */
		dims->capt_size_id = 0x0C;
		dims->paper_width = 5100;
		dims->paper_height = 8400;
	}
	else if ((dims->paper_width_pts == 612) && (dims->paper_height_pts == 792)) {
		/* Letter */
		dims->capt_size_id = 0x0D;
		dims->paper_width = 5100;
		dims->paper_height = 6600;
	}
	else if ((dims->paper_width_pts == 459) && (dims->paper_height_pts == 649)) {
		/* C5 Envelope */
		dims->capt_size_id = 0x15;
		dims->paper_width = 3826;
		dims->paper_height = 5408;
	}
	else if ((dims->paper_width_pts == 297) && (dims->paper_height_pts == 684)) {
		/* COM10 Envelope */
		dims->capt_size_id = 0x16;
		dims->paper_width = 2478;
		dims->paper_height = 5700;
	}
	else if ((dims->paper_width_pts == 279) && (dims->paper_height_pts == 540)) {
		/* Monarch Envelope */
		dims->capt_size_id = 0x17;
		dims->paper_width = 2328;
		dims->paper_height = 4500;
	}
	else if ((dims->paper_width_pts == 312) && (dims->paper_height_pts == 624)) {
		/* DL Envelope */
		dims->capt_size_id = 0x18;
		dims->paper_width = 2598;
		dims->paper_height = 5196;
	}
	else if ((dims->paper_width_pts == 216) && (dims->paper_height_pts == 360)) {
		/* Index Card 3x5" */
		dims->capt_size_id = 0x40;
		dims->paper_width = 1800;
		dims->paper_height = 3000;
	}
	else {
		/* custom sizes */
		dims->capt_size_id = 0x13;
		dims->paper_height = dims->paper_height_pts * dims->h_dpi / 72;
		dims->paper_width = dims->paper_width_pts * dims->w_dpi / 72;
	}

	switch(dims->capt_size_id) {
		case 0x02:
		case 0x03:
		case 0x07:
		case 0x0A:
		case 0x0C:
		case 0x0D:
			/* standard page size bounds */
			dims->bound_a = 0x0078;
			dims->bound_b = 0x0060;
			dims->num_lines = dims->paper_height - (dims->bound_a*2) + 2;
			break;
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x40:
			/* standard envelope size bounds */
			dims->bound_a = 0x00EC;
			dims->bound_b = 0x00EC;
			dims->num_lines = dims->paper_height - (dims->bound_a*2);
			break;
		default:
			/* custom page size bounds */
			dims->bound_a = 0x0076;
			dims->bound_b = 0x005E;
			dims->num_lines = dims->paper_height - (dims->bound_a*2);
			break;
	}

	dims->line_size = (dims->paper_width - (dims->bound_a*2) ) / 8;
	while(dims->line_size & 0x02) /* next int divisible by 4 */
		dims->line_size += 1;

	dims->band_size = dims->num_lines/8;
	fprintf(stderr, "DEBUG: CAPT: b_a=%u, b_b=%u\n",dims->bound_a,dims->bound_b);
}

static void lbp2900_cancel_cleanup(struct printer_state_s *state)
{
	(void) state;
	const struct capt_status_s *status = lbp2900_get_status(state->ops);
	uint8_t jbuf[2] = { LO(job), HI(job) };

	capt_cleanup();
	capt_sendrecv(CAPT_GPIO, lbp2900_gpio_init, ARRAY_SIZE(lbp2900_gpio_init), NULL, 0);
	send_job_start(4, status->page_completed);
	capt_sendrecv(CAPT_JOB_END, jbuf, 2, NULL, 0);
}

static void lbp3010_cancel_cleanup(struct printer_state_s *state)
{
	(void) state;
	capt_sendrecv(CAPT_GPIO, lbp3010_gpio_init, ARRAY_SIZE(lbp3010_gpio_init), NULL, 0);
}

static void lbp2900_wait_user(struct printer_state_s *state)
{
	(void) state;

	capt_sendrecv(CAPT_GPIO, lbp2900_gpio_blink, ARRAY_SIZE(lbp2900_gpio_blink), NULL, 0);
	lbp2900_wait_ready(state->ops);

	while (1) {
		const struct capt_status_s *status = lbp2900_get_status(state->ops);
		if (FLAG(status, CAPT_FL_BUTTON) || job_cancel) {
			fprintf(stderr, "DEBUG: CAPT: button pressed\n");
			break;
		}
		sleep(1);
	}

	capt_sendrecv(CAPT_GPIO, lbp2900_gpio_init, ARRAY_SIZE(lbp2900_gpio_init), NULL, 0);
	lbp2900_wait_ready(state->ops);
}

static void lbp3010_wait_user(struct printer_state_s *state)
{
	(void) state;

	capt_sendrecv(CAPT_GPIO, lbp3010_gpio_blink, ARRAY_SIZE(lbp3010_gpio_blink), NULL, 0);
	lbp2900_wait_ready(state->ops);

	while (1) {
		const struct capt_status_s *status = lbp2900_get_status(state->ops);
		if (FLAG(status, CAPT_FL_BUTTON)) {
			fprintf(stderr, "DEBUG: CAPT: button pressed\n");
//			break;
		}
		if (FLAG(status, CAPT_FL_nERROR)) {
			fprintf(stderr, "DEBUG: CAPT: virtual button pressed\n");
			break;
		}
		sleep(1);
	}

	capt_sendrecv(CAPT_GPIO, lbp3010_gpio_init, ARRAY_SIZE(lbp3010_gpio_init), NULL, 0);
	lbp2900_wait_ready(state->ops);
}

static struct lbp2900_ops_s lbp2900_ops = {
	.ops = {
		.job_prologue = lbp2900_job_prologue,
		.job_epilogue = lbp2900_job_epilogue,
		.page_setup = lbp2900_page_setup,
		.page_prologue = lbp2900_page_prologue,
		.page_epilogue = lbp2900_page_epilogue,
		.compress_band = ops_compress_band_hiscoa,
		.send_band = ops_send_band_hiscoa,
		.cancel_cleanup = lbp2900_cancel_cleanup,
		.wait_user = lbp2900_wait_user,
	},
	.get_status = capt_get_xstatus,
	.wait_ready = capt_wait_ready,
};

register_printer("LBP2900", lbp2900_ops.ops, WORKS);

static struct lbp2900_ops_s lbp3000_ops = {
	.ops = {
		.job_prologue = lbp3000_job_prologue,	/* different job prologue */
		.job_epilogue = lbp2900_job_epilogue,
		.page_setup = lbp2900_page_setup,
		.page_prologue = lbp2900_page_prologue,
		.page_epilogue = lbp2900_page_epilogue,
		.compress_band = ops_compress_band_hiscoa,
		.send_band = ops_send_band_hiscoa,
		.cancel_cleanup = lbp2900_cancel_cleanup,
		.wait_user = lbp2900_wait_user,
	},
	.get_status = capt_get_xstatus,
	.wait_ready = capt_wait_ready,
};

register_printer("LBP3000", lbp3000_ops.ops, EXPERIMENTAL);

static struct lbp2900_ops_s lbp3010_ops = {
	.ops = {
		.job_prologue = lbp3010_job_prologue,
		.job_epilogue = lbp2900_job_epilogue,
		.page_setup = lbp2900_page_setup,
		.page_prologue = lbp3010_page_prologue,
		.page_epilogue = lbp2900_page_epilogue,
		.compress_band = ops_compress_band_hiscoa,
		.send_band = ops_send_band_hiscoa,
		.cancel_cleanup = lbp3010_cancel_cleanup,
		.wait_user = lbp3010_wait_user,
	},
	.get_status = capt_get_xstatus_only,
	.wait_ready = capt_wait_xready_only,
};

register_printer("LBP3010/LBP3018/LBP3050", lbp3010_ops.ops, EXPERIMENTAL);

