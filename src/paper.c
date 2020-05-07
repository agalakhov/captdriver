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
	dims->paper_width_pts = header->PageSize[0];
	dims->paper_height_pts = header->PageSize[1];
	dims->paper_width  = header->PageSize[0] * header->HWResolution[0] / 72;
	dims->paper_height = header->PageSize[1] * header->HWResolution[1] / 72;
	dims->h_dpi = header->HWResolution[0];
	dims->w_dpi = header->HWResolution[1];
	dims->pause = (header->cupsInteger[1] != 0);
	dims->toner_save = header->cupsInteger[0]; 
}
