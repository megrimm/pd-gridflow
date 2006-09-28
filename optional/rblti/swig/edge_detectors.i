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
%{
#include "ltiThresholding.h"
#include "ltiNonMaximaSuppression.h"
#include "ltiGradientFunctor.h"
#include "ltiColorContrastGradient.h"
%}
#define IMPORTMODE
%include base_functors.i
#undef IMPORTMODE
#endif


// Edge and Corner Detectors    
HANDLE_FUNCTOR_WITH_PARAMETERS( cornerDetector,          "ltiCornerDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( edgeDetector,            "ltiEdgeDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( classicEdgeDetector,     "ltiClassicEdgeDetector.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( cannyEdges,              "ltiCannyEdges.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( harrisCorners,           "ltiHarrisCorners.h")
