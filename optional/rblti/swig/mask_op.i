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
//  Revision 1.2  2006/04/24 22:08:02  heri
//  I thought I already did this!
//  Any change in swig directory will now prompt a build from the start.
//  Added SWIG template for RgbPixel MaskFunctors
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

// Mask Operators    
HANDLE_SIMPLE_HEADER_FILE("ltiMaskFunctors.h")
    namespace lti {
        %template(imaskFunctor)     	maskFunctor<int>;
	%template(rbgPixelMaskFunctor)	maskFunctor<rgbPixel>;
//        %template(imaskAnd)         maskAnd<int>;
//        %template(imaskOr)          maskOr<int>;
//        %template(imaskNot)         maskNot<int>;
//        %template(imaskInvert)      maskInvert<int>;
//        %template(imaskMultiply)    maskMultiply<int>;
//        %template(imaskAlgebraicSum) maskAlgebraicSum<int>;
    }
