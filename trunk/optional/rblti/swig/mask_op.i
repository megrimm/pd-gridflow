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
//  Revision 1.7  2006/08/25 23:32:12  heri
//  A bunch of changes to split compilation into several parts (not yet working for some modules)
//
//  Revision 1.6  2006/06/23 18:01:32  heri
//  Fix for 64bit OS compilation error. Basically, just added gcc -fPIC flag.
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
#ifndef OLD_COMPILE
%module rblti
%include utils.i
%include dep.i
%import basedata.i
#define IMPORTMODE
%include base_functors.i
#undef IMPORTMODE
#endif

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
