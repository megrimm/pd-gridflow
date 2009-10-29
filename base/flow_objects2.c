/*
	$Id: flow_objects.c 4097 2008-10-03 19:49:03Z matju $

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

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

#include "../gridflow.h.fcs"
#ifdef DESIRE
#include "desire.h"
#else
extern "C" {
#include "bundled/g_canvas.h"
#include "bundled/m_imp.h"
extern t_class *text_class;
};
#endif
#define OP(x) op_dict[string(#x)]

static void expect_min_one_dim (P<Dim> d) {
	if (d->n<1) RAISE("expecting at least one dimension, got %s",d->to_s());}

// obviously unfinished
\class GridExpr : FObject {
	\constructor (...) {
		std::ostringstream os;
		for (int i=0; i<argc; i++) os << " " << argv[i];
		string s = os.str();
		post("expr = '%s'",s.data());
	}
};
\end class {install("#expr",1,1);}

\class GridClusterAvg : FObject {
	\attr int numClusters;
	\attr PtrGrid r;
	\attr PtrGrid sums;
	\attr PtrGrid counts;
	\constructor (int v) {_1_float(0,0,v); r.constrain(expect_min_one_dim);}
	\decl 1 float (int v);
	\grin 0 int32
	\grin 2
	template <class T> void make_stats (long n, int32 *ldata, T *rdata) {
		int32 chans = r->dim->v[r->dim->n-1];
		T     *sdata = (T     *)*sums;
		int32 *cdata = (int32 *)*counts;
		for (int i=0; i<n; i++, ldata++, rdata+=chans) {
			if (*ldata<0 || *ldata>=numClusters) RAISE("value out of range in left grid");
			OP(+)->zip(chans,sdata+(*ldata)*chans,rdata);
			cdata[*ldata]++;
		}
		for (int i=0; i<numClusters; i++) OP(/)->map(chans,sdata+i*chans,(T)cdata[i]);
		out = new GridOutlet(this,1,counts->dim,counts->nt);
		out->send(counts->dim->prod(),(int32 *)*counts);
		out = new GridOutlet(this,0,sums->dim,sums->nt);
		out->send(sums->dim->prod(),(T *)*sums);
	}
};

GRID_INLET(0) {
	NOTEMPTY(r);
	int32 v[r->dim->n];
	COPY(v,r->dim->v,r->dim->n-1);
	v[r->dim->n-1]=1;
	P<Dim> t = new Dim(r->dim->n,v);
	if (!t->equal(in->dim)) RAISE("left %s must be equal to right %s except last dimension should be 1",in->dim->to_s(),r->dim->to_s());
	in->set_chunk(0);
	int32 w[2] = {numClusters,r->dim->v[r->dim->n-1]};
	sums   = new Grid(new Dim(2,w),r->nt,  true);
	counts = new Grid(new Dim(1,w),int32_e,true);
} GRID_FLOW {
	#define FOO(U) make_stats(n,data,(U *)*r);
	TYPESWITCH(r->nt,FOO,)
	#undef FOO
} GRID_END
\def 1 float (int v) {numClusters = v;}
GRID_INPUT(2,r) {
} GRID_END

\end class {install("#cluster_avg",3,2);}

\class GFCanvasFileName : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	outlet_symbol(bself->outlets[0],mom->gl_name ? mom->gl_name : gensym("empty"));
}
\end class {install("gf/canvas_filename",1,1);}

\class GFCanvasDollarZero : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	outlet_float(bself->outlets[0],canvas_getenv(mom)->ce_dollarzero);
}
\end class {install("gf/canvas_dollarzero",1,1);}

\class GFCanvasGetPos : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	t_atom a[2];
	SETFLOAT(a+0,mom->gl_obj.te_xpix);
	SETFLOAT(a+1,mom->gl_obj.te_ypix);
	outlet_list(bself->outlets[0],&s_list,2,a);
}
\end class {install("gf/canvas_getpos",1,1);}

\class GFCanvasSetPos : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 list (...);
};
\def 0 list (...) {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	if (argc!=2) RAISE("wrong number of args");
	mom->gl_obj.te_xpix = atom_getfloatarg(0,argc,argv);
	mom->gl_obj.te_ypix = atom_getfloatarg(1,argc,argv);
	t_canvas *granny = mom->gl_owner;
	if (!granny) RAISE("no such canvas");
#ifdef DESIREDATA
	gobj_changed(mom);
#else
        gobj_vis((t_gobj *)mom,granny,0);
        gobj_vis((t_gobj *)mom,granny,1);
	canvas_fixlinesfor(glist_getcanvas(granny), (t_text *)mom);
#endif
}
\end class {install("gf/canvas_setpos",1,0);}

/*
struct GFCanvasEditModeProxy {
	t_pd x_pd;
	GFCanvasEditMode *parent;
};
t_class *GFCanvasEditModeProxy_class;
*/

