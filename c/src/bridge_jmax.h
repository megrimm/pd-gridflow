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
#include "lang.h"
#include <fts/fts.h>

/* **************************************************************** */
/* Var */

typedef fts_atom_t Var;
typedef fts_symbol_t Symbol;
typedef fts_alarm_t Timer;
typedef fts_object_t FObject;

#define Object_send_thru(_self_,_outlet_,_selector_,ac,at) \
		fts_outlet_send(OBJ(_self_),_outlet_,_selector_,ac,at)

#define Symbol_new(x) fts_new_symbol(x)
#define Symbol_name(x) fts_symbol_name(x)

#define Var_has_symbol(a)   fts_is_symbol(a)
#define Var_has_int(a)      fts_is_int(a)
#define Var_get_symbol(a)   fts_get_symbol(a)
#define Var_get_int(a)      fts_get_int(a)
#define Var_get_ptr(a)      fts_get_ptr(a)
#define Var_put_symbol(a,b) fts_set_symbol(a,b)
#define Var_put_int(a,b)    fts_set_int(a,b)
#define Var_put_ptr(a,b)    fts_set_ptr(a,b)

#define Timer_new(a,b,c) fts_alarm_new(a,b,c)
#define Timer_set_delay(a,b) fts_alarm_set_delay(a,b)
#define Timer_arm(a) fts_alarm_arm(a)

#define sprintf_vars(buf,ac,at) sprintf_atoms(buf,ac,at)

#endif /* __BRIDGE_JMAX_H */
