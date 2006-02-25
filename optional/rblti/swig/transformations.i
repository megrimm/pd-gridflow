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

//%include utils.i

// Image Transformations    
HANDLE_FUNCTOR_WITH_PARAMETERS( scaling,                 "ltiScaling.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( rotation,                "ltiRotation.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( flipImage,               "ltiFlipImage.h")
    %{ // PATCH for c++ phase: rename the enum constant "None" in flipImage::parameters, because of a conflict with Python None !!!
    #define _None None
    %}
HANDLE_FUNCTOR_WITH_PARAMETERS( geometricTransform,      "ltiGeometricTransform.h")


%rename (RdistanceTransform_sedMask) distanceTransform::sedMask;
HANDLE_FUNCTOR_WITH_PARAMETERS( distanceTransform,       "ltiDistanceTransform.h")
/*
%{
#include ltiDistanceTransform.h
#define _distanceTransform distanceTransform
#define RdistanceTransform_parameters parameters
#define RdistanceTransform_sedMask sedMask
namespace lti {
typedef lti::distanceTransform::parameters distanceTransform_parameters;
typedef lti::distanceTransform::sedMask distanceTransform_sedMask;
}
%}

#define parameters distanceTransform_parameters
#define sedMask distanceTransform_sedMask
%include _distanceTransform_parameters.h
%include _distanceTransform_sedMask.h
%include ltiDistanceTransform.h
#undef parameters
#undef sedMask
*/
