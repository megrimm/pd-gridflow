//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i

// Mask Operators
HANDLE_SIMPLE_HEADER_FILE("ltiMaskFunctors.h")
namespace lti {
  class maskFunctor<rgbPixel>;
  %template(imaskFunctor)     maskFunctor<int>;
  %template(dmaskFunctor)     maskFunctor<double>;
//%template(rgbPixelMaskFunctor) maskFunctor<rgbPixel>;
  template(umaskFunctor)      maskFunctor<ubyte>;
//%template(imaskAnd)         maskAnd<int>;
//%template(imaskOr)          maskOr<int>;
//%template(imaskNot)         maskNot<int>;
//%template(imaskInvert)      maskInvert<int>;
//%template(imaskMultiply)    maskMultiply<int>;
//%template(imaskAlgebraicSum) maskAlgebraicSum<int>;
}
