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

// FOR FUTURE USE
\class GFExpr : FObject {
	#define A_OP    t_atomtype(0x1000)
	#define A_VAR   t_atomtype(0x1001) /* for $f1-style variables, not other variables */
	#define A_OPEN  t_atomtype(0x1002) /* only between next() and parse() */
	#define A_CLOSE t_atomtype(0x1003) /* only between next() and parse() */

	vector<t_atom2> toks;
	vector<t_atom2> code;
	t_atom2 tok;
	void next (const char *&s) {
		while (*s && isspace(*s)) s++;
		if (!*s) tok.a_type=A_NULL;
		else if (isdigit(*s) || *s=='.') {char *e; tok = strtof(s,&e); s=(const char *)e;}
		else if (strchr("+-*/",*s)) {char t[2]={0,0}; *t=*s; tok.a_type=A_OP; tok.a_symbol=gensym(t); post("%c",*s); s++;}
		else if (*s=='(') {s++; tok.a_type=A_OPEN;}
		else if (*s==')') {s++; tok.a_type=A_CLOSE;}
		else if (*s==';') {s++; tok.a_type=A_SEMI;}
		else if (*s==',') {s++; tok.a_type=A_COMMA;}
		else RAISE("syntax error at character '%c'",*s);
		toks.push_back(tok);
	}
	void parse (const char *s, int level=0) { // should be storing last token for look-ahead...
		next(s);
		switch (int(tok.a_type)) {
		  case A_FLOAT: case A_SYMBOL: case A_VAR:
			code.push_back(tok);
			next(s);
			switch (int(tok.a_type)) {
				case A_OP: {t_atom2 tok2=tok; parse(s,level+1); code.push_back(tok2);} break;
				case A_NULL: return;
				default: {string z=tok.to_s(); RAISE("syntax error (%db) tok=%s",level,z.data());}
			}
		  break;
		  case A_OPEN: parse(s,level+1); break;
		  case A_CLOSE: return; // may be uncaught syntax error
		  case A_COMMA: RAISE("can't use comma there");
		  case A_SEMI:  RAISE("semi not supported");
		  case A_NULL:  return; // may be uncaught syntax error
		  default: {string z=tok.to_s(); RAISE("syntax error (%db) tok=%s",level,z.data());} // A_CANT, A_OP
		};
	}
	\constructor (...) {
		string s = join(argc,argv);
		try {parse(s.data());}
		catch (Barf &oozy) {oozy.error(gensym("gf/expr"),argc,argv);}
		string t = join(toks.size(),toks.data()); post("gf/expr toks: %s",t.data());
		string c = join(code.size(),code.data()); post("gf/expr code: %s",c.data());
	}
};
\end class {install("gf/expr",1,1);}

void startup_classes4 () {
	\startall
}