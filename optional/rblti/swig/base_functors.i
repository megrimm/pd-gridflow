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
//  Revision 1.1  2005/12/25 15:41:15  Michael
//  splitted lti.i into different swig interface files
//
//
//******************************************************************************

//%include utils.i

// Base Classes    
HANDLE_FUNCTOR_WITH_PARAMETERS( functor,                 "ltiFunctor.h")
    %{
    //typedef lti::functor::parameters functor_parameters;
    %}
    
HANDLE_FUNCTOR_WITH_PARAMETERS( modifier,                "ltiModifier.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( filter,                  "ltiFilter.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( ioFunctor,               "ltiIOFunctor.h")
