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

void page_set_dims(struct page_dims_s *dims, const struct cups_page_header2_s *header)
{
	dims->media_type = header->cupsMediaType;
	dims->paper_width  = header->PageSize[0] * header->HWResolution[0] / 72;
	dims->paper_height = header->PageSize[1] * header->HWResolution[1] / 72;
	dims->margin_width = header->Margins[0] * header->HWResolution[0] / 72;
	dims->margin_height = header->Margins[1] * header->HWResolution[1] / 72;;

	if (dims->auto_set) {
		/* Assume image margin is equal on both sides */
		unsigned cut_width = (header->PageSize[0] - header->Margins[0]*2);
		unsigned cut_height = (header->PageSize[1] - header->Margins[1]*2);
		unsigned const *res = header->HWResolution;
		dims->line_size = (cut_width * res[1] / 72)/8;
		dims->num_lines = cut_height * res[0] / 72;
		/* Increase line size and count to next larger value divisible by 4 */
		dims->line_size += dims->line_size % 4;
		dims->num_lines += dims->num_lines % 4;
	}
	/* 
		The use of cupsCompression to toggle toner save was inspired by the
	 	use of the same attribute to control darkness in label printer drivers.
	*/
	dims->toner_save = header->cupsCompression; 
}
