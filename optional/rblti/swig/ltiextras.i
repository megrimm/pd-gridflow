//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

%module ltiextras
%import utils.i

HANDLE_FUNCTOR_WITH_PARAMETERS( functor,         "ltiFunctor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( ioFunctor,       "ltiIOFunctor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( ioJPEG,          "ltiJPEGFunctor.h")
%{
#define lti_ioJPEG_parameters ioJPEG_parameters  // TODO: PATCH !
%}
HANDLE_FUNCTOR_WITH_PARAMETERS( ioPNG,           "ltiPNGFunctor.h")
%{
#define lti_ioPNG_parameters ioPNG_parameters    // TODO: PATCH !
%}
%{
using namespace lti;
%}
