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

// FOR FUTURE USE
\class GFExpr : FObject {
	#define A_OPEN  t_atomtype(0x1000) /* only between next() and parse() */
	#define A_CLOSE t_atomtype(0x1001) /* only between next() and parse() */
	#define A_VAR   t_atomtype(0x1002) /* for $f1-style variables, not other variables */
	#define A_OP1   t_atomtype(0x1003) /* operator: unary prefix */
	#define A_OP    t_atomtype(0x1004) /* operator: binary infix, or not parsed yet */

	string args;
	vector<t_atom2> toks;
	vector<t_atom2> code;
	t_atom2 tok;   // which token was last produced by next()
	int prev;      // look-ahead is done by next() followed by prev++;
	const char *s; // an iterator through the args variable
	void next (const char *&s) {
		if (prev) {tok=toks[toks.size()-prev]; prev--; return;}
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
		else if (*s==';') {s++; tok.a_type=A_SEMI;}
		else if (*s==',') {s++; tok.a_type=A_COMMA;}
		else if (*s=='$') {
			if (s[1]=='f') {}
			else RAISE("$ type must be f");
			char *e; long i = strtol(s+2,&e,10); s=(const char *)e;
			if (i<1) RAISE("$ var index must be greater than 0");
			if (i>32) RAISE("$ var index is bigger than 32, which is probably a mistake");
			tok = t_atom2(A_VAR,int(i+256*'f'));
		}
		else RAISE("syntax error at character '%c'",*s);
		toks.push_back(tok);
	}
	void add (const t_atom2 &a) {
		if (a.a_type==A_SEMI) {noutlets_set(noutlets+1); return;}
		if (a.a_type==A_OP1 && a.a_symbol==gensym("~")) {
			code.push_back(-1);
			code.push_back(t_atom2(A_OP,gensym("^")));
		} else code.push_back(a);
	}
	void parse (int level=0, t_atom2 prevop=t_atom2(A_NULL,0)) {
		//post("%*sbegin (prevop=%s)",level*2,"",prevop?prevop->s_name:"(null)");
           	next(s);
           top:
		switch (int(tok.a_type)) {
		  case A_OP: {
			t_symbol *o = tok.a_symbol;
			if      (o==gensym("+")) {parse(level,t_atom2(A_OP1,tok.a_symbol));}
			else if (o==gensym("-")) {parse(level,t_atom2(A_OP1,tok.a_symbol));}
			else if (o==gensym("!")) {parse(level,t_atom2(A_OP1,gensym("==")));}
			else if (o==gensym("~")) {parse(level,t_atom2(A_OP1,tok.a_symbol));}
			else RAISE("can't use '%s' as a unary prefix operator",tok.a_symbol->s_name);
		  } break;
		  case A_FLOAT: case A_SYMBOL: case A_VAR:
			add(tok);
		  infix: // this section could become another method
			next(s);
			switch (int(tok.a_type)) {
				case A_OP: {
					int priority1 = prevop.a_type!=A_NULL ? priorities[prevop] : 42;
					int priority2 =                       priorities[tok];
					if (!priority2) RAISE("unknown operator '%s'",tok.a_symbol->s_name);
					//post("priorities %d %d",priority1,priority2);
					if (priority1 <= priority2) {
						add(prevop);
						parse(level,tok);
					} else {
						parse(level,tok);
						if (prevop.a_type!=A_NULL) add(prevop);
					}
				} break;
				case A_CLOSE: case A_NULL: case A_SEMI: {
					if (level && int(tok.a_type)!=A_CLOSE) RAISE("missing ')' %d times",level);
					if (prevop.a_type!=A_NULL) add(prevop);
					if (tok.a_type==A_SEMI) {add(tok); parse(level);}
					break;
				}
				default: {
					string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
					RAISE("syntax error (%db) tok=%s type=%s",level,z.data(),zt.data());
				}
			}
		  break;
		  case A_OPEN: {parse(level+1); goto infix;}
		  default: {
			  string z=tok.to_s(), zt=atomtype_to_s(tok.a_type);
			  RAISE("syntax error (%da) tok=%s",level,z.data(),zt.data());
		  }
		};
		//post("%*send (prevop=%s)",level*2,"",prevop?prevop->s_name:"(null)");
	}
	\constructor (...) {
		prev=0; //toks.clear(); code.clear();
		args = join(argc,argv);
		s = args.data();
		parse();
		//try {parse(s);} // should use fclasses_pd[pd_class(x)]->name->s_name
		//catch (Barf &oozy) {oozy.error(gensym("#expr"),argc,argv);}
		if (*s) RAISE("expression not finished parsing");
	}
	\decl 0 bang () {
		//post("----------------------------------------------------------------");
		//string t = join(toks.size(),toks.data()); post("#expr toks: %s",t.data());
		//string c = join(code.size(),code.data()); post("#expr code: %s",c.data());
		vector<float> stack;
		int n = code.size();
		for (int i=0; i<n; i++) {
			{string z = code[i].to_s(); post("interpreting %s",z.data());}
			switch (int(code[i].a_type)) {
			  case A_FLOAT: stack.push_back(code[i]); break;
			  case A_OP: {
				  Numop *op = TO(Numop *,t_atom2(code[i].a_symbol->s_name));
				  float b = stack.back(); stack.pop_back();
				  float a = stack.back(); stack.pop_back();
				  op->map(1,&a,b);
				  stack.push_back(a);
			  } break;
			  case A_OP1: {
				  Numop *op = TO(Numop *,t_atom2(code[i].a_symbol->s_name));
				  float b = stack.back(); stack.pop_back();
				  float a = 0;
				  op->map(1,&a,b);
				  stack.push_back(a);
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
};
\end class {install("#expr",1,1,CLASS_NOPARENS);}

void startup_classes4 () {
	#define PR1(SYM) priorities[t_atom2(A_OP1,gensym(#SYM))]
	#define PR(SYM)  priorities[t_atom2(A_OP ,gensym(#SYM))]
	PR1(+) = PR1(-) = PR1(~) = PR1(==) = 3; // unary "==" is "!"
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
