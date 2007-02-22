//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
%define HANDLE_SIMPLE_HEADER_FILE(headername)
%{
#include headername
%}
%include headername
%enddef

%define IMPORT_SIMPLE_HEADER_FILE(headername)
%inline %{
#include headername
%}
%import headername
%enddef

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//%define HANDLE_SIMPLE_HEADER_FILE_NAMESPACE(headername)
//%wrapper %{
//namespace { using namespace lti;
//%}
//%include headername
//%inline %{
//#include headername
//%}
//%wrapper %{
//}
//%}
//%enddef

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// with this macro we handle all the shifting of namespaces for the nested parameters classes
%define HANDLE_FUNCTOR_WITH_PARAMETERS(func_name,functor_header)
// *************************************************
// this is for the c++ wrapper compile phase...
// it generates something like this:
//--> #include "ltiSkeleton.h"
// // durch SWIG manipulierte typ parameters in functor wieder zurueck benennen
//--> #define _skeleton skeleton                              // wegen Namenskonflikt mit schon deklarierter Klasse und dem Trick ueber den namespace um die Parameter-Klassen zu generieren (gen_ltilib_class.py)
//--> #define _skeleton_parameters parameters                 // sezte den aus dem XML generierten Parameter-Klassen-Namen wieder zurueck auf den urspruenglichen Namen
//--> namespace lti {
//--> typedef lti::skeleton::parameters skeleton_parameters;  // notwendig fuer die plain Methoden lti::write(...) und lti::read(...)
//--> }
// %inline 
%{
#include functor_header
#define _ ## func_name func_name
#define R ## func_name ## _parameters parameters
namespace lti {
typedef lti:: ## func_name ## ::parameters func_name ## _parameters;
}
%}
// *************************************************
// this is for the SWIG parser phase...
// it generates something like this:
//--> #define parameters skeleton_parameters    
//--> %include "_skeleton_parameters.h"
//--> %include "ltiSkeleton.h"
//--> #undef parameters

#define parameters func_name ## _parameters
#ifdef IMPORTMODE
%import _ ## func_name ## _parameters.h
%import functor_header
#else
%include _ ## func_name ## _parameters.h
%include functor_header
#endif
#undef parameters
%enddef


%define IMPORT_FUNCTOR_WITH_PARAMETERS(func_name,functor_header)
%{
#include functor_header
#define _ ## func_name func_name
#define R ## func_name ## _parameters parameters
namespace lti {
typedef lti:: ## func_name ## ::parameters func_name ## _parameters;
}
typedef lti:: ## func_name ## ::parameters lti_ ## func_name ## _parameters;
%}
#define parameters func_name ## _parameters
%import _ ## func_name ## _parameters.h
%import functor_header
#undef parameters
%enddef



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// with this macro we handle all the shifting of namespaces for the nested parameters classes
%define HANDLE_FUNCTOR_WITH_PARAMETERS_PATCH(func_name,functor_header)
// *************************************************
// this is for the c++ wrapper compile phase...
// it generates something like this:
//--> #include "ltiSkeleton.h"
// // durch SWIG manipulierte typ parameters in functor wieder zurueck benennen
//--> #define _skeleton skeleton                              // wegen Namenskonflikt mit schon deklarierter Klasse und dem Trick ueber den namespace um die Parameter-Klassen zu generieren (gen_ltilib_class.py)
//--> #define _skeleton_parameters parameters                 // sezte den aus dem XML generierten Parameter-Klassen-Namen wieder zurueck auf den urspruenglichen Namen
//--> namespace lti {
//--> typedef lti::skeleton::parameters skeleton_parameters;  // notwendig fuer die plain Methoden lti::write(...) und lti::read(...)
//--> }
//%inline 
%{
#include functor_header
#define _ ## func_name func_name
#define R ## func_name ## _parameters parameters
#define lti_ ## func_name ## _parameters lti:: ## func_name ## ::parameters      // TODO: PATCH !
namespace lti {
typedef lti:: ## func_name ## ::parameters func_name ## _parameters;
}
typedef lti:: ## func_name ## ::parameters func_name ## _parameters;    // wegen RUBY ?
%}
// *************************************************
// this is for the SWIG parser phase...
// it generates something like this:
//--> #define parameters skeleton_parameters    
//--> %include "_skeleton_parameters.h"
//--> %include "ltiSkeleton.h"
//--> #undef parameters
#define parameters func_name ## _parameters
%include _ ## func_name ## _parameters.h
%include functor_header
#undef parameters
%enddef

%define HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(func_name,functor_header)
%{
#include functor_header
#define _ ## func_name func_name<float>
#define R ## func_name ## _parameters parameters
namespace lti {
typedef lti:: ## func_name<float> ## ::parameters func_name ## _parameters;
}
%}
#define parameters func_name ## _parameters
%include _ ## func_name ## _parameters.h
%include functor_header
#undef parameters
%enddef


%define IMPORT_FUNCTOR_TEMPLATE_WITH_PARAMETERS(func_name,functor_header)
%{
#include functor_header
#define _ ## func_name func_name<float>
#define R ## func_name ## _parameters parameters
namespace lti {
typedef lti:: ## func_name<float> ## ::parameters func_name ## _parameters;
}
%}
#define parameters func_name ## _parameters
%import _ ## func_name ## _parameters.h
%import functor_header
#undef parameters
%enddef

