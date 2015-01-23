/*
 * Copyright (C) Massimo Cora' 2010 <maxcvs@email.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "general.h"	/* must always come first */
#include "read.h"
#include <assert.h>
#include <glib.h>
#include <string.h>


/**
 * IMPORTANT NOTE 
 *
 * This file should contain functions/helpers that aren't strictly 
 * ctags-standardized and that can be used in extra parsers (i.e. parser
 * not included with ctags distribution).
 * Doing so and keeping separate standard/extra files should make 
 * an upgrade of ctags easier.
 */
 

void get_file_pos (gint line, fpos_t *fpos, FILE *f);

