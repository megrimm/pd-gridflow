//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

//%include utils.i
%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i

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
//%ignore genericMatrix<rgbPixel>::castFrom(const genericMatrix<float> &);
namespace lti{
%ignore regionsPolygonizer::apply(const matrix<int> &, const int, std::vector<polygonPoints> &) const;
%ignore regionsPolygonizer::apply(const matrix<int> &, const int, std::vector<polygonPoints> &, 
std::vector<borderPoints> &, matrix<int> &) const;
}
HANDLE_FUNCTOR_WITH_PARAMETERS( regionsPolygonizer,      "ltiRegionsPolygonizer.h")

%extend lti::regionsPolygonizer{
  bool apply(const matrix<int>& imat, std::vector<polygonPoints>& poly)const {
    return self->apply(imat,imat.maximum(), poly);
  }
  bool apply(const matrix<int>& imat, std::vector<polygonPoints>& poly,
  std::vector<borderPoints>& border, matrix<int>& neigh) const {
    return self->apply(imat,imat.maximum(), poly, border, neigh);
  }
}
