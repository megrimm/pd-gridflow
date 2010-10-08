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

#include "gridflow.hxx.fcs"
extern "C" {
#include "bundled/g_canvas.h"
#include "bundled/m_imp.h"
extern t_class *text_class;
int sys_hostfontsize(int fontsize);
};
#include <algorithm>
#include <errno.h>
#include <sys/time.h>
#include <string>
#include <fcntl.h>

typedef int (*comparator_t)(const void *, const void *);

struct _outconnect {
    struct _outconnect *next;
    t_pd *to;
};
struct _outlet {
    t_object *owner;
    struct _outlet *next;
    t_outconnect *connections;
    t_symbol *sym;
};

#define foreach(ITER,COLL) for(typeof(COLL.begin()) ITER = COLL.begin(); ITER != (COLL).end(); ITER++)

//****************************************************************

#define INIT1 BFObject *bself = (BFObject*)x; THISCLASS *self = (THISCLASS *)bself->self; self=self;
	//if (long(self)==long(0xdeadbeef)) fprintf(stderr,"dead beef at bself=%lx\n",bself);
#define INIT INIT1 t_canvas *c = glist_getcanvas(glist); c=c;
#undef L
#define L             if (0) post("%s",   __PRETTY_FUNCTION__);
#define LL(A,ARGS...) if (0) post("%s(%p,%p"A")",__FUNCTION__,bself,self,ARGS);
#define BLAH t_gobj *x, t_glist *glist
#define pd_anything pd_typedmess

#define ciGUI_FObject ciFObject
#define THISCLASS GUI_FObject
class GUI_FObject : public FObject {
public:
	bool selected,vis,use_queue;
	int sy; int sx;
	t_symbol *rsym;
	string outline;
	GUI_FObject(BFObject *bself, t_symbol *s, VA) : FObject(bself,s,argc,argv) {
		selected = false;
		vis = false;
		use_queue = true;
		rsym = symprintf("gf%08x",this);
		pd_bind((t_pd *)bself,rsym);
		outline = "#aaaaaa";
	}
	~GUI_FObject() {
		pd_unbind((t_pd *)bself,rsym);
		sys_unqueuegui(bself);
	}
	static void visfn(BLAH, int flag) {INIT LL(",%d",flag);
		self->vis = !!flag;
		if (flag) self->changed();
		else { // can't call deletefn directly
			sys_vgui(".x%lx.c delete %s\n",(long)(intptr_t)c,self->rsym->s_name);
			sys_unqueuegui(x);
		}
	}
	static void getrectfn(BLAH, int *x1, int *y1, int *x2, int *y2) {INIT
		*x1 = text_xpix(bself,glist); *x2 = *x1+self->sx;
		*y1 = text_ypix(bself,glist); *y2 = *y1+self->sy;
	}
	static void displacefn(BLAH, int dx, int dy) {INIT L
		bself->te_xpix+=dx; bself->te_ypix+=dy;
		sys_vgui(".x%lx.c move %s %d %d\n",(long)(intptr_t)glist_getcanvas(glist),self->rsym->s_name,dx,dy);
		canvas_fixlinesfor(glist, (t_text *)x);
	}
	static void selectfn(BLAH, int state) {INIT L
		self->selected=!!state;
		sys_vgui(".x%lx.c itemconfigure {%sR || %sTEXT} -outline %s\n",(long)(intptr_t)c,
			self->rsym->s_name,self->rsym->s_name,self->selected?"#0000ff":self->outline.data());
	}
	static void deletefn(BLAH) {INIT L
		/* if (self->vis) */ sys_vgui(".x%lx.c delete %s\n",(long)(intptr_t)c,self->rsym->s_name);
		canvas_deletelinesfor(glist,(t_object *)bself); // hein ?
		sys_unqueuegui(x);
	}
	static int clickfn(BLAH, int xpix, int ypix, int shift, int alt, int dbl, int doit) {INIT
		//post("click1 %d %d %d %d %d %d",xpix,ypix,shift,alt,dbl,doit);
		//glist_grab(self->mom,(t_gobj *)bself,motionfn,keyfn,xpix,ypix);
		return 0;
	}
	static void motionfn(void *x, float dx, float dy) {/*INIT1 L*/}
	static void keyfn(void *x, float key) {/*INIT1 L*/}
	static void activatefn(BLAH, int state) {INIT /* post("activate %d",state); */}
	virtual void show() = 0;
	void changed(t_symbol *foo=0) {foo=foo; if (use_queue) sys_queuegui(bself,mom,redraw); else show();}
	static void redraw(BLAH) {INIT1 self->show();}
};
#define NEWWB /* C++ doesn't have virtual static ! */ static t_widgetbehavior *newwb () { \
	t_widgetbehavior *wb = new t_widgetbehavior; \
	wb->w_getrectfn    = getrectfn;  \
	wb->w_displacefn   = displacefn; \
	wb->w_selectfn     = selectfn;   \
	wb->w_activatefn   = activatefn; \
	wb->w_deletefn     = deletefn;   \
	wb->w_visfn        = visfn;      \
	wb->w_clickfn      = clickfn;    \
	return wb;}

