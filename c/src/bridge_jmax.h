/*
	$Id$

	GridFlow
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

#ifndef __BRIDGE_JMAX_H
#define __BRIDGE_JMAX_H

#include <fts/fts.h>

/* **************************************************************** */
/* Var */

typedef fts_atom_t Var;
typedef fts_symbol_t Symbol;
#define Symbol_new(x) fts_new_symbol(x)
#define Symbol_name(x) fts_symbol_name(x)

#endif /* __BRIDGE_JMAX_H */
