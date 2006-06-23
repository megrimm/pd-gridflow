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
//  Revision 1.6  2006/06/23 18:01:32  heri
//  Fix for 64bit OS compilation error. Basically, just added gcc -fPIC flag.
//
//  Revision 1.3  2006/04/01 04:58:43  heri
//  - Added Class MeanshiftTracker.
//  - All methods with the name "initialize" in lti-lib are renamed to "init" due to conflict with Ruby (including MeanshiftTracker::initialize).
//  - Added template names to allow instantiation of Class Rectangle: Rect (int), Drect (double), Frect (float). Some arguments of MeanshiftTracker methods are of type Rectangle, this will be useful when using MeanshiftTracker.
//  - Added template names for histogram classes: Histogram (double), Ihistogram, Fhistogram, Histogram1D, Histogram2D. Also needed as arguments to methods of MeanshiftTracker. "initialize" method of this class is also renamed to "init".
//  - Added drawing class. A new interface file (drawing.i) has been added for these. Currently, there are two classes available: ImageDraw and ChannelDraw to draw on objects of type Image and Channel respectively. These classes enable drawing simple shapes and text on the canvas provided to them.
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



/*namespace lti {
  %rename(init) meanshiftTracker::initialize(const image &, trectangle< int > &);
  %rename(init) meanshiftTracker::initialize(const image &, trectangle< int > &, const channel8 &);
  %rename(init) meanshiftTracker::initialize(const image &, trectangle< int > &, const channel &);
}*/

HANDLE_FUNCTOR_WITH_PARAMETERS( meanshiftTracker,         "ltiMeanshiftTracker.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( maskImage,                "ltiMaskImage.h")



