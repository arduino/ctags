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

#include "ctags-utils.h"

void
get_file_pos (gint line, fpos_t *fpos, FILE *f)
{
	vString * str = vStringNew ();
	gint i;
	g_assert (fseek (f, 0, SEEK_SET) == 0);

	for (i = 0;i < line - 1; i++)
	{
		if (readLine (str, f) == NULL)
		{
			vStringDelete (str);
			return;
		}
	}

	vStringDelete (str);
	g_assert (fgetpos (f, fpos) == 0);
}


