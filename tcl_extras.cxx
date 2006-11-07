#include "tcl_extras.h"
#include <map>
#include <string>

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
  self->self = (char *)malloc(16);
  sprintf(self->self,"pd%06x",cereal++);
  return self;
}

static void tclpd_free (t_tcl *self) {
  post("tclpd_free called");
}

t_class *tclpd_class_new (char *name, int flags) {
  t_class *qlass = class_new(gensym(name), (t_newmethod) tclpd_init,
    (t_method) tclpd_free, sizeof(t_tcl), flags, A_GIMME, A_NULL);
  post("tclpd_class_new called for class %s (%p)",name,qlass);
  class_table[string(name)] = qlass;
  return qlass;
}

