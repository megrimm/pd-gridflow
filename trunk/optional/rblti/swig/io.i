//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
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
