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
//  Revision 1.3  2006/08/08 19:55:02  heri
//  Compilation of rblti as a single file stopped working when the compilation was split into
//  several parts in the previous commit.
//  Now it should work again.
//
//  Revision 1.2  2006/08/03 22:40:03  heri
//  Splitting compilation into several steps
//
//  Revision 1.1  2006/02/25 23:43:56  matju
//  0.33.1
//
//  Revision 1.1  2005/12/25 15:41:15  Michael
//  splitted lti.i into different swig interface files
//
//
//******************************************************************************
#ifndef SWIGIMPORTED
#ifndef IMPORTMODE
#ifndef RBLTI
%module base_functors
%include utils.i
%include dep.i
%import basedata.i
#endif
#endif
#endif

// Base Classes    
HANDLE_FUNCTOR_WITH_PARAMETERS( functor,                 "ltiFunctor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( modifier,                "ltiModifier.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( filter,                  "ltiFilter.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( ioFunctor,               "ltiIOFunctor.h")
