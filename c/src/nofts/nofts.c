/*
	$Id$

	Video4jmax
	Copyright (c) 2001 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* this file is for allowing video4jmax to be compiled without jMax. */

#include "../grid.h"
#include <string.h>

static const char **symbols;
static int symbols_n = 0;

fts_symbol_t fts_new_symbol (const char *s) {
	int i;
	if (!symbols) symbols = NEW(const char *,1024);
	for (i=0; i<symbols_n; i++) {
		if (strcmp(symbols[i],s)==0) return i;
	}
	if (symbols_n >= 1024) assert(0);
	symbols[symbols_n] = strdup(s);
	return symbols_n++;
}

int main (void) {
	fts_new_symbol("symbol");
	fts_new_symbol("int");
	fts_new_symbol("list");
	fts_new_symbol("ptr");
	fts_new_symbol("init");
	fts_new_symbol("delete");
	fts_new_symbol("set");
	return 0;
}
