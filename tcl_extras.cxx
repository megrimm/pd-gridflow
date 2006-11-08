#include "tcl_extras.h"
#include <map>
#include <string>

extern Tcl_Interp *tcl_for_pd;

using namespace std;

void poststring2 (const char *s) { post("%s",s); }

//map<const char*,t_class*> class_table(strcmp);
map<string,t_class*> class_table;

static long cereal=0;

static void *tclpd_init (t_symbol *classsym, int ac, t_atom *at) {
  const char *name = classsym->s_name;
  t_class *qlass = class_table[string(name)];
  post("tclpd_init called for class %s (%p) with %d arguments",name,qlass,ac);
  t_tcl *self = (t_tcl *)pd_new(qlass);
  char s[32];
  sprintf(s,"pd%06x",cereal++);
  self->self = Tcl_NewStringObj(s,strlen(s));
  return self;
}

static void tclpd_anything (t_tcl *self, t_symbol *s, int ac, t_atom *at) {
  post("tclpd_anything called for object %s, selector %s, with %d arguments",
    Tcl_GetString(self->self),s->s_name,ac);
  Tcl_Obj *av[ac+2];
  av[0] = self->self;
  av[1] = Tcl_NewStringObj(s->s_name,strlen(s->s_name));
  for (int i=0,j=2; i<ac; i++,j++) {
    switch (at[i].a_type) {
      case A_FLOAT: {
        av[j] = Tcl_NewDoubleObj(at[i].a_w.w_float);
      } break;
      case A_SYMBOL: {
        char *s = at[i].a_w.w_symbol->s_name;
        av[j] = Tcl_NewStringObj(s,strlen(s));
      } break;
      case A_POINTER: {
        char s[32];
        sprintf(s,"<Pointer:%08x>",at[i].a_w.w_gpointer);
        av[j] = Tcl_NewStringObj(s,strlen(s));
      } break;
      default: {
        char *s = "<Unknown>";
        av[j] = Tcl_NewStringObj(s,strlen(s));
      } break;
    }
  }
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

