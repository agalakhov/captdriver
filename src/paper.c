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

#include "paper.h"
#include <cups/raster.h>
#include <stdio.h>

void page_set_dims(struct page_dims_s *dims, const struct cups_page_header2_s *header)
{
	dims->media_type = header->cupsMediaType;
	strncpy(dims->media_size, header->MediaType, 64);
	dims->paper_width  = header->cupsWidth;  //header->PageSize[0] * header->HWResolution[0] / 72;
	dims->paper_height = header->cupsHeight; //header->PageSize[1] * header->HWResolution[1] / 72;
	dims->toner_save = header->cupsInteger[0];
	dims->ink_k = header->cupsInteger[1];
	dims->line_size = header->PageSize[0];
	dims->num_lines = header->cupsHeight;
	dims->band_size = header->cupsRowCount;
	dims->margin_height = header->Margins[0];
	dims->margin_width = header->Margins[1];

	if (header->HWResolution[1] == 400)
	  dims->media_adapt  = 0x81;
	else
	  dims->media_adapt  = 0x11;
}
