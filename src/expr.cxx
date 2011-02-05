/*
	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

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

bool operator < (const t_atom2 &a, const t_atom2 &b) {
	if (a.a_type!=b.a_type) return a.a_type<b.a_type;
	return uintptr_t(a.a_symbol)<uintptr_t(b.a_symbol);
}
	
map<t_atom2, int> priorities;

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
	/* used for both */
        //      A_SYMBOL for [v] names and [table] names; also used between next() and parse() for function names.
        //      A_FLOAT  for float literals (unlike [expr], there are no integer literals)
        /* end */
	string args;
	//vector<t_atom2> toks;
	vector<t_atom2> code;
	vector<t_atom2> inputs;
	t_atom2 tok;   // which token was last produced by next()
	const char *s; // an iterator through the args variable
	void next () {
		while (*s && isspace(*s)) s++;
		if (!*s) tok.a_type=A_NULL;
		else if (isdigit(*s) || *s=='.') {char *e; tok = strtof(s,&e); s=(const char *)e;}
		else if (strchr("+-*/%&|^<>=!~",*s)) {
			char t[3]={0,0,0};
			t[0]=*s++;
			if (*s==s[-1] || *s=='=') t[1]=*s++;
			tok = t_atom2(A_OP,gensym(t));
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
			tok = t_atom2(A_VAR,int(i-1+256*'f'));
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
	void add (const t_atom2 &a) {
		if (a.a_type==A_SEMI) {noutlets_set(noutlets+1); return;}
		if (a.a_type==A_OP1 && a.a_symbol==gensym("~")) {
			code.push_back(-1);
			code.push_back(t_atom2(A_OP,gensym("^")));
		} else code.push_back(a);
	}
	// context: 0=outside, 1=plain parens; 2=func parens; 3=brackets
	int parse (int context, t_atom2 prevop=t_atom2(A_NULL,0)) {
           	next();
		switch (int(tok.a_type)) {
		  case A_OP: { // unary
			t_symbol *o = tok.a_symbol;
			if (o==gensym("+") || o==gensym("!") || o==gensym("~")) {parse(context,t_atom2(A_OP1,o));}
			else if (o==gensym("-")) {parse(context,t_atom2(A_OP1,gensym("unary-")));}
			else RAISE("can't use '%s' as a unary prefix operator",tok.a_symbol->s_name);
		  } break;
		  case A_FLOAT: case A_SYMBOL: case A_VAR:
			add(tok);
		  infix: // this section could become another method
			next();
			switch (int(tok.a_type)) {
			  case A_OP: { // binary
				int priority1 = prevop.a_type!=A_NULL ? priorities[prevop] : 42;
				int priority2 =                         priorities[tok];
				if (!priority2) RAISE("unknown operator '%s'",tok.a_symbol->s_name);
				if (priority1 <= priority2) {add(prevop); parse(context,tok);}
				else {parse(context,tok); if (prevop.a_type!=A_NULL) add(prevop);}
			  } break;
			  case A_OPEN: { // function (1 or 2 args)
				t_atom2 a = code.back(); code.pop_back();
				if (a.a_type!=A_SYMBOL) {
					string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
					RAISE("syntax error (c) tok=%s type=%s",z.data(),zt.data());
				}
				t_symbol *o = a.a_symbol; int e = o==gensym("if") ? 3 : TO(Numop *,a)->arity();
				if (parse(2)!=e) RAISE("wrong number of arguments for '%s'",o->s_name);
				code.push_back(t_atom2(e==1?A_OP1:e==2?A_OP:e==3?A_IF:A_CANT,o));
			  } break;
			  case A_CLOSE: {
				if (int(tok.a_type)==A_CLOSE && context!=1 && context!=2) RAISE("can't close a parenthesis at this point");
				if (prevop.a_type!=A_NULL) add(prevop);
			  } break;
			  case A_NULL: case A_SEMI: {
				if (context!=0) RAISE("unexpected end of expression");
				if (prevop.a_type!=A_NULL) add(prevop);
				if (tok.a_type==A_SEMI) {add(tok); int elems=parse(0); post("A_SEMI: elems=%d",elems);}
			  } break;
			  case A_SQOPEN: {
				t_atom2 a = code.back();
				if (a.a_type!=A_SYMBOL) {
					string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
					RAISE("syntax error (d) tok=%s type=%s",z.data(),zt.data());
				}
				parse(3);
				code.push_back(t_atom2(A_VAR_A,0));
			  }  break;
			  case A_SQCLOSE: {
				if (context!=3) RAISE("can't close a bracket at this point");
			  }  break;
			  case A_COMMA: {
			  	if (context!=2) RAISE("can't use comma in this context");
				return 1+parse(context);
			  }
			  default: {
				string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
				RAISE("syntax error (b) tok=%s type=%s",z.data(),zt.data());
			  }
			}
		  break;
		  case A_OPEN: {parse(1); goto infix;}
		  default: {
			  string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
			  RAISE("syntax error (a) tok=%s type=%s",z.data(),zt.data());
		  }
		};
		return 1;
	}
	\constructor (...) {
		//toks.clear(); code.clear();
		if (argc) args = join(argc,argv); else args = "0";
		s = args.data();
		parse(0);
		for (size_t i=0; i<inputs.size(); i++) inputs[i]=0;
		//try {parse(s);} // should use fclasses_pd[pd_class(x)]->name->s_name
		//catch (Barf &oozy) {oozy.error(gensym("#expr"),argc,argv);}
		if (*s) RAISE("expression not finished parsing");
	}
	t_atom2 lookup (const t_atom2 &a) {
		if (a.a_type!=A_SYMBOL) return a;
		float f; t_symbol *s=a;
		if (value_getfloat(s,&f)) RAISE("unknown variable '%s'",s->s_name);
		return f;//stack.push_back(f);
	}
	\decl 0 bang () {
		//post("----------------------------------------------------------------");
		//string t = join(toks.size(),toks.data()); post("#expr toks: %s",t.data());
		//string c = join(code.size(),code.data()); post("#expr code: %s",c.data());
		vector<t_atom2> stack;
		int n = code.size();
		for (int i=0; i<n; i++) {
			//{string z = code[i].to_s(); post("interpreting %s",z.data());}
			switch (int(code[i].a_type)) {
			  case A_FLOAT: case A_SYMBOL: stack.push_back(code[i]); break;
			  case A_VAR_A: {
				int i = lookup(stack.back()); stack.pop_back();
				t_symbol *t = stack.back();
				t_garray *a = (t_garray *)pd_findbyclass(t, garray_class); if (!a) RAISE("%s: no such array", t->s_name);
				int npoints; t_word *vec;
				if (!garray_getfloatwords(a, &npoints, &vec)) RAISE("%s: bad template for tabread", t->s_name);
				stack.back() = npoints ? vec[clip(i,0,npoints-1)].w_float : 0;
			  } break;
			  case A_VAR: {
				stack.push_back(inputs[code[i].a_index & 255]);
			  } break;
			  case A_OP: {
				Numop2 *op = TO(Numop2 *,t_atom2(code[i].a_symbol->s_name));
				float b = lookup(stack.back()); stack.pop_back();
				float a = lookup(stack.back());
				op->map(1,&a,b);
				stack.back() = a;
			  } break;
			  case A_OP1: {
				Numop1 *op = TO(Numop1 *,t_atom2(code[i].a_symbol->s_name));
				float a = lookup(stack.back());
				op->map(1,&a);
				stack.back() = a;
			  } break;
			  case A_IF: {
				float c = lookup(stack.back()); stack.pop_back();
				float b = lookup(stack.back()); stack.pop_back();
				float a = lookup(stack.back());
				stack.back() = a ? b : c;
			  } break;
			  default: {
				string z = code[i].to_s();
				RAISE("can't interpret %s (atomtype %s)",z.data(),atomtype_to_s(code[i].a_type).data());
			  }
			}
		}
		for (int i=noutlets-1; i>=0; i--) {
			if (stack.size()) {out[i](stack.back()); stack.pop_back();} else RAISE("no result");
		}
	}
	\decl n float (int winlet, float f) {
		if (winlet>=int(inputs.size())) RAISE("$f%d does not exist here",winlet+1);
		inputs[winlet] = f;
		if (!winlet) _0_bang();
	}
};
\end class {install("#expr",1,1,CLASS_NOPARENS|CLASS_NOCOMMA);}

void startup_classes4 () {
	#define PR1(SYM) priorities[t_atom2(A_OP1,gensym(#SYM))]
	#define PR(SYM)  priorities[t_atom2(A_OP ,gensym(#SYM))]
	// all functions() are supposed to have priority 2
	PR1(+) = PR1(unary-) = PR1(~) = PR1(!) = 3; // all unaries are supposed to have priority 3
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
