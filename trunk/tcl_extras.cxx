#include "tcl_extras.h"
//#include <map>
#include <ext/hash_map>
#include <string>
using namespace std;
using namespace __gnu_cxx; /* hash_map is not standard */
extern Tcl_Interp *tcl_for_pd;
void poststring2 (const char *s) {post("%s",s);}
//map<string,t_class*> class_table;
hash_map<string,t_class*> class_table;
static long cereal=0;

static Tcl_Obj *pd_to_tcl (t_atom *a) {
  switch (a->a_type) {
    case A_FLOAT: return Tcl_NewDoubleObj(a->a_w.w_float);
    case A_SYMBOL: {
      char *s = a->a_w.w_symbol->s_name;
      return Tcl_NewStringObj(s,strlen(s));
    }
    case A_POINTER: {
      char s[32];
      sprintf(s,"<Pointer:%08lx>",(long)a->a_w.w_gpointer);
      return Tcl_NewStringObj(s,strlen(s));
    }
    default: {
      char *s = "<Unknown>";
      return Tcl_NewStringObj(s,strlen(s));
    }
  }
}

static void tcl_to_pd (Tcl_Obj *a, t_atom *b) {
  double d;
  if (Tcl_GetDoubleFromObj(tcl_for_pd,a,&d) == TCL_OK) {SETFLOAT(b,d); return;}
  SETSYMBOL(b,gensym(Tcl_GetStringFromObj(a,0)));
}

static void *tclpd_init (t_symbol *classsym, int ac, t_atom *at) {
  const char *name = classsym->s_name;
  t_class *qlass = class_table[string(name)];
  post("tclpd_init called for class %s (%p) with %d arguments",name,qlass,ac);
  t_tcl *self = (t_tcl *)pd_new(qlass);
  char s[32];
  sprintf(s,"pd%06lx",cereal++);
  self->self = Tcl_NewStringObj(s,strlen(s));
  Tcl_IncrRefCount(self->self);
  Tcl_Obj *av[ac+2];
  av[0] = Tcl_NewStringObj(name,strlen(name));
  av[1] = self->self;
  for (int i=0; i<ac; i++) av[2+i] = pd_to_tcl(&at[i]);
  if (Tcl_EvalObjv(tcl_for_pd,ac+2,av,0) != TCL_OK) {
    post("tcl error: %s\n", Tcl_GetString(Tcl_GetObjResult(tcl_for_pd)));
    pd_free((t_pd *)self);
    return 0;
  }
  return self;
}

static void tclpd_anything (t_tcl *self, t_symbol *s, int ac, t_atom *at) {
  post("tclpd_anything called for object %s, selector %s, with %d arguments",
    Tcl_GetString(self->self),s->s_name,ac);
  Tcl_Obj *av[ac+2];
  av[0] = self->self;
//  av[1] = Tcl_NewStringObj(s->s_name,strlen(s->s_name));
  av[1] = Tcl_NewIntObj(0);
  Tcl_AppendToObj(av[1],"_",1);
  Tcl_AppendToObj(av[1],s->s_name,strlen(s->s_name));
  for (int i=0; i<ac; i++) av[2+i] = pd_to_tcl(&at[i]);
  if (Tcl_EvalObjv(tcl_for_pd,ac+2,av,0) != TCL_OK)
    post("tcl error: %s\n", Tcl_GetString(Tcl_GetObjResult(tcl_for_pd)));
}

static void tclpd_free (t_tcl *self) {
  post("tclpd_free called");
}

t_class *tclpd_class_new (char *name, int flags) {
  t_class *qlass = class_new(gensym(name), (t_newmethod) tclpd_init,
    (t_method) tclpd_free, sizeof(t_tcl), flags, A_GIMME, A_NULL);
  //post("tclpd_class_new called for class %s (%p)",name,qlass);
  class_table[string(name)] = qlass;
  class_addanything(qlass,tclpd_anything);
  return qlass;
}
