/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

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

#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <errno.h>
#include "gridflow.hxx.fcs"
//using namespace std; // can't
//#undef GRID_INPUT
//#define GRID_INPUT(I,V) GRID_INLET(I) {in.buf=V=new Grid(in.dim,NumberTypeE_type_of(data));} GRID_FINISH

#ifdef __MINGW32__
void *alloca(size_t);
#endif

/* ---------------------------------------------------------------- */

// BAD HACK: GCC complains: unimplemented (--debug mode only) (i don't remember which GCC this was)
#ifdef HAVE_DEBUG
#define SCOPY(a,b,n) COPY(a,b,n)
#else
#define SCOPY(a,b,n) SCopy<n>::f(a,b)
#endif
template <long n> struct SCopy {template <class T> static inline void __attribute__((always_inline)) f(T *a, T *b) {*a=*b; SCopy<n-1>::f(a+1,b+1);}};
template <>    struct SCopy<0> {template <class T> static inline void __attribute__((always_inline)) f(T *a, T *b) {}};

/*template <> class SCopy<4> {
public: template <class T>
	static inline void __attribute__((always_inline)) f(T *a, T *b) {
		*a=*b; SCopy<3>::f(a+1,b+1);}
	static inline void __attribute__((always_inline)) f(uint8 *a, uint8 *b)
	{ *(int32 *)a=*(int32 *)b; }
};*/

Numop2 *op_add, *op_sub, *op_mul, *op_div, *op_mod, *op_shl, *op_and, *op_put;

static CONSTRAINT(expect_dim_dim_list) {if (d.n!=1) RAISE("dimension list should be Dim[n], not %s",d.to_s());}
//static CONSTRAINT(expect_min_one_dim) {if (d.n<1 ) RAISE("minimum 1 dimension");}
static CONSTRAINT(expect_max_one_dim) {if (d.n>1 ) RAISE("expecting Dim[] or Dim[n], got %s",d.to_s());}
//static CONSTRAINT(expect_exactly_one_dim) {if (d.n!=1) RAISE("expecting Dim[n], got %s",d.to_s());}

//****************************************************************
\class GridCast {
	\attr NumberTypeE cast;
	\constructor (NumberTypeE nt=int32_e) {this->cast = nt;}
	\grin 0
};
GRID_INLET(0) {
	go = new GridOut(this,0,in.dim,cast);
} GRID_FLOW {
	go->send(n,data);
} GRID_END
\end class {install("#cast",1,1); add_creator("@cast");}

//****************************************************************

GridHandler *stromgol; // remove this asap
//{ ?,Dim[B] -> Dim[*Cs] }
// out0 nt to be specified explicitly
\class GridImport {
	\attr NumberTypeE cast;
	/*\attr*/ Dim dim; // size of grids to send
	\attr bool per_message;
	P<Grid> dim_grid;
	\constructor (...) {
		per_message = true;
		dim_grid.but(expect_dim_dim_list);
		this->cast = argc>=2 ? NumberTypeE_find(argv[1]) : int32_e;
		if (argc>2) RAISE("too many arguments");
		if (argc>0 && argv[0]!=gensym("per_message")) {
			per_message=false;
			dim_grid=new Grid(argv[0]);
			dim = dim_grid->to_dim();
			if (!dim.prod()) RAISE("target grid size must not be zero");
		}
	}
	\decl 0 reset() {int32 foo[1]={0}; if (go) while (go->sender) go->send(1,foo);}
	\decl 0 symbol(t_symbol *x) {
		const char *name = x->s_name;
		long n = strlen(name);
		if (per_message) go=new GridOut(this,0,Dim(n));
		process(n,(uint8 *)name);
	}
	\decl 0 to_ascii(...) {
		ostringstream os;
		pd_oprint(os,argc,argv);
		string s = os.str();
		long n = s.length();
		if (per_message) go=new GridOut(this,0,Dim(n),cast);
		process(n,(uint8 *)s.data());
	}
	\decl 1 per_message() {per_message=true; dim_grid=0;}
	\grin 0
	\grin 1 int32
	template <class T> void process (long n, T *data) {
		if (in.size()<=0) in.resize(1);
		if (!in[0]) in[0]=new GridInlet((FObject *)this,stromgol);
		while (n) {
			if (!go || !go->sender) {
				// the cast of per_message was changed twice, because of [hpgl_font_render]
				// and because of something else. This suggests that there is a problem about
				// what [#import] is expected to do in per_message mode. This could lead to a
				// special default value of cast that wouldn't be a NumberType but instead would
				// be "copy" or "from_message" or some similar name. But is that what we want
				// to be using as a default instead of cast i, even when #import is not per_message ?
				if (per_message) go = new GridOut(this,0,in[0]->dim,     cast /*in[0]->nt*/);
				else             go = new GridOut(this,0,       dim,     cast);
			}
			long n2 = min((long)n,go->dim.prod()-go->dex);
			go->send(n2,data);
			n-=n2; data+=n2;
		}
	}
};
GRID_INLET(0) {} GRID_FLOW {process(n,data);} GRID_FINISH {
	if (!in.dim.prod() && per_message) go = new GridOut(this,0,Dim(0),cast);
} GRID_END
GRID_INPUT(1,dim_grid) {
	Dim d = dim_grid->to_dim();
	if (!d.prod()) RAISE("target grid size must not be zero");
	dim = d;
	per_message=false;
} GRID_END
// needs to stay a \def because it's implicitly \decl'd in \grin 0
\def 0 list(...) {//first two lines are there until grins become strictly initialized.
	if (in.size()<=0) in.resize(1);
	if (!in[0]) {/*post("[#import] INIT INLET");*/ in[0]=new GridInlet((FObject *)this,stromgol);}
	in[0]->from_list(argc,argv,cast);
}
\end class {install("#import",2,1); add_creator("@import"); stromgol = &GridImport_grid_0_hand;}

