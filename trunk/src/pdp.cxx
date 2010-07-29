/*
	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "bundled/pdp_imagebase.h"
#include "gridflow.hxx.fcs"

static t_class *pdpproxy_class;

struct pdpproxy : t_pdp_imagebase {};
static void pdpproxy_process(pdpproxy *x) {
    int p0 = pdp_base_get_packet(x,0);
    /* int mask = pdp_imagebase_get_chanmask(x); */
}
static void pdpproxy_free(pdpproxy *x) {pdp_imagebase_free(x);}
static void *pdpproxy_new () {
    pdpproxy *x = (pdpproxy *)pd_new(pdpproxy_class);
    pdp_imagebase_init(x);
    pdp_base_set_process_method(x, (t_pdp_method)pdpproxy_process);
    pdp_base_add_pdp_outlet(x);
    return x;
}

\class GridToPdp : FObject {
	\constructor () {
	}
	\grin 0
};
GRID_INLET(0) {
	
} GRID_FLOW {
	
} GRID_FINISH {
	
} GRID_END
\end class {install("#to_pdp",1,1);}

\class GridFromPdp : FObject {
	\constructor () {
		
	}
	\decl 0 pdp (t_symbol *s, t_float f) {
		if (s==gensym("register_ro")) {
		} else if (s==gensym("register_rw")) {
		} else if (s==gensym("process")) {
			
		} else RAISE("unknown pdp command");
	}
};
\end class {install("#from_pdp",1,1);}

void pdp_setup() {
	\startall
	pdpproxy_class =
	  class_new(gensym("#to_pdp_proxy"), (t_newmethod)pdpproxy_new, (t_method)pdpproxy_free, sizeof(pdpproxy),0,A_NULL);
	pdp_imagebase_setup(pdpproxy_class);
}
