#include "m_pd.h"

typedef struct t_tcl {
  t_object o;
  char *self;
} t_tcl;

void poststring2 (const char *s);

t_class *tclpd_class_new (char *name, int flags);

