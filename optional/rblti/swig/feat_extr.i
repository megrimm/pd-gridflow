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

// Feature Extraction    
HANDLE_FUNCTOR_WITH_PARAMETERS( featureExtractor,        "ltiFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( globalFeatureExtractor,  "ltiGlobalFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( localFeatureExtractor,   "ltiLocalFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( localMoments,            "ltiLocalMoments.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( geometricFeatures,       "ltiGeometricFeatures.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( chromaticityHistogram,   "ltiChromaticityHistogram.h")
