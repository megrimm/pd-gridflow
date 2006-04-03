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

// Edge and Corner Detectors    
HANDLE_FUNCTOR_WITH_PARAMETERS( cornerDetector,          "ltiCornerDetector.h")
//TODO HANDLE_FUNCTOR_WITH_PARAMETERS( harrisCorners,           "ltiHarrisCorners.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( edgeDetector,            "ltiEdgeDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( classicEdgeDetector,     "ltiClassicEdgeDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( cannyEdges,              "ltiCannyEdges.h")