#undef THISCLASS

//****************************************************************

\class Display : GUI_FObject {
	int y,x;
	ostringstream text;
	t_pd *gp;
	\constructor () {
		//fprintf(stderr,"bself=%lx this=%ld\n",bself,this);
		selected=false; y=0; x=0; sy=16; sx=80; vis=false;
		pd_anything(&pd_objectmaker,gensym("#print"),0,0);
		gp = pd_newest();
		{t_atom2 a[1] = {20}; pd_anything(gp,gensym("maxrows"),1,a);}
		text << "...";
		{t_atom2 a[1] = {(t_gpointer *)bself}; pd_anything(gp,gensym("dest"),1,a);}
 		changed();
	}
	~Display () {pd_free(gp);}
	\decl 0 set_size(int sy, int sx) {this->sy=sy; this->sx=sx;}
	\decl 0 grid(...) {text.str(""); pd_typedmess(gp,s_grid,argc,argv); changed();}
	\decl 0 very_long_name_that_nobody_uses (...) { // for magic use by [#print]
		if (text.str().length()) text << "\n";
		for (int i=0; i<argc; i++) text << char(int32(argv[i]));
	}
 	void show() { /* or hide */
		ostringstream quoted;
		string ss = text.str();
		const char *s = ss.data();
		int n = ss.length();
		for (int i=0;i<n;i++) {
			if     (s[i]=='\n') quoted << "\\n";
			else if (s[i]=='{') quoted << "\\x7b";
			else if (s[i]=='}') quoted << "\\x7d";
			//else if (strchr("\\[]\"$",s[i])) quoted << "\\" << (char)s[i];
			else if (strchr("[]\"$",s[i])) quoted << "\\" << (char)s[i];
			else quoted << (char)s[i];
		}
		// used to have {Courier -12} but this changed to use pdtk_canvas_new
		if (vis) sys_vgui("display_update %s .x%lx.c %d %d #000000 #ffffc8 %s %d \"%s\"\n",
			rsym->s_name,(long)(intptr_t)glist_getcanvas(mom),text_xpix(bself,mom),text_ypix(bself,mom),
			selected?"#0000ff":"#aaaaaa", sys_hostfontsize(glist_getfont(mom)),quoted.str().data());
		else {
			t_canvas *c = glist_getcanvas(mom);
			sys_vgui(".x%lx.c delete %s\n",(long)(intptr_t)c,rsym->s_name);
		}
	}
	NEWWB
	static void redraw(BLAH) {INIT1 self->show();}
	static void selectfn(BLAH, int state) {INIT L
		self->selected=!!state;
		sys_vgui(".x%lx.c itemconfigure %sR -outline %s\n",(long)(intptr_t)c,self->rsym->s_name,self->selected?"#0000ff":"#aaaaaa");
		sys_vgui(".x%lx.c itemconfigure %sTEXT -fill %s\n",(long)(intptr_t)c,self->rsym->s_name,self->selected?"#0000ff":"#000000");
	}
	\decl void anything (...) {
		t_symbol *sel = argv[1];
		text.str("");
		if (sel==&s_float) {}
		else if (sel==&s_list && argc>=3 && argv[2].a_type==A_FLOAT) {}
		else {text << sel; if (argc>2) text << " ";}
		long nl=0;
		for (int i=2; i<argc; i++) {
			text << argv[i];
			if (i!=argc-1) {
				text << " ";
				long length = text.str().size();
				if (length-nl>64) {text << "\\\n"; nl=length;}
			}
		}
		changed();
	}
};
\end class {
	install("display",1,0);
	class_setwidget(fclass->bfclass,Display::newwb());
	sys_gui("proc display_update {self c x y fg bg outline font text} { \n"
		"$c delete $self\n"
		"pdtk_text_new $c ${self}TEXT [expr $x+2] [expr $y+4] $text $font $fg\n"
		"$c addtag $self withtag ${self}TEXT\n"
		"foreach {x1 y1 x2 y2} [$c bbox ${self}TEXT] {}\n"
		"set sx [expr $x2-$x1+2]; set x2 [expr {$x+$sx}]\n"
		"set sy [expr $y2-$y1+4]; set y2 [expr {$y+$sy}]\n"
		"$c create polygon $x $y $x2 $y $x2 [expr {$y2-2}] [expr {$x2-2}] $y2 [expr {$x+2}] $y2 $x [expr {$y2-2}]"
		" $x $y -fill $bg -tags [list $self ${self}R] -outline $outline\n"
		"$c create rectangle $x $y [expr $x+7] [expr $y+2] -fill white -tags [list $self ${self}i0] -outline black\n"
		"$c lower ${self}R ${self}TEXT\n"
		"pd \"$self set_size $sy $sx;\" \n"
	"}\n");
}

