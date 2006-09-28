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
//  Revision 1.3  2006/09/28 19:16:36  heri
//  Added HarrisCorners (Corner detection)
//
//  Revision 1.2  2006/08/25 23:32:12  heri
//  A bunch of changes to split compilation into several parts (not yet working for some modules)
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
#endif

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

HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS( maximumFilter, "ltiMaximumFilter.h")
namespace lti{
%template(MaximumFilter) maximumFilter<float>;
}
