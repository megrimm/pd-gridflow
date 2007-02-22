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
%{
using namespace lti;
%}
#endif

// Image Transformations    
HANDLE_FUNCTOR_WITH_PARAMETERS( scaling,                 "ltiScaling.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( rotation,                "ltiRotation.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( flipImage,               "ltiFlipImage.h")
    %{ // PATCH for c++ phase: rename the enum constant "None" in flipImage::parameters, because of a conflict with Python None !!!
    #define _None None
    %}
HANDLE_FUNCTOR_WITH_PARAMETERS( geometricTransform,      "ltiGeometricTransform.h")


%rename (RdistanceTransform_sedMask) distanceTransform::sedMask;

#ifndef OLD_COMPILE
IMPORT_FUNCTOR_WITH_PARAMETERS( morphology,       "ltiMorphology.h")
#endif
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
