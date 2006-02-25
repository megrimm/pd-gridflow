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
//  Revision 1.1  2006/02/25 23:43:56  matju
//  0.33.1
//
//  Revision 1.1  2005/12/25 15:41:16  Michael
//  splitted lti.i into different swig interface files
//
//
//******************************************************************************

%module ltiextras

%import utils.i

HANDLE_FUNCTOR_WITH_PARAMETERS( functor,                 "ltiFunctor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( ioFunctor,               "ltiIOFunctor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( ioJPEG,                  "ltiJPEGFunctor.h")
    %{
    #define lti_ioJPEG_parameters ioJPEG_parameters          // TODO: PATCH !
    %}
HANDLE_FUNCTOR_WITH_PARAMETERS( ioPNG,                   "ltiPNGFunctor.h")
    %{
    #define lti_ioPNG_parameters ioPNG_parameters            // TODO: PATCH !
    %}

%{
using namespace lti;
%}
