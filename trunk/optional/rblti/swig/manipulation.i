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

// Points, Contours and Shape Manipulation    
HANDLE_FUNCTOR_WITH_PARAMETERS( borderExtrema,           "ltiBorderExtrema.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( interpolator,            "ltiInterpolator.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( variablySpacedSamplesInterpolator, "ltiVariablySpacedSamplesInterpolator.h")    
//HANDLE_FUNCTOR_WITH_PARAMETERS( cubicSpline,             "ltiCubicSpline.h")             // Template Klasse !!!
//    %{
//    %template(icubicSpline) cubicSpline<int>;
//    %}
//TODO HANDLE_FUNCTOR_WITH_PARAMETERS( boundingBox,             "ltiBoundingBox.h")      // Template Klasse !!!
HANDLE_FUNCTOR_WITH_PARAMETERS( polygonApproximation,    "ltiPolygonApproximation.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( convexHull,              "ltiConvexHull.h")
