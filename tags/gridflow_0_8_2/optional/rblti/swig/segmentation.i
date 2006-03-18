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

// Segmentation and Localization    
HANDLE_FUNCTOR_WITH_PARAMETERS( segmentation,            "ltiSegmentation.h")
#ifdef SWIGRUBY
%{
#undef connect
%}
#endif
HANDLE_FUNCTOR_WITH_PARAMETERS( meanShiftSegmentation,   "ltiMeanShiftSegmentation.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( kMeansSegmentation,      "ltiKMeansSegmentation.h")
    %{
    typedef lti::kMeansSegmentation::parameters lti_kMeansSegmentation_parameters;
    %}
    %extend lti::_kMeansSegmentation::RkMeansSegmentation_parameters {
    // TODO: is there a better way to support complex attributes ?
    // a helper method to set complex attributes of a parameters-class
    void setQuantParameters(const lti::_kMColorQuantization::RkMColorQuantization_parameters & value) 
    {
        self->quantParameters = value;
    } 
    };
HANDLE_FUNCTOR_WITH_PARAMETERS( whiteningSegmentation,   "ltiWhiteningSegmentation.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( csPresegmentation,       "ltiCsPresegmentation.h")
//deprecated: HANDLE_FUNCTOR_WITH_PARAMETERS( thresholdSegmentation,   "ltiThresholdSegmentation.h")    
HANDLE_FUNCTOR_WITH_PARAMETERS( thresholding,            "ltiThresholding.h")    
HANDLE_FUNCTOR_WITH_PARAMETERS( watershedSegmentation,   "ltiWatershedSegmentation.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( regionGrowing,           "ltiRegionGrowing.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( snake,                   "ltiSnake.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( regionMerge,             "ltiRegionMerge.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( similarityMatrix,        "ltiSimilarityMatrix.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( fastRelabeling,          "ltiFastRelabeling.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( objectsFromMask,         "ltiObjectsFromMask.h")
    %{
    #define RobjectsFromMask_objectStruct objectStruct
    namespace lti {
    typedef lti::objectsFromMask::objectStruct objectStruct;
    typedef lti::objectsFromMask::objectStruct objectsFromMask_objectStruct;
    }
    %}
HANDLE_FUNCTOR_WITH_PARAMETERS( backgroundModel,         "ltiBackgroundModel.h")
