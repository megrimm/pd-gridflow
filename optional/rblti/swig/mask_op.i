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
//  Revision 1.5  2006/05/26 02:27:26  heri
//  Let's try this again
//
//  Revision 1.3  2006/04/25 15:47:37  heri
//  Fixed the following methods for matrix types, including Channel, Channel8, Image, Matrix templates:
//  - at
//  - fill: for this overloaded method, all the versions that take an array as argument are ignored because
//   SWIG confuses an int for the array pointer. Anyway, C arrays cannot be used directly in rblti.
//  - castFrom
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
    class maskFunctor<rgbPixel>;
    
        %template(imaskFunctor)     	maskFunctor<int>;
	%template(dmaskFunctor)		maskFunctor<double>;
	//%template(rgbPixelMaskFunctor)	maskFunctor<rgbPixel>;
	%template(umaskFunctor)		maskFunctor<ubyte>;
        //%template(imaskAnd)         maskAnd<int>;
        //%template(imaskOr)          maskOr<int>;
        //%template(imaskNot)         maskNot<int>;
        //%template(imaskInvert)      maskInvert<int>;
        //%template(imaskMultiply)    maskMultiply<int>;
        //%template(imaskAlgebraicSum) maskAlgebraicSum<int>;
		
    }
