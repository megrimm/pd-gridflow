
#include "../gridflow.h.fcs"

\class GridExpr : FObject {
	\constructor (...) {
		std::ostringstream os;
		for (int i=0; i<argc; i++) os << " " << argv[i];
		string s = os.str();
		post("expr = '%s'",s.data());
	}
};
\end class {install("#expr",1,1);}

void startup_flow_objects2 () {
	\startall
}