//****************************************************************
/*{ Dim[*As] -> ? }*/
/* in0: integer nt */
\class GridToFloat {
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {
} GRID_FLOW {
	for (int i=0; i<n; i++) out[0](float(data[i]));
} GRID_END
\end class {install("#to_float",1,1); add_creator("#to_f"); add_creator("#export"); add_creator("@export");}

\class GridToSymbol {
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {
	in.set_chunk(0);
} GRID_FLOW {
	char c[n+1];
	for (int i=0; i<n; i++) c[i]=(char)data[i];
	c[n]=0;
	out[0](gensym(c));
} GRID_END
\end class {install("#to_symbol",1,1); add_creator("#to_s"); add_creator("#export_symbol"); add_creator("@export_symbol");}

/*{ Dim[*As] -> ? }*/
/* in0: integer nt */
\class GridToList {
	\constructor () {}
	int n;
	\grin 0
};
GRID_INLET(0) {
	long n = in.dim.prod();
	if (n>1000000) RAISE("list too big (%ld elements, max 1000000)", n);
	this->n = n;
	in.set_chunk(0);
} GRID_FLOW {
	send_out(0,n,data);
} GRID_FINISH {
	if (in.dim.prod()==0) send_out(0,0,data);
} GRID_END
\end class {install("#to_list",1,1); add_creator("#to_l"); add_creator("#export_list"); add_creator("@export_list");}

/* **************************************************************** */
\class GridPrint {
	ostream *dest;
	void redirect(ostream *dest);
	int base;
	uint32 trunc;
	int columns;
	\attr t_symbol *name;
	\attr int maxrows;
	\constructor (t_symbol *name=0) {
		this->dest = 0;
		this->name = name;
		base=10; trunc=70; maxrows=50;
	}
	\grin 0
	\decl 0 base (int x) {if (x==2 || x==8 || x==10 || x==16) base=x; else RAISE("base %d not supported",x);}
	\decl 0 trunc (int x) {if (x<0 || x>240) RAISE("out of range (not in 0..240 range)"); trunc = x;}
	void puts (const char *s) {if (dest) *dest << s << "\n"; else post("%s",s);}
	void puts (string s) {puts(s.data());}
	void puts (ostringstream &s) {puts(s.str());}
	template <class T> void make_columns (int n, T *data) {
		if (NumberTypeE_type_of(data)==float32_e) {columns=10; return;}
		if (NumberTypeE_type_of(data)==float64_e) {columns=20; return;}
		long maxv=0;
		long minv=0;
		for (int i=0; i<n; i++) {
			if (maxv<data[i]) maxv=long(data[i]);
			if (minv>data[i]) minv=long(data[i]);
		}
		int maxd = 1 + (maxv<0) + int(log(max(1.,fabs(maxv)))/log(base));
		int mind = 1 + (minv<0) + int(log(max(1.,fabs(minv)))/log(base));
		columns = max(maxd,mind);
	}
	string format (NumberTypeE nt) {
		if (nt==float32_e) return "%.6g";
		if (nt==float64_e) return "%.14g";
		ostringstream r;
		r << "%";
		r << columns;
		//if (nt==int64_e) r << "l";
		r << "d"; // integer bases 2,8,16 are no longer handled here
		return r.str();
	}
	template <class T> void dump(ostream &s, int n, T *data, char sep=' ', int trunc=-1) {
		if (trunc<0) trunc=this->trunc;
		string f = format(NumberTypeE_type_of(data));
		for (int i=0; i<n; i++) {
			if (base==2 || base==8 || base==16) {
				static char digits[] = "0123456789ABCDEF";
				int chunk = highest_bit((uint32)base);
				T x = gf_abs(data[i]);
				int ndigits = 1 + highest_bit(uint64(x))/chunk;
				for (int j=columns-ndigits-(data[i]!=x); j>=0; j--) s<<' ';
				if (data[i]!=x) s<<'-';
				for (int j=ndigits-1; j>=0; j--) s<<digits[((uint64)x>>(j*chunk))&(base-1)];
			} else {
				int count = snprintf(0,0,f.data(),data[i]);
				if (columns>count) oprintf(s,"%*s",int(columns-count),"");
				oprintf(s,f.data(),data[i]);
			}
			if (i<n-1) s << sep;
			if (s.tellp()>trunc) return;
		}
	}
	void dump_dims(ostream &s, GridInlet &in) {
		if (name && name!=&s_) s << name->s_name << ": ";
		s << "Dim[";
		for (int i=0; i<in.dim.n; i++) {
			s << in.dim.v[i];
			if (i<in.dim.n-1) s << ',';
		}
		s << "]";
		if (in.nt!=int32_e) s << "(" << number_type_table[in.nt].name << ")";
		s << ": ";
	}
};
GRID_INLET(0) {
	in.set_chunk(0);
} GRID_FLOW {
	ostringstream head;
	dump_dims(head,in);
	int ndim = in.dim.n;
	if (ndim > 3) {
		head << " (not printed)";
		puts(head);
	} else if (ndim < 2) {
		make_columns(n,data);
		dump(head,n,data,' ',trunc);
		puts(head);
	} else if (ndim == 2) {
		puts(head);
		make_columns(n,data);
		long sy = in.dim[0];
		long sx = n/sy;
		for (int row=0; row<sy; row++) {
			if (row>=maxrows) {puts("..."); break;}
			ostringstream body;
			dump(body,sx,&data[sx*row],' ',trunc);
			if (body.tellp()>trunc) body << "...";
			puts(body);
		}
	} else if (ndim == 3) {
		puts(head);
		make_columns(n,data);
		int sy = in.dim[0];
		int sx = in.dim[1];
		int sz = n/sy;
		int sz2 = sz/in.dim[1];
		for (int row=0; row<sy; row++) {
			ostringstream str;
			for (int col=0; col<sx; col++) {
				str << "(";
				dump(str,sz2,&data[sz*row+sz2*col],' ',trunc);
				if (str.tellp()>trunc) {str << "..."; break;} else str << ")";
			}
			puts(str);
			if (row>maxrows) {puts("..."); break;}
		}
	}
} GRID_FINISH {
	ostringstream head;
	dump_dims(head,in);
	if (in.dim.prod()==0) puts(head);
} GRID_END
\end class {install("#print",1,1,CLASS_NOPARENS); add_creator("@print");}
void GridPrint::redirect (ostream *dest) {this->dest=dest;}

/* **************************************************************** */
// [#store] is the class for storing a grid and restituting it on demand.
// The right inlet receives the grid. The left inlet receives either a bang
// (which forwards the whole image) or a grid describing what to send.
//{ Dim[*As,B],Dim[*Cs,*Ds] -> Dim[*As,*Ds] }
// in0: integer nt
// in1: whatever nt
// out0: same nt as in1
\class GridStore {
	P<Grid> r; // can't be \attr (why ?)
	P<Grid> put_at; // can't be //\attr (why ?)
	\attr Numop2 *op;
	int32 *wdex ; // temporary buffer, copy of put_at
	int32 *fromb;
	int32 *to2  ;
	int lsd; // lsd = Last Same Dimension (for put_at)
	int d; // goes with wdex
	long cs; // chunksize used in put_at
	\constructor (Grid *r=0) {
		put_at.but(expect_max_one_dim);
		this->r = r?r:new Grid(Dim(),int32_e,true);
		op = op_put;
		wdex  = NEWBUF(int32,Dim::MAX_DIM); // temporary buffer, copy of put_at
		fromb = NEWBUF(int32,Dim::MAX_DIM);
		to2   = NEWBUF(int32,Dim::MAX_DIM);
	}
	~GridStore () {
		DELBUF(wdex);
		DELBUF(fromb);
		DELBUF(to2);
	}
	\decl 0 bang () {t_atom2 a[2] = {0,gensym("#")}; pd_list((t_pd *)bself,&s_list,2,a);}
	\decl 1 reassign () {put_at=0;}
	\decl 1 put_at (...) {
		if (argv[0].a_type==A_LIST) put_at=(Grid *)argv[0];
		else {
			put_at=new Grid(Dim(argc),int32_e);
			int32 *v = (int32 *)*put_at;
			for (int i=0; i<argc; i++) v[i]=argv[i];
		}
	}
	\grin 0 int
	\grin 1
	template <class T> void compute_indices(T *v, long nc, long nd);
};

// takes the backstore of a grid and puts it back into place. a backstore
// is a grid that is filled while the grid it would replace has not
// finished being used.
static void snap_backstore (P<Grid> &r) {if (r && r->next) {P<Grid> tmp=r->next; r=tmp;}}

template <class T> void GridStore::compute_indices(T *v, long nc, long nd) {
	for (int i=0; i<nc; i++) {
		uint32 wrap = r->dim[i];
		bool fast = lowest_bit(wrap)==highest_bit(wrap); // is power of two?
		if (i) {
			if (fast) op_shl->map(nd,v,(T)highest_bit(wrap));
			else      op_mul->map(nd,v,(T)wrap);
		}
		if (fast) op_and->map(nd,v+nd*i,(T)(wrap-1));
		else      op_mod->map(nd,v+nd*i,(T)(wrap));
		if (i) op_add->zip(nd,v,v+nd*i);
	}
}

// !@#$ i should ensure that n is not exceedingly large
// !@#$ worse: the size of the foo buffer may still be too large
GRID_INLET(0) {
	// snap_backstore must be done before *anything* else
	snap_backstore(r);
	int na = in.dim.n;
	int nb = r->dim.n;
	int32 v[Dim::MAX_DIM];
	if (na<1) RAISE("must have at least 1 dimension, got %d",na);
	long nc = in.dim[na-1];
	if (nc>nb) RAISE("got %ld elements in last dimension, expecting <= %d",nc,nb);
	long nnc = r->dim.prod(nc);
	int lastindexable = nnc ? r->dim.prod()/nnc-1 : 0; // SIGFPE happened when r was especially empty (nnc==0)
	int ngreatest = nt_greatest((T *)0);
	if (lastindexable > ngreatest) RAISE("lastindexable=%d > ngreatest=%d (ask matju)",lastindexable,ngreatest);
	int nd = nb-nc+na-1;
	COPY(v,in.dim.v,na-1);
	COPY(v+na-1,r->dim.v+nc,nb-nc);
	go=new GridOut(this,0,Dim(nd,v),r->nt);
	if (nc>0) in.set_chunk(na-1);
} GRID_FLOW {
	int na = in.dim.n;
	int nc = in.dim[na-1];
	long size = r->dim.prod(nc);
	long nd = n/nc;
	T w[n];
	T *v=w;
	if (sizeof(T)==1 && nc==1 && r->dim[0]<=256) {
		// bug? shouldn't modulo be done here?
		v=data;
	} else {
		COPY(v,data,n);
		for (long k=0,i=0; i<nc; i++) for (long j=0; j<n; j+=nc) v[k++] = data[i+j];
		compute_indices(v,nc,nd);
	}
#define FOO(type) { \
	type *p = (type *)*r; \
	if (size<=16) { \
		type tada[nd*size]; \
		type *foo = tada; \
		long i=0; \
		switch (size) { \
		case 1: for (; i<(nd&-4); i+=4, foo+=4) { \
			foo[0] = p[v[i+0]]; \
			foo[1] = p[v[i+1]]; \
			foo[2] = p[v[i+2]]; \
			foo[3] = p[v[i+3]]; \
		} break; \
		case 2: for (; i<nd; i++, foo+=2) SCOPY(foo,p+2*v[i],2); break; \
		case 3: for (; i<nd; i++, foo+=3) SCOPY(foo,p+3*v[i],3); break; \
		case 4: for (; i<nd; i++, foo+=4) SCOPY(foo,p+4*v[i],4); break; \
		default:; }; \
		for (; i<nd; i++, foo+=size) COPY(foo,p+size*v[i],size); \
		go->send(size*nd,tada); \
	} else { \
		for (int i=0; i<nd; i++) go->send(size,p+size*v[i]); \
	} \
}
	TYPESWITCH(r->nt,FOO,)
#undef FOO
} GRID_FINISH {
	if (in.dim.prod()==0) {
		long n = in.dim.prod(0,-2);
		long size = r->dim.prod();
#define FOO(T) while (n--) go->send(size,(T *)*r);
		TYPESWITCH(r->nt,FOO,)
#undef FOO
	}
} GRID_END

GRID_INLET(1) {
	NumberTypeE nt = NumberTypeE_type_of(data);
	if (!put_at) { // reassign
		//!@#$ this was supposed to be inlets[0].sender i suppose... but it's not.
		if (in.sender) r->next = new Grid(in.dim,nt);
		else           r       = new Grid(in.dim,nt);
		return;
	}
	// put_at ( ... )
	snap_backstore(r);
	SAME_TYPE(in,r);
	long nn=r->dim.n, na=put_at->dim[0], nb=in.dim.n;
	if (nn<na) RAISE("stored grid dims (%ld) < length of put_at list (%ld)",nn,na);
	if (nn<nb) RAISE("stored grid dims (%ld) < right inlet dims (%ld)"     ,nn,nb);
	int32 sizeb[nn];
	for (int i=0; i<nn; i++) {fromb[i]=0; sizeb[i]=1;}
	COPY(wdex       ,(int32 *)*put_at   ,put_at->dim.prod());
	COPY(fromb+nn-na,(int32 *)*put_at   ,na);
	COPY(sizeb+nn-nb,(int32 *)in.dim.v,nb);
	for (int i=0; i<nn; i++) to2[i] = fromb[i]+sizeb[i];
	d=0;
	// find out when we can skip computing indices
	//!@#$ should actually also stop before blowing up packet size
	lsd=nn;
	while (lsd>=nn-in.dim.n) {
		lsd--;
		//int cs = in.dim.prod(lsd-nn+in.dim.n);
		if (/*cs*(number_type_table[in.nt].size/8)>GridOut::MAX_PACKET_SIZE ||*/
			fromb[lsd]!=0 || sizeb[lsd]!=r->dim[lsd]) break;
	}
	lsd++;
	long chunk = lsd-nn+in.dim.n;
	in.set_chunk(    chunk);
	cs = in.dim.prod(chunk);
} GRID_FLOW {
	if (!put_at) { // reassign
		COPY(((T *)*(r->next ? r->next.p : &*r.p))+in.dex, data, n);
		return;
	}
	// put_at (...)
	int32 v[lsd];
	int32 *x = wdex;
	while (n) {
		// here d is the dim# to reset; d=n for none
		for(;d<lsd;d++) x[d]=fromb[d];
		COPY(v,x,lsd);
		compute_indices(v,lsd,1);
		op->zip(cs,(T *)*r+v[0]*cs,data);
		data+=cs;
		n-=cs;
		// find next set of indices; here d is the dim# to increment
		for(;;) {d--; if (d<0) return; x[d]++; if (x[d]<to2[d]) break;}
		d++;
	}
} GRID_END
\end class {install("#store",2,1); add_creator("@store");}

//****************************************************************
//{ Dim[*As]<T> -> Dim[*As]<T> } or
//{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As]<T> }
\class GridOp {
	\attr Numop *op;
	P<Grid> r;
	\constructor (Numop *op=0, Grid *r=0) {
		this->op=op?op:TO(Numop2*,gensym("ignore"));
		this->r=r?r:new Grid(Dim(),int32_e,true);
		//post("op->name=%s op->size=%d op->arity=%d",op->name,op->size,op->arity());
	}
	\grin 0
	\grin 1
};
GRID_INLET(0) {
	snap_backstore(r);
	if (op->arity()==2) SAME_TYPE(in,r);
	if (op->size>1 && (in.dim[in.dim.n-1]!=op->size || (op->arity()==2 && r->dim[r->dim.n-1]!=op->size)))
		RAISE("using %s requires Dim(...,%d) in both inlets but got: left=%s right=%s",
			op->name,op->size,in.dim.to_s(),r->dim.to_s());
	go=new GridOut(this,0,in.dim,in.nt);
	//if (go->inlets.size()==1) post("[#]: 1 receiver with bugger size %s",go->inlets[0]->dim.to_s());
	if (go->fresh) go->create_buf(); /* force it now (hack) */
} GRID_FLOW {
	long moton = go->buf.dim.prod();
	if (op->arity()==2) {
		Numop2 *op = (Numop2 *)this->op;
		T *rdata = (T *)*r;
		long loop = r->dim.prod();
		while (n) {
			long pn = min(moton,n);
			T tada[pn];
			COPY(tada,data,pn);
			if (loop>1) {
				if (in.dex+pn <= loop) op->zip(pn/op->size,tada,rdata+go->dex); else {
					// !@#$ should prebuild and reuse this array when "loop" is small
					T data2[pn];
					long ii = mod(go->dex,loop);
					long m = min(loop-ii,pn);
					COPY(data2,rdata+ii,m);
					long nn = m+((pn-m)/loop)*loop;
					for (long i=m; i<nn; i+=loop) COPY(data2+i,rdata,loop);
					if (pn>nn) COPY(data2+nn,rdata,pn-nn);
					op->zip(pn/op->size,tada,data2);
				}
			} else op->map(pn,tada,loop ? *rdata : T(0));
			go->send(pn,tada);
			n-=pn; data+=pn;
		}
	} else {
		Numop1 *op = (Numop1 *)this->op;
		T tada[n];
		COPY(tada,data,n);
		op->map(n/op->size,tada);
		go->send(n,tada);
	}
} GRID_END
GRID_INPUT2(1,r) {} GRID_END
\end class {install("#",2,1); add_creator("@");}

//****************************************************************
\class GridFold {
	\attr Numop2 *op;
	\attr P<Grid> seed;
	\constructor (Numop2 *op) {this->op=op;}
	\grin 0
};

GRID_INLET(0) {
	//{ Dim[*As,B,*Cs]<T>,Dim[*Cs]<T> -> Dim[*As,*Cs]<T> }
	if (seed) SAME_TYPE(in,seed);
	int an = in.dim.n;
	int bn = seed?seed->dim.n:0;
	if (an<=bn) RAISE("minimum 1 more dimension than the seed (%d vs %d)",an,bn);
	int32 v[an-1];
	int yi = an-bn-1;
	COPY(v,in.dim.v,yi);
	COPY(v+yi,in.dim.v+an-bn,bn);
	if (seed) SAME_DIM(an-(yi+1),in.dim,(yi+1),seed->dim,0);
	if (!op->on(*data)->fold) RAISE("operator %s does not support fold",op->name);
	go=new GridOut(this,0,Dim(an-1,v),in.nt);
	in.set_chunk(yi);
	if (in.dim.prod(yi)==0) {
		long n = go->dim.prod();
		T x=0; op->on(x)->neutral(&x,at_left);
		for(long i=0; i<n; i++) go->send(1,&x);
	}
} GRID_FLOW {
	int an = in.dim.n;
	int bn = seed?seed->dim.n:0;
	long yn = in.dim[an-bn-1];
	long zn = in.dim.prod(an-bn);
	T buf[n/yn];
	long nn=n;
	long yzn=yn*zn;
	if (!seed && yn==1) {go->send(nn/yn,data); return;}
	/* don't know why this isn't any faster than what there was before 9.13 */
	/* except for the yn==1 case above, which makes a huge difference */
	if (seed)    for (long i=0; n; i+=zn, data+=yzn, n-=yzn) {COPY(buf+i,((T *)*seed),zn); op->fold(zn,yn  ,buf+i,data);}
	else if (yn) for (long i=0; n; i+=zn, data+=yzn, n-=yzn) {op_put->zip(zn,buf+i,data);  op->fold(zn,yn-1,buf+i,data+zn);}
	else {
		T neu; op->on(*buf)->neutral(&neu,at_left); 
		for (long i=0; n; i+=zn, data+=yzn, n-=yzn) op_put->map(zn,buf+i,neu);
	}
	go->send(nn/yn,buf);
} GRID_FINISH {
} GRID_END

\end class {install("#fold",1,1);}

\class GridScan {
	\attr Numop2 *op;
	\attr P<Grid> seed;
	\constructor (Numop2 *op) {this->op = op;}
	\grin 0
};

GRID_INLET(0) {
	//{ Dim[*As,B,*Cs]<T>,Dim[*Cs]<T> -> Dim[*As,B,*Cs]<T> }
	if (seed) SAME_TYPE(in,seed);
	int an = in.dim.n;
	int bn = seed?seed->dim.n:0;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand");
	if (seed) SAME_DIM(bn,in.dim,an-bn,seed->dim,0);
	if (!op->on(*data)->scan) RAISE("operator %s does not support scan",op->name);
	go=new GridOut(this,0,in.dim,in.nt);
	in.set_chunk(an-bn-1);
} GRID_FLOW {
	int an = in.dim.n;
	int bn = seed?seed->dim.n:0;
	long yn = in.dim[an-bn-1];
	long zn = in.dim.prod(an-bn);
	long factor = yn*zn;
	T buf[n];
	COPY(buf,data,n);
	if (seed) {
		for (long i=0; i<n; i+=factor) op->scan(zn,yn,(T *)*seed,buf+i);
	} else {
		T neu; op->on(*buf)->neutral(&neu,at_left);
		T seed[zn]; op_put->map(zn,seed,neu);
		for (long i=0; i<n; i+=factor) op->scan(zn,yn,      seed,buf+i);
	}
	go->send(n,buf);
} GRID_END

\end class {install("#scan",1,1);}

//****************************************************************
// L      is a Dim[*si,sj,    *ss]<T>
// R      is a Dim[    sj,*sk,*ss]<T>
// Seed   is a Dim[           *ss]<T>
// result is a Dim[*si,   *sk,*ss]<T>
// Currently *ss can only be = Dim[]
\class GridInner {
	\attr Numop2 *op;
	\attr Numop2 *fold;
	\attr P<Grid> seed;
	P<Grid> r;
	P<Grid> r2; // temporary
	bool use_dot;
	\constructor (Grid *r=0) {
		this->op = op_mul;
		this->fold = op_add;
		this->seed = 0;
		this->r    = r ? r : new Grid(Dim(),int32_e,true);
	}
	\grin 0
	\grin 1
};

// let's see this as a matrix product like L[i,j]*R[j,k] in Einstein notation
//   L: matrix of size si by sj
//   R: matrix of size sj by sk
//  LR: matrix of size si by sk
#define FOO for (int j=0; j<chunk; j++, as+=sk, bs+=sj) op_put->map(sk,as,*bs);
template <class T> void inner_child_a (T *as, T *bs, int sj, int sk, int chunk) {FOO}
template <class T, int sk> void inner_child_b (T *as, T *bs, int sj, int chunk) {FOO}
#undef FOO

// Inner product in a Module on the (+,*) Ring ... or on the (^,&) Ring
//     | BBBBB
//     j BBBBB
//     | BBBBB
// -j--*---k---
// AAA i CCCCC
// AAA | CCCCC
#define MAKE_DOT(NAME,FOO) \
	template <class T> void NAME (long sk, long sj, T *cs, T *as, T *bs) {FOO} \
	template <class T, long sj> void NAME (long sk, T *cs, T *as, T *bs) {FOO} \
	template <class T, long sj, long sk> void NAME (T *cs, T *as, T *bs) {FOO}
#define FOO for (long k=0; k<sk; k++) {T c=0; for (long j=0; j<sj; j++) {c+=as[j]*bs[j*sk+k];} *cs++=c;}
MAKE_DOT(dot_add_mul,FOO)
#undef FOO

#define MAX_PACKET_SIZE 4096
GRID_INLET(0) {
	SAME_TYPE(in,r);
	if (seed) SAME_TYPE(in,seed);
	Dim &a=in.dim, &b=r->dim;
	if (a.n<1) RAISE("a: minimum 1 dimension");
	if (b.n<1) RAISE("b: minimum 1 dimension");
	if (seed && seed->dim.n != 0) RAISE("seed must be a scalar");
	int n = a.n+b.n-2;
	SAME_DIM(1,a,a.n-1,b,0);
	int32 v[n];
	COPY(v,a.v,a.n-1);
	COPY(v+a.n-1,b.v+1,b.n-1);
	go=new GridOut(this,0,Dim(n,v),in.nt);
	in.set_chunk(a.n-1);
	long sjk=r->dim.prod(), sj=in.dim.prod(a.n-1), sk=sjk/sj;
	long chunk = max(1L,MAX_PACKET_SIZE/sjk);
	T *rdata = (T *)*r;
	r2=new Grid(Dim(chunk*sjk),r->nt);
	T *buf3 = (T *)*r2;
	for (long i=0; i<sj; i++)
		for (long j=0; j<chunk; j++)
			COPY(buf3+(j+i*chunk)*sk,rdata+i*sk,sk);
	use_dot = op==op_mul && fold==op_add && (!seed || (seed->dim.n==0 && *(T *)*seed==0));
} GRID_FLOW {
    long sjk=r->dim.prod(), sj=in.dim.prod(in.dim.n-1), sk=sjk/sj;
    long chunk = max(1L,MAX_PACKET_SIZE/sjk), off=chunk;
    T buf [chunk*sk];
    T buf2[chunk*sk];
    if (use_dot) {
	while (n) {
		if (chunk*sj>n) chunk=n/sj;
		if (sj<=4 && sk<=4) switch ((sj-1)*4+(sk-1)) {
		#define DOT_ADD_MUL_3(sj,sk) \
			case (sj-1)*4+(sk-1): for (int i=0; i<chunk; i++) dot_add_mul<T,sj,sk>( buf2+sk*i,data+sj*i,(T *)*r); break;
		DOT_ADD_MUL_3(1,1) DOT_ADD_MUL_3(1,2) DOT_ADD_MUL_3(1,3) DOT_ADD_MUL_3(1,4)
		DOT_ADD_MUL_3(2,1) DOT_ADD_MUL_3(2,2) DOT_ADD_MUL_3(2,3) DOT_ADD_MUL_3(2,4)
		DOT_ADD_MUL_3(3,1) DOT_ADD_MUL_3(3,2) DOT_ADD_MUL_3(3,3) DOT_ADD_MUL_3(3,4)
		DOT_ADD_MUL_3(4,1) DOT_ADD_MUL_3(4,2) DOT_ADD_MUL_3(4,3) DOT_ADD_MUL_3(4,4)
		} else switch (sj) {
#define DOT_ADD_MUL_2(sj) case sj: for (int i=0; i<chunk; i++) dot_add_mul<T,sj>(sk,buf2+sk*i,data+sj*i,(T *)*r); break;
		DOT_ADD_MUL_2(1)
		DOT_ADD_MUL_2(2)
		DOT_ADD_MUL_2(3)
		DOT_ADD_MUL_2(4)
		default:for (int i=0; i<chunk; i++) dot_add_mul(sk,sj,buf2+sk*i,data+sj*i,(T *)*r);
		}
		go->send(chunk*sk,buf2);
		n-=chunk*sj;
		data+=chunk*sj;
	}
    } else {
	while (n) {
		if (chunk*sj>n) chunk=n/sj;
		op_put->map(chunk*sk,buf2, seed ? *(T *)*seed : T(0));
		for (long i=0; i<sj; i++) {
			switch (sk) {
			case 1:  inner_child_b<T,1>(buf,data+i,sj,chunk); break;
			case 2:  inner_child_b<T,2>(buf,data+i,sj,chunk); break;
			case 3:  inner_child_b<T,3>(buf,data+i,sj,chunk); break;
			case 4:  inner_child_b<T,4>(buf,data+i,sj,chunk); break;
			default: inner_child_a(buf,data+i,sj,sk,chunk);
			}
			op->zip(chunk*sk,buf,(T *)*r2+i*off*sk);
			fold->zip(chunk*sk,buf2,buf);
		}
		go->send(chunk*sk,buf2);
		n-=chunk*sj;
		data+=chunk*sj;
	}
    }
} GRID_FINISH {
	r2=0;
} GRID_END

GRID_INPUT(1,r) {} GRID_END

\end class {install("#inner",2,1);}

/* **************************************************************** */
/*{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As,*Bs]<T> }*/
\class GridOuter {
	\attr Numop2 *op;
	P<Grid> r;
	\constructor (Numop2 *op, Grid *r=0) {
		this->op = op;
		this->r = r ? r : new Grid(Dim(),int32_e,true);
	}
	\grin 0
	\grin 1
};

GRID_INLET(0) {
	SAME_TYPE(in,r);
	Dim &a = in.dim, &b = r->dim;
	int n = a.n+b.n;
	int32 v[n];
	COPY(v,a.v,a.n);
	COPY(v+a.n,b.v,b.n);
	go=new GridOut(this,0,Dim(n,v),in.nt);
} GRID_FLOW {
	long b_prod = r->dim.prod();
	if (!b_prod) return; /* nothing to do... and avoid deadly divisions by zero */
	if (b_prod > 4) {
		T buf[b_prod];
		while (n) {
			for (long j=0; j<b_prod; j++) buf[j] = *data;
			op->zip(b_prod,buf,(T *)*r);
			go->send(b_prod,buf);
			data++; n--;
		}
		return;
	}
	n*=b_prod;
	T buf[n];
	T buf2[b_prod*64];
	for (int i=0; i<64; i++) COPY(buf2+i*b_prod,(T *)*r,b_prod);
	switch (b_prod) {
	#define Z buf[k++]=data[i]
	case 1:	for (long i=0,k=0; k<n; i++) {Z;} break;
	case 2: for (long i=0,k=0; k<n; i++) {Z;Z;} break;
	case 3:	for (long i=0,k=0; k<n; i++) {Z;Z;Z;} break;
	case 4:	for (long i=0,k=0; k<n; i++) {Z;Z;Z;Z;} break;
	default:for (long i=0,k=0; k<n; i++) for (int j=0; j<b_prod; j++, k++) Z;
	}
	#undef Z
	int ch=64*b_prod;
	int nn=(n/ch)*ch;
	for (int j=0; j<nn; j+=ch) op->zip(ch,buf+j,buf2);
	op->zip(n-nn,buf+nn,buf2);
	go->send(n,buf);
} GRID_END

GRID_INPUT(1,r) {} GRID_END

\end class {install("#outer",2,1); add_creator("@outer");}

//****************************************************************
//{ Dim[]<T>,Dim[]<T>,Dim[]<T> -> Dim[A]<T> } or
//{ Dim[B]<T>,Dim[B]<T>,Dim[B]<T> -> Dim[*As,B]<T> }
\class GridFor {
	\attr P<Grid> from;
	\attr P<Grid> to  ;
	\attr P<Grid> step;
	\constructor (Grid *from, Grid *to, Grid *step=0) {
		this->from.but(expect_max_one_dim);
		this->to  .but(expect_max_one_dim);
		this->step.but(expect_max_one_dim);
		this->from=from;
		this->to  =to  ;
		this->step=step;
	}
	//\decl 0 set (Grid *l=0) {from=l;}
	\decl 0 set (...) {from = Grid::from_msg(argc,argv);}
	\decl 0 bang () {
		SAME_TYPE(*from,to);
		if (step) {SAME_TYPE(*from,step);}
		if (from->dim!=to->dim || (step && to->dim!=step->dim))
			RAISE("dimension mismatch: from:%s to:%s step:%s", from->dim.to_s(), to->dim.to_s(),
				step ? step->dim.to_s() : "default");
		#define FOO(T) trigger((T)0);
		TYPESWITCH(from->nt,FOO,);
		#undef FOO
	}
	\grin 0
	\grin 1
	\grin 2
	template <class T> void trigger (T bogus) {
		int n = from->dim.prod();
		int32 nn[n+1];
		T x[64*n];
		T *fromb = (T *)*from;
		T *  tob = (T *)*to  ;
		T *stepb = step ? (T *)*step : (T *)alloca(n*sizeof(T *)); if (!step) for (int i=0; i<n; i++) stepb[i]=1;

		for (int i=n-1; i>=0; i--) if (!stepb[i]) RAISE("step must not contain zeroes");
		for (int i=0; i<n; i++) {
			nn[i] = (tob[i] - fromb[i] + stepb[i] - cmp(stepb[i],(T)0)) / stepb[i];
			if (nn[i]<0) nn[i]=0;
		}
		Dim d;
		if (from->dim.n==0) {d = Dim(*nn);}
		else {nn[n]=n;       d = Dim(n+1,nn);}
		int total = d.prod();
		go=new GridOut(this,0,d,from->nt);
		if (total==0) return;
		int k=0;
		for(int d=0;;d++) {
			// here d is the dim# to reset; d=n for none
			for(;d<n;d++) x[k+d]=fromb[d];
			if (k==63*n) {go->send(k+n,x); COPY(x    ,x+k,n); k=0;}
			else         {                 COPY(x+k+n,x+k,n); k+=n;}
			d--;
			// here d is the dim# to increment
			for(;;d--) {
				if (d<0) goto end;
				x[k+d]+=stepb[d];
				if (stepb[d]>0 ? x[k+d]<tob[d] : x[k+d]>tob[d]) break;
			}
		}
		end: if (k) go->send(k,x);
	}
};
GRID_INPUT(2,step) {} GRID_END
GRID_INPUT(1,to) {} GRID_END
GRID_INPUT(0,from) {_0_bang();} GRID_END
\end class {install("#for",3,1); add_creator("@for");}

//****************************************************************
\class GridFinished {
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {} GRID_FINISH {out[0]();} GRID_END
\end class {install("#finished",1,1); add_creator("@finished");}
\class GridDim {
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {
	GridOut go(this,0,Dim(in.dim.n));
	go.send(in.dim.n,in.dim.v);
} GRID_END
\end class {install("#dim",1,1); add_creator("@dim");}
\class GridType {
	\attr bool abbr;
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {
	if (abbr) out[0](gensym(const_cast<char *>(number_type_table[in.nt].alias)));
	else      out[0](gensym(const_cast<char *>(number_type_table[in.nt].name)));
} GRID_END
\end class {install("#type",1,1); add_creator("@type");}

//****************************************************************
//{ Dim[*As]<T>,Dim[B] -> Dim[*Cs]<T> }
\class GridRedim {
	/*\attr*/ Dim dim;
	P<Grid> dim_grid;
	P<Grid> temp; // temp->dim != dim
	~GridRedim() {}
	\constructor (Grid *d=0) {
		dim_grid.but(expect_dim_dim_list);
		dim_grid=d?d:new Grid(Dim(0));
		dim = dim_grid->to_dim();
	}
	\grin 0
	\grin 1 int32
};

GRID_INLET(0) {
	long a=in.dim.prod(), b=dim.prod();
	if (a<b) temp=new Grid(Dim(a),in.nt);
	go=new GridOut(this,0,dim,in.nt);
} GRID_FLOW {
	long i = in.dex;
	if (!temp) {long n2 = min(n,   dim.prod()-i);                             if (n2>0) go->send(n2,data);}
	else       {long n2 = min(n,in.dim.prod()-i); COPY((T *)*temp+i,data,n2); if (n2>0) go->send(n2,data);}
} GRID_FINISH {
	if (!!temp) {
		long a = in.dim.prod(), b = dim.prod();
		if (a) {
			for (long i=a; i<b; i+=a) go->send(min(a,b-i),(T *)*temp);
		} else {
			T foo[1]={0}; for (long i=0; i<b; i++) go->send(1,foo);
		}
	}
	temp=0;
} GRID_END

GRID_INPUT(1,dim_grid) {
	Dim d = dim_grid->to_dim();
	dim = d;
} GRID_END

\end class {install("#redim",2,1); add_creator("@redim");}

#define OP(x) dynamic_cast<Numop2 *>(op_dict[string(#x)])
void startup_classes1 () {
	op_add = OP(+);
	op_sub = OP(-);
	op_mul = OP(*);
	op_shl = OP(<<);
	op_mod = OP(%);
	op_and = OP(&);
	op_div = OP(/);
	op_put = OP(put);
	\startall
}
