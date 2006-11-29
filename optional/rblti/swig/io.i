//******************************************************************************
//
//      project:  pylti
//    copyright:  (c) 2005 by Michael Neuroth
//
//  description:  python modul for ltilib (using SWIG)
//                    ltilib extras functions and classes
//
//  $Source$
//  $Revision$
//  $Date$
// 
//  $Log$
//  Revision 1.4  2006/11/29 20:50:21  heri
//  Blob Tracker part V, plus some other minor previous changes.
//
//  Revision 1.3  2006/10/26 18:28:38  heri
//  Compilation in separate modules now working.
//  basedata and base_functors MUST be loaded before any other modules, otherwise you get a segfault.
//
//  Revision 1.2  2006/08/25 23:32:12  heri
//  A bunch of changes to split compilation into several parts (not yet working for some modules)
//
//  Revision 1.1  2006/02/25 23:43:56  matju
//  0.33.1
//
//  Revision 1.1  2005/12/25 15:41:16  Michael
//  splitted lti.i into different swig interface files
//
//
//******************************************************************************

//%include utils.i
#ifndef OLD_COMPILE
%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i
#endif

// Image formats    
HANDLE_FUNCTOR_WITH_PARAMETERS_PATCH( ioBMP,                   "ltiBMPFunctor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS_PATCH( ioJPEG,                  "ltiJPEGFunctor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS_PATCH( ioPNG,                   "ltiPNGFunctor.h")

//HANDLE_FUNCTOR_WITH_PARAMETERS( ioBMP,                   "ltiBMPFunctor.h")
//    %{
//    #define lti_ioBMP_parameters ioBMP_parameters            // TODO: PATCH !
//    %}
//HANDLE_FUNCTOR_WITH_PARAMETERS( ioJPEG,                  "ltiJPEGFunctor.h")
//    %{
//    #define lti_ioJPEG_parameters ioJPEG_parameters          // TODO: PATCH !
//    %}
//HANDLE_FUNCTOR_WITH_PARAMETERS( ioPNG,                   "ltiPNGFunctor.h")
//    %{
//    #define lti_ioPNG_parameters ioPNG_parameters            // TODO: PATCH !
//    %}

    // Patch needed for ioImage functor...
    %{
    #define ioImage_parameters parameters                    // TODO: PATCH !
    %}
HANDLE_FUNCTOR_WITH_PARAMETERS( ioImage,                 "ltiALLFunctor.h")
    
// Misc    
HANDLE_FUNCTOR_WITH_PARAMETERS( viewerBase,              "ltiViewerBase.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( externViewer,            "ltiExternViewer.h")
namespace lti {
%ignore fastViewer::setParameters;
}
HANDLE_FUNCTOR_WITH_PARAMETERS( fastViewer,              "ltiFastViewer.h")