//****************************************************************

// copied from [#io.sdl] :
static t_symbol *keyboard[128];
static void KEYS_ARE (int i, const char *s__) {
	char *s_ = strdup(s__);
	char *s = s_;
	while (*s) {
		char *t = strchr(s,' ');
		if (t) *t=0;
		keyboard[i] = gensym(s);
		if (!t) break;
		s=t+1; i++;
	}
	free(s_);
}

static t_symbol *s_default;
static t_symbol *s_empty;

/* because of recursion problem with pd_unbind and bindlist_anything */
\class MouseSpyProxy : FObject {
	t_clock *clock;
	t_symbol *rcv;
	t_pd *snd;
	\constructor (t_symbol *r=s_empty) {
		snd=0;
		rcv = r==s_empty?0:r;
		if (rcv) pd_bind((t_pd *)bself,rcv);
	}
	\decl void anything (...) {if (snd) pd_anything(snd,argv[1],argc-2,argv+2);}
	void set_rcv (t_symbol *rcv_=0) {
		if (rcv) pd_unbind((t_pd *)bself,rcv);
		rcv=rcv_;
		if (rcv)   pd_bind((t_pd *)bself,rcv);
	}
	static void bye (void *x) {INIT1
		clock_free(self->clock);
		if (self->rcv) pd_unbind((t_pd *)bself,self->rcv);
		pd_free((t_pd *)x);
	}
	void delayed_free () {
		snd = 0;
		clock = clock_new(bself,(void(*)())bye);
		clock_delay(clock,0);
	}
};
\end class {install("gf/mouse_spy_proxy",1,1);}
\class MouseSpy : FObject {
	int y,x,flags;
	t_pd *snd;
	BFObject *proxy;
	\constructor (t_symbol *rcv_=s_default) {
		snd = 0;
		t_atom2 a[1] = {rcv_==s_default?symprintf(".x%x",mom):rcv_};
		pd_anything(&pd_objectmaker,gensym("gf/mouse_spy_proxy"),1,a);
		proxy = (BFObject *)pd_newest();
		((MouseSpyProxy *)proxy->self)->snd = (t_pd *)bself;
		y=x=flags=0;
	}
	void set_rcv (t_symbol *rcv_=0) {((MouseSpyProxy *)proxy->self)->set_rcv(rcv_);}
	~MouseSpy () {((MouseSpyProxy *)proxy->self)->delayed_free();}
	void event (const char *ss, t_symbol *key=0) {
		t_atom2 a[4] = {y,x,flags&511}; if (key) a[3]=key;
		t_symbol *s = gensym(ss);
		if (snd) pd_anything((t_pd *)snd,s,key?4:3,a);
		else                      out[0](s,key?4:3,a);
	}
	\decl 0 mouse   (int x_, int y_, int but, int z=0) {y=y_; x=x_; flags|=  128<<but ;          event("position");}
	\decl 0 mouseup (int x_, int y_, int but, int z=0) {y=y_; x=x_; flags&=~(128<<but);          event("position");}
	\decl 0 motion  (int x_, int y_, int flags_      ) {y=y_; x=x_; flags&=~12; flags|=flags_*2; event("position");}
	\decl 0 key     (int on, t_atom2 ascii, int drop) {
		t_symbol *key;
		if (ascii.a_type==A_SYMBOL) key = ascii;
		else {
			int i = int(ascii.a_float);
			key = (i<0 || i>=128 || !keyboard[i]) ? symprintf("%c",i) : keyboard[i];
			if (!key) key = symprintf("whoah_%d",i);
		}
		event(on ? "keypress" : "keyrelease",key);
	}
	\decl void anything (...) {}
};
\end class {
	install("gf/mouse_spy",1,1);
	s_default = gensym("default");
	s_empty   = gensym("empty");
	// copied from [#io.sdl] :
	KEYS_ARE(8,"BackSpace Tab");
	KEYS_ARE(13,"Return");
	KEYS_ARE(19,"Pause");
	KEYS_ARE(27,"Escape");
	KEYS_ARE(32,"space exclam quotedbl numbersign dollar percent ampersand apostrophe");
	KEYS_ARE(40,"parenleft parenright asterisk plus comma minus period slash");
	KEYS_ARE(48,"D0 D1 D2 D3 D4 D5 D6 D7 D8 D9");
	KEYS_ARE(58,"colon semicolon less equal greater question at");
	KEYS_ARE(91,"bracketleft backslash bracketright asciicircum underscore grave quoteleft");
	KEYS_ARE(65,"a b c d e f g h i j k l m n o p q r s t u v w x y z");
	KEYS_ARE(97,"a b c d e f g h i j k l m n o p q r s t u v w x y z");
	KEYS_ARE(127,"Delete");
}

