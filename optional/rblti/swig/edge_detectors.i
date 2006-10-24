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
//  Revision 1.5  2006/10/24 22:07:59  heri
//  Three more modules compiling.
//  colors, segmentation, and edge_detectors
//
//  Revision 1.4  2006/09/28 19:16:36  heri
//  Added HarrisCorners (Corner detection)
//
//  Revision 1.3  2006/08/25 23:32:12  heri
//  A bunch of changes to split compilation into several parts (not yet working for some modules)
//
//  Revision 1.2  2006/08/03 22:40:30  heri
//  Splitting compilation into several steps.
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
#define IMPORTMODE
%include base_functors.i
#undef IMPORTMODE
%{
/*#include "ltiThresholding.h"
#include "ltiNonMaximaSuppression.h"
#include "ltiGradientFunctor.h"
#include "ltiColorContrastGradient.h"*/
using namespace lti;
%}
#endif

#ifndef OLD_COMPILE
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

%{
typedef lti::gradientFunctor::parameters lti_gradientFunctor_parameters;
typedef lti::colorContrastGradient::parameters lti_colorContrastGradient_parameters;
%}
#endif

// Edge and Corner Detectors    
HANDLE_FUNCTOR_WITH_PARAMETERS( cornerDetector,          "ltiCornerDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( edgeDetector,            "ltiEdgeDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( classicEdgeDetector,     "ltiClassicEdgeDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( cannyEdges,              "ltiCannyEdges.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( harrisCorners,           "ltiHarrisCorners.h")
