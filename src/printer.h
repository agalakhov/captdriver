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

enum printer_support {
	UNSUPPORTED,
	BROKEN,
	EXPERIMENTAL,
	WORKS,
	FULLY,
};

struct page_dims_s;

struct printer_state_s {
	const struct printer_ops_s *ops;
	struct page_queue_s *page_queue;
	unsigned ipage;
	unsigned iband;
	unsigned isend;
};

struct printer_ops_s {
	struct printer_state_s * (*alloc_state) (void);
	void (*free_state) (struct printer_state_s *state);
	void (*job_prologue) (struct printer_state_s *state);
	void (*job_epilogue) (struct printer_state_s *state);
	void (*page_setup) (struct printer_state_s *state,
		struct page_dims_s *dims, unsigned width, unsigned height);
	bool (*page_prologue) (struct printer_state_s *state, const struct page_dims_s *dims);
	bool (*page_epilogue) (struct printer_state_s *state, const struct page_dims_s *dims);
	void (*send_band) (struct printer_state_s *state,
		const void *band, unsigned line_size, unsigned num_lines);
};

const struct printer_ops_s *printer_detect(void);


void __printer_register_ops(const char *name, const struct printer_ops_s *ops,
		enum printer_support status);

#define ___concat(text, line) text##line
#define __printer_register_func(line) \
	___concat(static void __register_printer_, line)(void)

#define register_printer(mdl, ops, status) \
	__attribute__((constructor(150))) \
	__printer_register_func(__LINE__) \
	{ __printer_register_ops(mdl, &ops, status); } \
	__printer_register_func(__LINE__)