static t_pd *seesend;
\class GridSee : GUI_FObject {
	BFObject *spy;
	P<Grid> buf;
	bool hold;
	//\attr bool fast;
	bool fast; // does not work
	\decl 0 fast (bool fast=1) {this->fast=fast;}
	t_clock *clock; // pitiééééééééééééééé
	int my1,mx1,my2,mx2; // margins
	\constructor () {
		clock = clock_new(bself,(void(*)())doh);
		my1=5; my2=5; mx1=3; mx2=3;
		compute_size();
		hold = false;
		sys_vgui("image create photo %s -width %d -height %d\n",rsym->s_name,sx,sy);
		fast=false;
		changed();
		t_atom2 a[1] = {s_empty};
		pd_anything(&pd_objectmaker,gensym("gf/mouse_spy"),1,a);
		spy = (BFObject *)pd_newest(); if (!spy) RAISE("no spy?");
		((MouseSpy *)spy->self)->snd = (t_pd *)bself;
	}
	void compute_size () { /* 1 is a fudge due to how Tk sizes work vis-à-vis pixel counting */
		sy = my1 + (buf ? buf->dim[0] : 48) + my2 - 1;
		sx = mx1 + (buf ? buf->dim[1] : 64) + mx2 - 1;
	}
	~GridSee () {
		if (spy) pd_free((t_pd *)spy);
		clock_free(clock);
	}
	// post("can=%p text_ypix=%d text_xpix=%d",can,text_ypix(bself,can),text_xpix(bself,can));
	void event (int y, int x, int flags, t_symbol *k, const char *sel) {
		t_canvas *can = mom; /* and not glist_getcanvas(mom) */
		y-=text_ypix(bself,can)+4; x-=text_xpix(bself,can)+2;
		if (hold || (y>=0 && y<sy-9 && x>=0 && x<sx-5)) {
			t_atom2 a[4] = {y,x,flags}; if (k) a[3]=k;
			hold = (flags&-256) != 0;
			out[0](gensym(sel),k?4:3,a);
		}
	}
	\decl 0 position   (int y, int x, int flags             ) {event(y,x,flags,0,"position"  );}
	\decl 0 keypress   (int y, int x, int flags, t_symbol *k) {event(y,x,flags,k,"keypress"  );}
	\decl 0 keyrelease (int y, int x, int flags, t_symbol *k) {event(y,x,flags,k,"keyrelease");}
	\decl 0 margins (int y1,        int x1,        int y2,        int x2) {
		   my1=max(0,y1); mx1=max(0,x1); my2=max(0,y2); mx2=max(0,x2); compute_size(); changed();}
	#undef FOO
	\grin 0
	void sendbuf () {
	    ostringstream os;
	    int i=0,j=0,t=0;
	    int chans = buf->dim[2];
	    int xs = buf->dim[1];
	    int ys = buf->dim[0];
	    compute_size();
	    if (fast) {
		sys_vgui("encoding system iso8859-1; image create photo %s -data \"P6\\n%d %d\\n255\\n",rsym->s_name,xs,ys);
		char fub[xs*ys*3+1]; fub[xs*ys*3]=0;
		static char tab[256];
		for (int k=0; k<256; k++) tab[k]=k;
		tab[0]=1; tab[10]=9; tab[34]=33; tab[36]=35; tab[91]=90; tab[92]=90; tab[93]=94; tab[123]=122; tab[125]=126;
		//for (int k=192; k<223; k++) tab[k]=191;
		#define FUX(i,j) fub[j]=tab[unsigned(data[i])&255];
		#define FOO(T) {T *data = (T *)*buf; \
		  if(chans>2) for(int y=0; y<ys; y++) for(int x=0; x<xs; x++,i+=chans,j+=3) {FUX(i+0,j+0)FUX(i+1,j+1)FUX(i+2,j+2)}\
		  else        for(int y=0; y<ys; y++) for(int x=0; x<xs; x++,i+=chans,j+=3) {FUX(i  ,j+0)FUX(i  ,j+1)FUX(i  ,j+2)}}
		TYPESWITCH(buf->nt,FOO,)
		#undef FOO
		sys_gui(fub);
		sys_gui("\"\n");
	    } else {
		char fub[xs*ys*6+1]; fub[xs*ys*6]=0;
		sys_gui("set zut ");
		static char tab[]="0123456789abcdef";
		#define HEX(i,j) t=unsigned(data[i]); fub[j+0]=tab[(t>>4)&15]; fub[j+1]=tab[t&15];
		#define FOO(T) {T *data = (T *)*buf; \
		  if(chans>2) for(int y=0; y<ys; y++) for(int x=0; x<xs; x++,i+=chans,j+=6) {HEX(i+0,j+0)HEX(i+1,j+2)HEX(i+2,j+4)}\
		  else        for(int y=0; y<ys; y++) for(int x=0; x<xs; x++,i+=chans,j+=6) {HEX(i  ,j+0)HEX(i  ,j+2)HEX(i  ,j+4)}}
		TYPESWITCH(buf->nt,FOO,)
		#undef FOO
		#undef HEX
		sys_gui(fub);
		sys_vgui("\nimage create photo %s -data \"P6\\n%d %d\\n255\\n[binary format H* $zut]\"\n",rsym->s_name,xs,ys);
	    }
	}
 	void show() {L
		int osx=sx, osy=sy;
		if (buf) sendbuf();
		t_glist *c = glist_getcanvas(mom);
		if (osx!=sx || osy!=sy) canvas_fixlinesfor(c,(t_object *)bself);
		sys_vgui("gridsee_update %s .x%lx.c %d %d %d %d %d %d %d %d #cccccc %s\n",rsym->s_name,(long)(intptr_t)c,
			text_xpix(bself,mom),text_ypix(bself,mom),sx,sy,mx1,my1,mx2,my2,selected?"#0000ff":"#aaaaaa");
		out[0](gensym("shown"),0,0);
	}
	static void doh (void *x) {INIT1
		MouseSpy *ms = (MouseSpy *)self->spy->self;
		ms->set_rcv(symprintf(".x%x",glist_getcanvas(self->mom)));
		//if (self->clock) {clock_free(self->clock); self->clock=0;}
	}
	static void visfn(BLAH, int flag) {INIT1
		//post("#see visfn c=%p flag=%d",glist,flag);
		GUI_FObject::visfn(x,glist,flag);//super
		clock_delay(self->clock,0);
	}
	NEWWB
};
GRID_INLET(0) {
	if (in.dim.n != 3) RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in.dim[2]<1 || in.dim[2]>4) RAISE("expecting 1 to 4 channels: y, ya, rgb, rgba (got %d)",in.dim[2]);
	if (!in.dim.prod()) RAISE("zero-sized image not supported");
	in.set_chunk(0);
	buf=new Grid(in.dim,NumberTypeE_type_of(data));
} GRID_FLOW {
	COPY((T *)*buf+in.dex,data,n);
	changed();
} GRID_END
\end class {
	install("#see",1,1);
	class_setwidget(fclass->bfclass,GridSee::newwb());
	sys_gui("proc max {a b} {if {$a>$b} {return $a} {return $b}}\n");
	sys_gui("proc GUI_FObject_update {self c x1 y1 sx sy bg outline} {\n"
	"$c delete $self\n"
	"set x2 [expr {$x1+$sx}]; set y2 [expr {$y1+$sy}]; set x7 [expr {$x1+7}]\n"
	"$c create rectangle $x1 $y1            $x2 $y2            -fill $bg   -tags [list $self ${self}R ] -outline $outline\n"
	"$c create rectangle $x1 $y1            $x7 [expr {$y1+2}] -fill white -tags [list $self ${self}i0] -outline black\n"
	"$c create rectangle $x1 [expr {$y2-2}] $x7 $y2            -fill white -tags [list $self ${self}o0] -outline black\n"
	"}\n");
	sys_gui("proc gridsee_update {self c x1 y1 sx sy mx1 my1 mx2 my2 bg outline} {\n"
	"GUI_FObject_update $self $c $x1 $y1 $sx $sy $bg $outline\n"
	"set x2 [expr {$x1+$sx}]; set y2 [expr {$y1+$sy}]\n"
	"set x3 [expr {$x1+[max 0 [expr {$mx1-1}]]}]\n"
	"set y3 [expr {$y1+[max 0 [expr {$my1-1}]]}]\n"
	"set x4 [expr {$x2-[max 0 [expr {$mx2-1}]]}]\n"
	"set y4 [expr {$y2-[max 0 [expr {$my2-1}]]}]\n"
	"$c create rectangle $x3 $y3 $x4 $y4 -fill black -tags [list $self ${self}RR] -outline black\n"
	"$c create image  [expr {$x1+$mx1}] [expr {$y1+$my1}] -tags [list $self ${self}IMAGE] -image $self -anchor nw\n"
	"}\n");
/*
 * 	sys_gui("set gridsee_socket [socket -server -port 9999]\n");
	pd_anything(&pd_objectmaker,gensym("netsend"),0,0);
	seesend = (t_pd *)pd_newest(); if (!seesend) post("no seesend?"); else post("seesend!");
	post("preconnect");
	t_atom2 a[2] = {gensym("localhost"),9999}; pd_anything((t_pd *)seesend,gensym("connect"),2,a);
	post("postconnect");
*/
}

