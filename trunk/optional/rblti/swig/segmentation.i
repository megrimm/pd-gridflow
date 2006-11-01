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
//  Revision 1.10  2006/11/01 21:20:47  heri
//  Added another apply() method to ObjectsFromMask, which outputs a channel8 containing only
//  the largest blobs from the input.
//
//  Revision 1.9  2006/10/26 18:28:38  heri
//  Compilation in separate modules now working.
//  basedata and base_functors MUST be loaded before any other modules, otherwise you get a segfault.
//
//  Revision 1.8  2006/10/24 22:07:59  heri
//  Three more modules compiling.
//  colors, segmentation, and edge_detectors
//
//  Revision 1.7  2006/08/25 23:32:12  heri
//  A bunch of changes to split compilation into several parts (not yet working for some modules)
//
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
#ifndef OLD_COMPILE
%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i
#endif



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

#ifndef OLD_COMPILE
%{
using namespace lti;
%}
#endif
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

void apply(const lti::channel8& in, const int& minsize, lti::channel8& out)
{
   std::list<lti::areaPoints> objlist;
   out.resize((in.size()).y, (in.size()).x, 0, false, true);
   self->apply(in, objlist);
   std::list<lti::areaPoints>::iterator objiter = objlist.begin();
   
   lti::ipoint pt;
   lti::areaPoints blob;
   lti::areaPoints::iterator ptiter;

   //iterate through each blob found in the channel
   for(; objiter != objlist.end(); objiter++)
   {
   blob = *(objiter);
   ptiter = blob.begin();
      if (blob.size() >= minsize)
      {
	for(; ptiter != blob.end(); ptiter++)
	{
		out.at((*ptiter).y, (*ptiter).x) = 255;
	}
      }
   }
}
}
/*namespace lti{
%ignore objectsFromMask::apply(const channel8 &, std::list< ioPoints > &);
%ignore objectsFromMask::apply(const matrix< int > &, std::list< ioPoints > &);
%ignore objectsFromMask::apply(const channel8 &, std::list< borderPoints > &);
%ignore objectsFromMask::apply(const imatrix &, std::list< borderPoints > &);
%ignore objectsFromMask::apply(const channel8 &, std::list< areaPoints > &);
%ignore objectsFromMask::apply(const matrix< int > &, std::list< areaPoints > &);
%ignore objectsFromMask::apply(const channel8 &, std::list< areaPoints > &, matrix< int > &);
%ignore objectsFromMask::apply(const matrix< int > &, std::list< areaPoints > &, matrix< int > &);
}
extend lti::objectsFromMask {
bool 	apply (const channel8 &src8, std::list< ioPoints > &lstIOPointLists)
bool 	apply (const matrix< int > &src, std::list< ioPoints > &lstIOPointLists)
bool 	apply (const channel8 &src8, std::list< borderPoints > &lstBorderPointLists)
bool 	apply (const imatrix &src, std::list< borderPoints > &lstBorderPointLists)
bool 	apply (const channel8 &src8, std::list< areaPoints > &lstAreaPointLists)
bool 	apply (const matrix< int > &src, std::list< areaPoints > &lstAreaPointLists)
bool 	apply (const channel8 &src8, std::list< areaPoints > &lstAreaPointLists, matrix< int > &labeledMask)
bool 	apply (const matrix< int > &src, std::list< areaPoints > &lstAreaPointLists, matrix< int > &labeledMask)
}*/

HANDLE_FUNCTOR_WITH_PARAMETERS( backgroundModel,         "ltiBackgroundModel.h")



/*namespace lti {
  %rename(init) meanshiftTracker::initialize(const image &, trectangle< int > &);
  %rename(init) meanshiftTracker::initialize(const image &, trectangle< int > &, const channel8 &);
  %rename(init) meanshiftTracker::initialize(const image &, trectangle< int > &, const channel &);
}*/

HANDLE_FUNCTOR_WITH_PARAMETERS( meanshiftTracker,         "ltiMeanshiftTracker.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( maskImage,                "ltiMaskImage.h")



