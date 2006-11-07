%module tclpd
%ignore pd_getfilename;
%ignore pd_getdirname;
%ignore pd_anything;
%ignore class_parentwidget;
%ignore sys_isreadablefile;
%ignore garray_get;
%ignore c_extern;
%ignore c_addmess;
%include "m_pd.h"

%{
#include "m_pd.h"

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
  int ret = Tcl_Eval(tcl_for_pd,"source $env(HOME)/.pd.tcl");
  if (ret != TCL_OK) {
    post("tcl error: %s\n", Tcl_GetString(Tcl_GetObjResult(tcl_for_pd)));
  }
}

%}
