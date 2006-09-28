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
//  Revision 1.4  2006/09/28 19:16:36  heri
//  Added HarrisCorners (Corner detection)
//
//  Revision 1.3  2006/08/25 23:32:12  heri
//  A bunch of changes to split compilation into several parts (not yet working for some modules)
//
//  Revision 1.2  2006/03/05 01:18:55  heri
//  More changes related to the added classes: backgroundModel,
//  geometricFeaturesFromMask and
//  multiGeometricFeaturesFromMask
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
#endif

// Feature Extraction    
HANDLE_FUNCTOR_WITH_PARAMETERS( featureExtractor,               "ltiFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( globalFeatureExtractor,         "ltiGlobalFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( localFeatureExtractor,          "ltiLocalFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( localMoments,                   "ltiLocalMoments.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( geometricFeatures,              "ltiGeometricFeatures.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( chromaticityHistogram,          "ltiChromaticityHistogram.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( geometricFeaturesFromMask,      "ltiGeometricFeaturesFromMask.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( multiGeometricFeaturesFromMask, "ltiMultiGeometricFeaturesFromMask.h")

//HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS( localMaxima, "ltiLocalMaxima.h")
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
%include _localMaxima_parameters.h
#undef T
%include "ltiLocalMaxima.h"
#undef parameters

namespace lti{
%template(LocalMaxima) localMaxima<float>;
}