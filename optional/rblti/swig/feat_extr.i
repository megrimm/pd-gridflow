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

// Feature Extraction    
HANDLE_FUNCTOR_WITH_PARAMETERS( featureExtractor,               "ltiFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( globalFeatureExtractor,         "ltiGlobalFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( localFeatureExtractor,          "ltiLocalFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( localMoments,                   "ltiLocalMoments.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( geometricFeatures,              "ltiGeometricFeatures.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( chromaticityHistogram,          "ltiChromaticityHistogram.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( geometricFeaturesFromMask,      "ltiGeometricFeaturesFromMask.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( multiGeometricFeaturesFromMask, "ltiMultiGeometricFeaturesFromMask.h")