/*
	GridFlow
	Copyright (c) 2001-2011 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
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

typedef t_atom2 Atom;

bool operator < (const Atom &a, const Atom &b) {
	if (a.a_type!=b.a_type) return a.a_type<b.a_type;
	return uintptr_t(a.a_symbol)<uintptr_t(b.a_symbol);
}
	
map<Atom, int> priorities;

#define a_pointer a_w.w_gpointer

\class GFExpr {
	/* only between next() and parse() */
	#define A_OPEN    t_atomtype(0x1000) // '('
	#define A_CLOSE   t_atomtype(0x1001) // ')'
	#define A_SQOPEN  t_atomtype(0x1008) // '['
	#define A_SQCLOSE t_atomtype(0x1009) // ']'
	/* used for bytecode */
	#define A_VAR      t_atomtype(0x1002) /* for $f1-style variables, not other variables */
	#define A_OP1      t_atomtype(0x1003) /* unary prefix operator or unary function */
	#define A_OP       t_atomtype(0x1004) /* operator: binary infix, or not parsed yet */
	#define A_VAR_A    t_atomtype(0x1006) /* [tabread] */
	#define A_IF	   t_atomtype(0x1007) /* if(,,) */

	#define A_OP1FAST  t_atomtype(0x1013) /* direct Numop1 * */
	#define A_OPFAST   t_atomtype(0x1014) /* direct Numop2 * */

	/* used for both */
        //      A_SYMBOL for [v] names and [table] names; also used between next() and parse() for function names.
        //      A_FLOAT  for float literals (unlike [expr], there are no integer literals)
        /* end */
	string args;
	//vector<Atom> toks;
	vector<Atom> code;
	vector<Atom> inputs;
	Atom tok;   // which token was last produced by next()
	const char *s; // an iterator through the args variable
	void next () {
		while (*s && isspace(*s)) s++;
		if (!*s) tok.a_type=A_NULL;
		else if (isdigit(*s) || *s=='.') {char *e; tok = strtof(s,&e); s=(const char *)e;}
		else if (strchr("+-*/%&|^<>=!~",*s)) {
			char t[3]={0,0,0};
			t[0]=*s++;
			if (*s==s[-1] || *s=='=') t[1]=*s++;
			tok = Atom(A_OP,gensym(t));
		}
		else if (*s=='(') {s++; tok.a_type=A_OPEN;}
		else if (*s==')') {s++; tok.a_type=A_CLOSE;}
		else if (*s=='[') {s++; tok.a_type=A_SQOPEN;}
		else if (*s==']') {s++; tok.a_type=A_SQCLOSE;}
		else if (*s==';') {s++; tok.a_type=A_SEMI;}
		else if (*s==',') {s++; tok.a_type=A_COMMA;}
		else if (*s=='$') {
			if (s[1]=='f') {}
			else RAISE("$ type must be f");
			char *e; long i = strtol(s+2,&e,10); s=(const char *)e;
			if (i<1) RAISE("$ var index must be greater than 0");
			if (i>32) RAISE("$ var index is bigger than 32, which is probably a mistake");
			tok = Atom(A_VAR,int(i-1+256*'f'));
			if (int(inputs.size())<i) {inputs.resize(i); ninlets_set(i);}
		}
		else if (isalpha(*s)) {
			char *e=(char *)s; while (isalnum(*e)) e++;
			tok = symprintf("%.*s",e-s,s);
			s=e;
		}
		else RAISE("syntax error at character '%c'",*s);
		//toks.push_back(tok);
	}
	void add (const Atom &a) {
		if (a.a_type==A_SEMI) {noutlets_set(noutlets+1); return;}
		if (a.a_type==A_OP1 && a.a_symbol==gensym("~")) {
			code.push_back(-1);
			code.push_back(Atom(A_OP,gensym("^")));
		} else code.push_back(a);
	}
	// context: 0=outside, 1=plain parens; 2=func parens; 3=brackets
	int parse (int context, Atom prevop=Atom(A_NULL,0)) {
		int commas=0;
	  next:	next();
		switch (int(tok.a_type)) {
		  case A_OP: { // unary
			t_symbol *o = tok.a_symbol;
			if (o==gensym("!") || o==gensym("~"))   {prevop=Atom(A_OP1,o); goto next;}
			else if (o==gensym("-")) {prevop=Atom(A_OP1,gensym("unary-")); goto next;}
			else if (o==gensym("+")) {prevop=Atom(A_OP1,gensym("unary+")); goto next;}
			else RAISE("can't use '%s' as a unary prefix operator",tok.a_symbol->s_name);
		  } break;
		  case A_OPEN: {parse(1); goto infix;}
		  case A_FLOAT: case A_SYMBOL: case A_VAR:
			add(tok);
		  infix: // this section could become another method
			next();
			switch (int(tok.a_type)) {
			  case A_OP: { // binary
				int priority1 = prevop.a_type!=A_NULL ? priorities[prevop] : 42;
				int priority2 =                         priorities[tok];
				if (!priority2) RAISE("unknown operator '%s'",tok.a_symbol->s_name);
				if (priority1 <= priority2) {add(prevop); prevop=tok; goto next;}
				else {commas+=parse(context,tok); if (prevop.a_type!=A_NULL) add(prevop);}
			  } break;
			  case A_OPEN: { // function (1 or 2 args)
				Atom a = code.back(); code.pop_back();
				if (a.a_type!=A_SYMBOL) {
					string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
					RAISE("syntax error (c) tok=%s type=%s",z.data(),zt.data());
				}
				t_symbol *o = a.a_symbol; int e = o==gensym("if") ? 3 : TO(Numop *,a)->arity();
				int n = parse(2)+1;
				if (n!=e) RAISE("wrong number of arguments for '%s': got %d instead of %d",o->s_name,n,e);
				code.push_back(Atom(e==1?A_OP1:e==2?A_OP:e==3?A_IF:A_CANT,o));
				goto infix;
			  } break;
			  case A_CLOSE: {
				if (int(tok.a_type)==A_CLOSE && context!=1 && context!=2) RAISE("can't close a parenthesis at this point");
				if (prevop.a_type!=A_NULL) add(prevop);
			  } break;
			  case A_NULL: case A_SEMI: {
				if (context!=0) RAISE("unexpected end of expression: context=%d",context);
				if (prevop.a_type!=A_NULL) add(prevop);
				if (tok.a_type==A_SEMI) {add(tok); /*int elems=*/parse(0); /*post("A_SEMI: elems=%d",elems);*/}
			  } break;
			  case A_SQOPEN: {
				Atom a = code.back();
				if (a.a_type!=A_SYMBOL) {
					string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
					RAISE("syntax error (d) tok=%s type=%s",z.data(),zt.data());
				}
				parse(3);
				code.push_back(Atom(A_VAR_A,0));
			  }  break;
			  case A_SQCLOSE: {
				if (context!=3) RAISE("can't close a bracket at this point");
			  }  break;
			  case A_COMMA: {
			  	if (context!=2) RAISE("can't use comma in this context");
			  	if (prevop.a_type!=A_NULL) add(prevop);
				commas++; prevop=Atom(A_NULL,0); goto next;
			  }
			  default: {
				string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
				RAISE("syntax error (b) tok=%s type=%s",z.data(),zt.data());
			  }
			}
		  break;
		  default: {
			  string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
			  RAISE("syntax error (a) tok=%s type=%s",z.data(),zt.data());
		  }
		};
		return commas;
	}
	void prelookup () {
		for (size_t i=0; i<code.size(); i++) {
			if (code[i].a_type==A_OP)  code[i]=Atom(A_OPFAST ,(t_gpointer *)op_dict[code[i].a_symbol]);
			if (code[i].a_type==A_OP1) code[i]=Atom(A_OP1FAST,(t_gpointer *)op_dict[code[i].a_symbol]);
		}
	}
	\constructor (...) {
		//post("----------------------------------------------------------------");
		//toks.clear(); code.clear();
		if (argc) args = join(argc,argv); else args = "0";
		s = args.data();
		parse(0);
		prelookup();
		for (size_t i=0; i<inputs.size(); i++) inputs[i]=0;
		//try {parse(s);} // should use fclasses_pd[pd_class(x)]->name->s_name
		//catch (Barf &oozy) {oozy.error(gensym("#expr"),argc,argv);}
		if (*s) RAISE("expression not finished parsing");
	}
	Atom lookup (const Atom &a) {
		if (a.a_type!=A_SYMBOL) return a;
		float f; t_symbol *s=a;
		if (value_getfloat(s,&f)) RAISE("unknown variable '%s'",s->s_name);
		return f;//stack.push_back(f);
	}
	\decl 0 bang () {
		//post("----------------------------------------------------------------");
		//string t = join(toks.size(),toks.data()); post("#expr toks: %s",t.data());
		//string c = join(code.size(),code.data()); post("#expr code: %s",c.data());
		Atom stack[32]; int m=0; // I'm not checking for overflow. Beware !
		int n = code.size();
		for (int i=0; i<n; i++) {
			//{string z = code[i].to_s(); post("interpreting %s",z.data());}
			switch (int(code[i].a_type)) {
			  case A_FLOAT: case A_SYMBOL: stack[m++]=code[i]; break;
			  case A_VAR_A: {
				int i = lookup(stack[--m]);
				t_symbol *t = stack[m-1];
				t_garray *a = (t_garray *)pd_findbyclass(t, garray_class); if (!a) RAISE("%s: no such array", t->s_name);
				int npoints; t_word *vec;
				if (!garray_getfloatwords(a, &npoints, &vec)) RAISE("%s: bad template for tabread", t->s_name);
				stack[m-1] = npoints ? vec[clip(i,0,npoints-1)].w_float : 0;
			  } break;
			  case A_VAR: {
				stack[m++] = inputs[code[i].a_index & 255];
			  } break;
#if 0
			  case A_OP: {
				Numop2 *op = (Numop2 *)op_dict[code[i].a_symbol]; //TO(Numop2 *,Atom(code[i].a_symbol->s_name));
				float b = lookup(stack[--m]);
				float a = lookup(stack[m-1]);
				op->map(1,&a,b);
				stack[m-1] = a;
			  } break;
			  case A_OP1: {
				Numop1 *op = (Numop1 *)op_dict[code[i].a_symbol]; //TO(Numop1 *,Atom(code[i].a_symbol->s_name));
				float a = lookup(stack[m-1]);
				op->map(1,&a);
				stack[m-1] = a;
			  } break;
#endif
			  case A_OPFAST: {
				Numop2 *op = (Numop2 *)code[i].a_pointer;
				float b = lookup(stack[--m]);
				float a = lookup(stack[m-1]);
				op->map(1,&a,b);
				stack[m-1] = a;
			  } break;
			  case A_OP1FAST: {
				Numop1 *op = (Numop1 *)code[i].a_pointer;
				float a = lookup(stack[m-1]);
				op->map(1,&a);
				stack[m-1] = a;
			  } break;
			  case A_IF: {
				float c = lookup(stack[--m]);
				float b = lookup(stack[--m]);
				float a = lookup(stack[m-1]);
				stack[m-1] = a ? b : c;
			  } break;
			  default: {
				string z = code[i].to_s();
				RAISE("can't interpret %s (atomtype %s)",z.data(),atomtype_to_s(code[i].a_type).data());
			  }
			}
		}
		for (int i=noutlets-1; i>=0; i--) {
			if (m) out[i](stack[--m]); else RAISE("no result");
		}
	}
	\decl n float (int winlet, float f) {
		if (winlet>=int(inputs.size())) RAISE("$f%d does not exist here",winlet+1);
		inputs[winlet] = f;
		if (!winlet) _0_bang();
	}
	\decl 0 float (float f) {inputs[0]=f; _0_bang();} // faster than the n float wildcard ?
	//\decl 1 float (float f) {inputs[1]=f;} // I'd put more like this, but it slows down the lookup. should investigate this in BFObject_anything.
	//etc
};
\end class {install("#expr",1,1,CLASS_NOPARENS|CLASS_NOCOMMA);}

void startup_classes4 () {
	#define PR1(SYM) priorities[Atom(A_OP1,gensym(#SYM))]
	#define PR(SYM)  priorities[Atom(A_OP ,gensym(#SYM))]
	// all functions() are supposed to have priority 2
	PR1(+) = PR1(unary+) = PR1(unary-) = PR1(~) = PR1(!) = 3; // all unaries are supposed to have priority 3
	PR(*) = PR(/) = PR(%) = 5;
	PR(+) = PR(-) = 6;
	PR(<<) = PR(>>) = 7;
	PR(<) = PR(>) = PR(<=) = PR(>=) = PR(==) = PR(!=) = 8;
	PR(&) = 10;
	PR(^) = 11;
	PR(|) = 12;
	PR(&&) = 13;
	PR(||) = 14;
	\startall
}
