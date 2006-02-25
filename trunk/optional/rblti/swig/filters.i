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
HANDLE_FUNCTOR_WITH_PARAMETERS( transform,               "ltiTransform.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( realFFT,                 "ltiRealFFT.h")
//TODO HANDLE_FUNCTOR_WITH_PARAMETERS( qmf,                     "ltiQmf.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( orientedHLTransform,     "ltiOrientedHLTransform.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( gHoughTransform,         "ltiGHoughTransform.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( orientationMap,          "ltiOrientationMap.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( medianFilter,            "ltiMedianFilter.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( kNearestNeighFilter,     "ltiKNearestNeighFilter.h")

// Filters and Transformations
HANDLE_FUNCTOR_WITH_PARAMETERS( convolution,             "ltiConvolution.h")         // TODO: problems with private class accumulator !!! --> can we solve this with generated header file out of the XML-output ?
//TODO HANDLE_FUNCTOR_WITH_PARAMETERS( correlation,             "ltiCorrelation.h")

HANDLE_FUNCTOR_WITH_PARAMETERS( gradientFunctor,         "ltiGradientFunctor.h")
    %{
//    typedef lti::gradientFunctor::parameters gradientFunctor_parameters;    // for RUBY ?
//    #define lti_gradientFunctor_parameters gradientFunctor_parameters      // TODO: PATCH !
      #define lti_gradientFunctor_parameters lti::gradientFunctor::parameters      // TODO: PATCH !
    %}
HANDLE_FUNCTOR_WITH_PARAMETERS( colorContrastGradient,   "ltiColorContrastGradient.h")
    %{
//    typedef lti::colorContrastGradient::parameters colorContrastGradient_parameters;    // for RUBY ?
//    #define lti_colorContrastGradient_parameters colorContrastGradient_parameters      // TODO: PATCH !
      #define lti_colorContrastGradient_parameters lti::colorContrastGradient::parameters      // TODO: PATCH !
    %}
