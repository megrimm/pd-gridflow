//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

//%include utils.i
#ifndef OLD_COMPILE
%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i
%{
using namespace lti;
%}


IMPORT_FUNCTOR_WITH_PARAMETERS( fastRelabeling,          	"ltiFastRelabeling.h")
IMPORT_FUNCTOR_TEMPLATE_WITH_PARAMETERS( maximumFilter, 	"ltiMaximumFilter.h")
namespace lti{
%template(MaximumFilter) maximumFilter<float>;
}
#endif


// Feature Extraction    
HANDLE_FUNCTOR_WITH_PARAMETERS( featureExtractor,               "ltiFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( globalFeatureExtractor,         "ltiGlobalFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( localFeatureExtractor,          "ltiLocalFeatureExtractor.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( localMoments,                   "ltiLocalMoments.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( geometricFeatures,              "ltiGeometricFeatures.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( chromaticityHistogram,          "ltiChromaticityHistogram.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( geometricFeaturesFromMask,      "ltiGeometricFeaturesFromMask.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( multiGeometricFeaturesFromMask, "ltiMultiGeometricFeaturesFromMask.h")

//HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS( localMaxima, "ltiLocalMaxima.h")
%{
#include "ltiLocalMaxima.h"
#define _localMaxima localMaxima<float>
#define RlocalMaxima_parameters parameters
namespace lti {
typedef lti::localMaxima<float>::parameters localMaxima_parameters;
}
%}
#define parameters localMaxima_parameters
#define T float
%include _localMaxima_parameters.h
#undef T
%include "ltiLocalMaxima.h"
#undef parameters

namespace lti{
%template(LocalMaxima) localMaxima<float>;
}

//HANDLE_FUNCTOR_WITH_PARAMETERS( fastLineExtraction, "ltiFastLineExtraction.h")

%{
#include "ltiFastLineExtraction.h"
#define _fastLineExtraction fastLineExtraction
#define RfastLineExtraction_parameters parameters
#define RfastLineExtraction_segmEntry segmEntry
namespace lti {
typedef lti::fastLineExtraction::parameters fastLineExtraction_parameters;
typedef lti::fastLineExtraction::segmEntry fastLineExtraction_segmEntry;
}
%}

#define parameters fastLineExtraction_parameters
#define segmEntry fastLineExtraction_segmEntry
%include _fastLineExtraction_parameters.h
%include _fastLineExtraction_segmEntry.h
%include ltiFastLineExtraction.h
#undef parameters
//#undef segmEntry


namespace lti {
%template(vector_segmEntry) std::vector< fastLineExtraction_segmEntry >;
}


%extend lti::fastLineExtraction {
void apply(const lti::channel8& chan, std::vector<segmEntry>& list){
self->apply(chan);
list = self->getLineList(0);
}
}