\class GFCanvasEditMode : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	t_atom a[1]; SETFLOAT(a+0,0);
	outlet_float(bself->outlets[0],mom->gl_edit);
}
\end class {install("gf/canvas_edit_mode",1,1);}

extern "C" void canvas_setgraph(t_glist *x, int flag, int nogoprect);
\class GFCanvasSetGOP : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 float (float gop);
};
\def 0 float (float gop) {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	t_atom a[1]; SETFLOAT(a+0,0);
	canvas_setgraph(mom,gop,0);
}
\end class {install("gf/canvas_setgop",1,0);}

\class GFCanvasXID : FObject {
	int n;
	t_symbol *name;
	\constructor (int n_) {
		n=n_;
		name=symprintf("gf/canvas_xid:%lx",bself);
		pd_bind((t_pd *)bself,name);
	}
	~GFCanvasXID () {pd_unbind((t_pd *)bself,name);}
	\decl 0 bang ();
	\decl 0 whole ();
	\decl 0 xid (t_symbol *t);
};
\def 0 bang () {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	sys_vgui("pd %s xid [winfo id .x%lx.c] \\;\n",name->s_name,long(mom));
}
\def 0 whole () {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	sys_vgui("pd %s xid [winfo id .x%lx] \\;\n",  name->s_name,long(mom));
}
\def 0 xid (t_symbol *t) {outlet_symbol(bself->outlets[0],t);}
\end class {install("gf/canvas_xid",1,1);}

\class GFCanvasHeHeHe : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 float (float y);
};
\def 0 float (float y) {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	mom->gl_screenx2 = mom->gl_screenx1 + 568;
	if (mom->gl_screeny2-mom->gl_screeny1 < y) mom->gl_screeny2 = mom->gl_screeny1+y;
	sys_vgui("wm geometry .x%lx %dx%d\n",long(mom),
	  int(mom->gl_screenx2-mom->gl_screenx1),
	  int(mom->gl_screeny2-mom->gl_screeny1));
}
\end class {install("gf/canvas_hehehe",1,1);}

\class GFCanvasHoHoHo : FObject {
	int n;
	t_canvas *last;
	\constructor (int n) {this->n=n; last=0;}
	void hide () {if (last) sys_vgui(".x%lx.c delete %lxRECT\n",long(last),bself);}
	~GFCanvasHoHoHo () {hide();}
	\decl 0 list (int x1, int y1, int x2, int y2);
};
\def 0 list (int x1, int y1, int x2, int y2) {
	hide();
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	last = mom;
	sys_vgui(".x%lx.c create rectangle %d %d %d %d -outline #00aa66 -dash {3 5 3 5} -tags %lxRECT\n",
		long(last),x1,y1,x2,y2,bself);
}
\end class {install("gf/canvas_hohoho",1,0);}

#define canvas_each(y,x) for (t_gobj *y=x->gl_list; y; y=y->g_next)
\class GFCanvasCount : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 bang ();
};
\def 0 bang () {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	int k=0;
	canvas_each(y,mom) k++;
	outlet_float(bself->outlets[0],k);
}
\end class {install("gf/canvas_count",1,1);}

\class GFCanvasLoadbang : FObject {
	int n;
	\constructor (int n) {this->n=n;}
	\decl 0 float (float m);
};
\def 0 float (float m) {
	t_canvas *mom = bself->mom;
	for (int i=0; i<n; i++) {mom = mom->gl_owner; if (!mom) RAISE("no such canvas");}
	int k=0;
	canvas_each(y,mom) {
		k++;
		if (k>=m && pd_class((t_pd *)y)==canvas_class) canvas_loadbang((t_canvas *)y);
	}
	
}
\end class {
	install("gf/canvas_loadbang",1,0);
#ifndef DESIREDATA
	post("text_class hack begin");
	//text_class->c_firstin = 1;
	class_setpropertiesfn(text_class,(t_propertiesfn)0xDECAFFED);
	unsigned long *lol = (unsigned long *)text_class;
	int i=0;
	while (lol[i]!=0xDECAFFED) i++;
	*((char *)(lol+i+1) + 6) = 1;
	class_setpropertiesfn(text_class,0);
	post("text_class hack end");
#endif
}

\class GFSearchAndReplace : FObject {
	t_symbol *from;
	t_symbol *to;
	\constructor (t_symbol *from, t_symbol *to=&s_) {this->from=from; this->to=to;}
	\decl 0 symbol (t_symbol *victim);
};
\def 0 symbol (t_symbol *victim) {
	string a = string(victim->s_name);
	string b = string(from->s_name);
	string c = string(to->s_name);
	for (size_t i=0;;) {
	  i = a.find(b,i);
	  if (i==string::npos) break;
	  a = a.replace(i,b.length(),c);
	  i += c.length();
	}
	outlet_symbol(bself->outlets[0],gensym(a.c_str()));
}
\end class {install("gf/string_replace",1,1);}

void startup_flow_objects2 () {
	\startall
}
