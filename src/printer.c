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

#include "printer.h"

#include "std.h"
#include "capt-command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct printer_rec {
	struct printer_rec *next;
	const char *name;
	const struct printer_ops_s *ops;
	enum printer_support state;
};

static struct printer_rec *printer_recs;

static void __attribute__((constructor(101))) __init_printers(void)
{
	printer_recs = NULL;
}

static void __attribute__((destructor(101))) __clear_printers(void)
{
	while (printer_recs) {
		struct printer_rec *r = printer_recs;
		printer_recs = r->next;
		free(r);
	}
}

static bool ieee_isspace(char c)
{
	switch (c) {
	case 0x20:
	case 0x09:
	case 0x0B:
	case 0x0D:
	case 0x0A:
	case 0x0C:
		return true;
	default:
		return false;
	}
}

static const struct printer_ops_s *find_ops(const char *model, size_t size)
{
	const struct printer_rec *r;
	char mdl[size + 1];
	if (! size) {
		fprintf(stderr, "ERROR: CAPT: printer model name is empty\n");
		return NULL;
	}
	memcpy(mdl, model, size);
	mdl[size + 1] = '\0';
	fprintf(stderr, "DEBUG: CAPT: detected printer '%s'\n", mdl);

	for (r = printer_recs; r; r = r->next) {
		if (! strcmp(r->name, mdl)) {
			switch (r->state) {
			case UNSUPPORTED:
				fprintf(stderr,
					"ERROR: printer %s is not supported by this driver\n", mdl);
				exit(1);
			case BROKEN:
				fprintf(stderr,
					"WARNING: support of %s is currently broken\n", mdl);
				break;
			case EXPERIMENTAL:
				fprintf(stderr,
					"WARNING: support of %s is experimental\n", mdl);
				break;
			default:
				break;
			}
			fprintf(stderr, "DEBUG: CAPT: found ops for printer '%s'\n", mdl);
			return r->ops;
		}
	}

	return NULL;
}

void __printer_register_ops(const char *name, const struct printer_ops_s *ops,
		enum printer_support state)
{
	struct printer_rec *r = malloc(sizeof(struct printer_rec));
	if (! r)
		abort();
	r->name = name;
	r->ops = ops;
	r->state = state;
	r->next = printer_recs;
	printer_recs = r;
}

const struct printer_ops_s *printer_detect(void)
{
	const char *ieee;
	const char *pos;
	const char *end;
	ieee = capt_identify();
	pos = ieee;
	for (; *pos && ieee_isspace(*pos); ++pos)
		;
	end = pos + strlen(pos);
	for (; end != ieee && ieee_isspace(*(end - 1)); --end)
		;

	while (pos != end) {
		const char *delim;
		const char *colon;
		for (delim = pos; delim != end && *delim != ';'; ++delim)
			;
		for (colon = pos; colon != delim && *colon != ':'; ++colon)
			;
		if (colon != delim) {
			size_t klen = colon - pos;
			if ((klen == 5 && !memcmp(pos, "MODEL", 5))
					|| (klen == 3 && !memcmp(pos, "MDL", 3))) {
				const struct printer_ops_s *ret;
				ret = find_ops(colon + 1, delim - colon - 1);
				if (ret)
					return ret;
				break;
			}
		}
		if (delim == end)
			break;
		pos = delim + 1;
	}

	fprintf(stderr, "ERROR: CAPT: unknown printer '%s'\n", ieee);
	exit(1);
}