//****************************************************************

\class GFTkButton : GUI_FObject {
	t_symbol *text;
	\constructor (t_symbol *text=0) {
		sy=32;
		sx=64;
		this->text = text?text:gensym("OK");
		outline = "#eeeeee";
	}
	\decl 0 bang () {out[0]();}
	\decl 0 size (int sy, int sx) {this->sy=sy; this->sx=sx;}
	void show () {
		//if (osx!=sx || osy!=sy) canvas_fixlinesfor(c,(t_object *)bself);
		t_glist *c = glist_getcanvas(mom);
		sys_vgui("gf/tk_button.update %s .x%lx.c %d %d %d %d #ffffff %s\n",rsym->s_name,(long)(intptr_t)c,
			text_xpix(bself,mom),text_ypix(bself,mom),sx,sy,selected?"#0000ff":outline.data());
	}
	static void visfn(BLAH, int flag) {INIT
		if (flag && !self->vis) sys_vgui("gf/tk_button.create %s .x%lx.c %d %d {%s}\n",self->rsym->s_name,(long)(intptr_t)c,
			text_xpix(bself,self->mom),text_ypix(bself,self->mom),self->text->s_name);
		if (!flag && self->vis) sys_vgui("gf/tk_button.destroy %s .x%lx.c\n",self->rsym->s_name,(long)(intptr_t)c);
		GUI_FObject::visfn(x,glist,flag);//super
	}
	NEWWB
};
\end class {
	install("gf/tk_button",1,1);
	class_setwidget(fclass->bfclass,GFTkButton::newwb());
	sys_gui("proc gf/tk_button.create {self c x1 y1 text} {\n"
	"button $c.$self -text $text -borderwidth 1 -command [list pd $self bang \\;]\n"
	"place $c.$self -x 0 -y 0\n" // bogus placement just to force [winfo] to work properly later
	"}\n");
	sys_gui("proc gf/tk_button.update {self c x1 y1 sx sy bg outline} {\n"
	"set sx2 [winfo width $c.$self]\n"
	"set sy2 [expr 5+[winfo height $c.$self]]\n"
	"if {$sx!=$sx2 || $sy!=$sy2} {pd $self size $sy2 $sx2\\;}\n"
	"set sx $sx2; set sy $sy2\n"

	//"GUI_FObject_update $self $c $x1 $y1 $sx $sy $bg $outline\n" // but without the main box
	"$c delete $self\n"
	"set x2 [expr {$x1+$sx}]; set y2 [expr {$y1+$sy}]; set x7 [expr {$x1+7}]\n"
	"$c create rectangle $x1 $y1            $x7 [expr {$y1+2}] -fill white -tags [list $self ${self}i0] -outline black\n"
	"$c create rectangle $x1 [expr {$y2-2}] $x7 $y2            -fill white -tags [list $self ${self}o0] -outline black\n"
	
	"$c create window $x1 [expr $y1+3] -window $c.$self -anchor nw -tags [list $self ${self}W]\n"
	//"$c coords ${self}W $x1 [expr $y1+3]\n"
	"}\n");
	sys_gui("proc gf/tk_button.destroy {self c} {\n"
	"destroy $c.$self\n"
	"}\n");
}

void startup_classes_gui () {
	\startall
}
