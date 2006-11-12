%module tclpd

/* functions that are in m_pd.h but don't exist in modern versions of pd */
%ignore pd_getfilename;
%ignore pd_getdirname;
%ignore pd_anything;
%ignore class_parentwidget;
%ignore sys_isreadablefile;
%ignore garray_get;
%ignore c_extern;
%ignore c_addmess;

/* functions that are only in Miller's pd, not in devel_0_39/DesireData */
%ignore sys_idlehook;

/* functions that we can't auto-wrap, because they have varargs */
%ignore post;
%ignore class_new;

/* end of ignore-list */

%include "m_pd.h"
%include "tcl_extras.h"

%{
#include "m_pd.h"
#include "tcl_extras.h"

#include <unistd.h>
#include "config.h"

Tcl_Interp *tcl_for_pd = 0;

extern "C" SWIGEXPORT int Tclpd_SafeInit(Tcl_Interp *interp);

extern "C" void tcl_setup (void) {
  if (tcl_for_pd) {
    post("Tcl-for-Pd already loaded");
    return;
  }
  post("loading Tcl-for-Pd");
  tcl_for_pd = Tcl_CreateInterp();
  Tclpd_SafeInit(tcl_for_pd);

  char *dirname   = new char[242];
  char *dirresult = new char[242];
  /* nameresult is only a pointer in dirresult space so don't delete[] it. */
  char *nameresult;
  if (getcwd(dirname,242)<0) {post("AAAARRRRGGGGHHHH!"); exit(69);}
  int       fd=open_via_path(dirname,"gridflow/tcl",PDSUF,dirresult,&nameresult,242,1);
  if (fd<0) fd=open_via_path(dirname,         "tcl",PDSUF,dirresult,&nameresult,242,1);
  if (fd>=0) {
    post("%s found itself in %s","tcl"PDSUF,dirresult);
    close(fd);
  } else {
    post("%s was not found via the -path!","tcl"PDSUF);
  }
  Tcl_SetVar(tcl_for_pd,"DIR",dirresult,0);
  Tcl_Eval(tcl_for_pd,"set auto_path [concat [list $DIR/.. $DIR $DIR/optional/rblti] $auto_path]");
  delete[] dirresult;
  delete[] dirname;

  if (Tcl_Eval(tcl_for_pd,"source $DIR/tcl.tcl") != TCL_OK)
    post("tcl error: %s\n", Tcl_GetString(Tcl_GetObjResult(tcl_for_pd)));
  if (Tcl_Eval(tcl_for_pd,"source $env(HOME)/.pd.tcl") != TCL_OK)
    post("tcl error: %s\n", Tcl_GetString(Tcl_GetObjResult(tcl_for_pd)));
}

%}
