//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i

// Segmentation and Localization    
HANDLE_FUNCTOR_WITH_PARAMETERS( segmentation,            "ltiSegmentation.h")
#ifdef SWIGRUBY
%{
  #undef connect
%}
#endif
HANDLE_FUNCTOR_WITH_PARAMETERS( meanShiftSegmentation,   "ltiMeanShiftSegmentation.h")

IMPORT_FUNCTOR_WITH_PARAMETERS( colorQuantization,       "ltiColorQuantization.h")
IMPORT_FUNCTOR_WITH_PARAMETERS(kMColorQuantization,     "ltiKMColorQuantization.h")

%{
  using namespace lti;
%}
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

%extend lti::objectsFromMask {
  void apply(const lti::channel8& in, const int& minsize, lti::channel8& out) {
    std::list<lti::areaPoints> objlist;
    out.resize((in.size()).y, (in.size()).x, 0, false, true);
    self->apply(in, objlist);
    std::list<lti::areaPoints>::iterator objiter = objlist.begin();
    lti::ipoint pt;
    lti::areaPoints blob;
    lti::areaPoints::iterator ptiter;
    //iterate through each blob found in the channel
    for(; objiter != objlist.end(); objiter++) {
    blob = *objiter;
    ptiter = blob.begin();
       if (blob.size() >= minsize) {
	 for(; ptiter != blob.end(); ptiter++) {
	    out.at(ptiter->y, ptiter->x) = 255;
	 }
       }
    }
  }
}

HANDLE_FUNCTOR_WITH_PARAMETERS( backgroundModel,         "ltiBackgroundModel.h")

/*namespace lti {
  %rename(init) meanshiftTracker::initialize(const image &, trectangle< int > &);
  %rename(init) meanshiftTracker::initialize(const image &, trectangle< int > &, const channel8 &);
  %rename(init) meanshiftTracker::initialize(const image &, trectangle< int > &, const channel &);
}*/

HANDLE_FUNCTOR_WITH_PARAMETERS( meanshiftTracker,         "ltiMeanshiftTracker.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( maskImage,                "ltiMaskImage.h")
