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

#ifndef OLD_COMPILE
IMPORT_FUNCTOR_WITH_PARAMETERS( transform,               "ltiTransform.h")
IMPORT_FUNCTOR_WITH_PARAMETERS( thresholding,            "ltiThresholding.h")
IMPORT_FUNCTOR_WITH_PARAMETERS( gradientFunctor,         "ltiGradientFunctor.h")

%{
#include "ltiLocalMaxima.h"
#define _localMaxima localMaxima<float>
#define RlocalMaxima_parameters parameters
namespace lti {
typedef lti::localMaxima<float>::parameters localMaxima_parameters;
}
%}
#define parameters localMaxima_parameters
#define T float
%import _localMaxima_parameters.h
#undef T
%import "ltiLocalMaxima.h"
#undef parameters


IMPORT_FUNCTOR_WITH_PARAMETERS( colorContrastGradient,   "ltiColorContrastGradient.h")
#endif

// Edge and Corner Detectors    
HANDLE_FUNCTOR_WITH_PARAMETERS( cornerDetector,          "ltiCornerDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( edgeDetector,            "ltiEdgeDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( classicEdgeDetector,     "ltiClassicEdgeDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( cannyEdges,              "ltiCannyEdges.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( harrisCorners,           "ltiHarrisCorners.h")
