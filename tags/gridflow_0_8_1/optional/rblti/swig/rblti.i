//******************************************************************************
//
//      project:  pylti
//    copyright:  (c) 2005 by Michael Neuroth
//
//  description:  python modul for ltilib (using SWIG)
//
//  $Source$
//  $Revision$
//  $Date$
// 
//  $Log$
//  Revision 1.4  2006/03/04 18:08:24  matju
//  added Imatrix.meat
//
//  Revision 1.3  2006/03/02 22:11:42  matju
//  longs for pointers oughta be unsigned.
//
//  Revision 1.2  2006/03/02 21:55:32  matju
//  fixed warnings.
//  tried adding method "meat" to Image...
//
//  Revision 1.1  2006/02/25 23:43:56  matju
//  0.33.1
//
//  Revision 1.16  2005/12/25 15:44:24  Michael
//  splitted lti.i into different interface files.
//
//  Revision 1.15  2005/07/24 18:46:20  Michael
//  added simple geometric operations for images
//
//  Revision 1.14  2005/07/04 20:02:11  Michael
//  Cleaning up the code
//
//  Revision 1.13  2005/06/28 20:53:30  Michael
//  Added new functors: cannyEdges and classifier
//
//  Revision 1.12  2005/06/20 19:25:53  Michael
//  Support for colour space added.
//
//  Revision 1.11  2005/06/17 21:06:56  Michael
//  Added and improved some %extend statements
//
//  Revision 1.10  2005/06/14 20:15:35  Michael
//  Better template-support: %extend works also on templates, optimized typedeclaration sequence
//
//  Revision 1.9  2005/06/13 20:43:23  Michael
//  Experiments for tree support.
//
//  Revision 1.8  2005/06/12 21:11:38  Michael
//  Tests for the support for the tree.
//
//  Revision 1.7  2005/05/16 13:38:38  Michael
//  Experimented with tree support. included lti_manual.cpp.
//
//  Revision 1.6  2005/05/01 10:40:12  Michael
//  Added index-operator [] for generic vectors. Tests for support for trees.
//
//  Revision 1.5  2005/02/22 23:06:50  Michael
//  Added more functors, improved classes with %extend command
//
//  Revision 1.4  2005/02/19 22:29:09  Michael
//  Added some more functors and functions.
//
//  Revision 1.3  2005/02/14 22:16:38  Michael
//  bugfix: swig has problems with %feature statement
//
//  Revision 1.2  2005/02/12 10:43:07  Michael
//  generate an aditional doc strings and better support for inheritance of parameters classes
//
//  Revision 1.1.1.1  2005/02/09 21:41:11  Michael
//  initial checkin
//
//
//******************************************************************************
//
// generated swig-wrapper compiles only with VC++ 7.1 (.NET 2003), NOT with VC++ 6.0 ! 
//

%module rblti

%{
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
%}

%rename(inplace_add) operator +=;
%rename(inplace_sub) operator -=;
%rename(inplace_mul) operator *=;
%rename(inplace_div) operator /=;
%rename(not_equal)   operator !=;
%include "std_string.i"
#ifndef SWIGRUBY
%include "std_list.i"
#endif
#ifdef SWIGRUBY
%include "std_list_ruby.i"
#endif

//%include "std_vector.i" 
//%include "std_map.i"

using namespace std;

// sollte nach dem include der std_*.i Dateien stehen, ansonsten gibt swig einen Fehlercode zurueck !
//%feature("autodoc","1")

std::string _getStdString(std::string * pStr);
bool _setStdString(std::string * pStr, const std::string & strValue);

// for the access to c arrays
%include "carrays.i"
%array_functions(float,floatArray)
%array_functions(double,doubleArray)
%array_functions(int,intArray)
%array_class(float,floatArr)
%array_class(double,doubleArr)
%array_class(int,intArr)

//test:
//namespace std {
//    %template(sdmap) map<string,double>;		// TODO: does not work yet ...
//}
//namespace std {
//    %template(vectordouble) vector<double>;
//}

%import utils.i

// **************************************************************************
// some helper functions for the bindings
%{
#include <string>

// TODO: to be removed, only for tests
std::string _getStdString(std::string * pStr) {return *pStr;}
// TODO: to be removed, only for tests

bool _setStdString(std::string * pStr, const std::string & strValue) {
    if (pStr) {*pStr = strValue; return true;}
    return false;
}

//#include <iostream>

typedef std::ostream ostream;
typedef std::istream istream;

%}

// **************************************************************************
// This part is for the swig parser phase !
// This code will be used by swig to build up the type hirachy.

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++ now stats the wrapping of the ltilib header files +++
#ifndef SWIGPYTHON
%include "ltiLinConfig.h"
HANDLE_SIMPLE_HEADER_FILE("ltiTypes.h")                 // makes problems with byte type --> lti::byte and windows byte

%{
// anscheinend bug in ruby swig modul: int32 wird nicht lti::int32 wie in python modul
typedef unsigned int MYUINT32;
typedef int MYINT32;
typedef unsigned short int MYUINT16;
typedef short int MYINT16;
typedef unsigned char MYUBYTE;
typedef signed char MYBYTE;
#define byte MYBYTE
#define ubyte MYUBYTE
#define uint32 MYUINT32
#define int32 MYINT32
#define uint16 MYUINT16
#define int16 MYINT16
namespace lti {
typedef unsigned char MYUBYTE;
typedef signed char MYBYTE;
typedef unsigned short int MYUINT16;
typedef signed short int MYINT16;
typedef unsigned int MYUINT32;
typedef signed int MYINT32;
}
//using namespace lti;
//typedef lti::ubyte ubyte;
//typedef lti::byte byte;
//typedef lti::uint32 uint32;
//typedef lti::int32 int32;
//typedef lti::uint16 uint16;
//typedef lti::int16 int16;
//#define ubyte lti::ubyte
//#define byte lti::byte
//#define uint32 lti::uint32
//#define int32 lti::int32
//#define uint16 lti::uint16
//#define int16 lti::int16
%}
#else
// +++ wrapping of simple data types +++
// for successfull mapping of const ubyte & to simple data types !!!
//using namespace lti;              // WORKING (12.1.2006) removing this statement for ruby support !
namespace lti {
typedef unsigned char ubyte;
typedef signed char byte;
typedef unsigned short int uint16;
typedef signed short int int16;
typedef unsigned int uint32;
typedef signed int int32;
}
%{
// now the namespace lti is known !
#include "ltiTypes.h"
//typedef lti::int32 int32;
//typedef lti::uint32 uint32;
//typedef lti::int16 int16;
//typedef lti::uint16 uint16;
//typedef lti::ubyte ubyte;
//typedef lti::byte byte;
/*
namespace {
using namespace lti;              // WORKING (12.1.2006) removing this statement for ruby support !
typedef unsigned int uint32;
typedef lti::ubyte ubyte;
typedef lti::byte byte;
}
*/
/*
typedef unsigned int MYUINT32;
typedef int MYINT32;
typedef unsigned short int MYUINT16;
typedef short int MYINT16;
typedef unsigned char MYUBYTE;
typedef signed char MYBYTE;
#define byte MYBYTE
#define ubyte MYUBYTE
#define uint32 MYUINT32
#define int32 MYINT32
#define uint16 MYUINT16
#define int16 MYINT16
*/
%}
#endif
HANDLE_SIMPLE_HEADER_FILE("ltiObject.h")
HANDLE_SIMPLE_HEADER_FILE("ltiIoHandler.h")
HANDLE_SIMPLE_HEADER_FILE("ltiBoundaryType.h")
HANDLE_SIMPLE_HEADER_FILE("ltiIoObject.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMathObject.h")
HANDLE_SIMPLE_HEADER_FILE("ltiRGBPixel.h")
HANDLE_SIMPLE_HEADER_FILE("ltiRectangle.h")
// TODO --> hmm, hier gibt es noch Probleme mit der 'Reihenfolge' ?
//    #ifdef SWIGPYTHON
//    %{
//    #include "ltiRGBPixel.h"
//    %}
//    %extend lti::rgbPixel {
//        // add index support for python (Warning: this is Python-specific!) 
//    	const T & __getitem__( int index )
//    	{
//    		return self->at(index);
//    	}
//    	void __setitem__( int index, const T & value )
//    	{
//    		(*self)[index] = value;
//    	}
//        int __len__()
//        {
//            return self->size();
//        }
//    }
//    #endif
HANDLE_SIMPLE_HEADER_FILE("ltiPoint.h")
    #ifdef SWIGPYTHON
    %extend lti::tpoint {
        // add index support for python (Warning: this is Python-specific!) 
    	const T & __getitem__( int index )
    	{
    		return (*self)[index];
    	}
    	void __setitem__( int index, const T & value )
    	{
    		(*self)[index] = value;
    	}
        int __len__()
        {
            return self->size();
        }
    }
    #endif
    namespace lti {
        %template(ipoint) tpoint<int>;
        %template(fpoint) tpoint<float>;
        %template(dpoint) tpoint<double>;
        %template(ipoint3d) tpoint3D<int>;
        %template(fpoint3d) tpoint3D<float>;
        %template(dpoint3d) tpoint3D<double>;
    }
//#ifndef SWIGRUBY
%wrapper %{
namespace { using namespace lti;    // opening namespace using
%}

%template(list_ipoint) std::list<lti::ipoint>;
HANDLE_SIMPLE_HEADER_FILE("ltiPointList.h")
//%template(tpointList_int) lti::tpointList<int>;
//    %swig_list_methods(lti::tpointList<T>);       // --> TODO: in die Klasse aufnehmen...
    %extend lti::tpointList {
    // TODO: add a better (pythonic) support for iterators
    void * createIterator()
    {
        lti::tpointList<T>::iterator * pIter = new lti::tpointList<T>::iterator;
        (*pIter) = self->begin();
        return (void *) (pIter);
    }
    void deleteIterator(void *p) 
    {
        lti::tpointList<T>::iterator * pIter = (lti::tpointList<T>::iterator *)p;
        delete pIter;
    }
    bool isEnd(void *p) 
    {
        lti::tpointList<T>::iterator * pIter = (lti::tpointList<T>::iterator *)p;
        return *pIter == self->end();
    }
    lti::tpoint<T> nextElement(void * p) 
    {
        lti::tpointList<T>::iterator * pIter = (lti::tpointList<T>::iterator *)p;
        lti::tpoint<T> aPointOut = *(*pIter);
        ++(*pIter);
        return aPointOut;
    }
    }
    
    namespace lti {
        %template(pointList) lti::tpointList<int>;
    }
%wrapper %{
}  // closing namespace using
%}
//#endif
HANDLE_SIMPLE_HEADER_FILE("ltiLocation.h")
HANDLE_SIMPLE_HEADER_FILE("ltiPolygonPoints.h")
    namespace lti {
    //    %template(ipolygonPoints) tpolygonPoints<int>;        // PATCH in ltiPolygonPoints.h
    }
HANDLE_SIMPLE_HEADER_FILE("ltiGeometry.h")
    namespace lti {
    //TODO:    %template(iintersection) intersection<ipoint>;
    }
HANDLE_SIMPLE_HEADER_FILE("ltiGenericVector.h")
    #ifdef SWIGPYTHON
    %extend lti::genericVector {
        //%swig_container_methods(lti::genericVector);      // maybe a better way...
        // add index support for python (Warning: this is Python-specific!) 
    	const T & __getitem__( int index )
    	{
    		return self->at(index);
    	}
    	void __setitem__( int index, const T & value )
    	{
    		(*self)[index] = value;
    	}
        int __len__()
        {
            return self->size();
        }
    }
    #endif
    namespace lti {
        %template(dgenericVector) genericVector<double>;
        %template(fgenericVector) genericVector<float>;
        %template(igenericVector) genericVector<lti::int32>;
        %template(ugenericVector) genericVector<lti::ubyte>;
        %template(rgbgenericVector) genericVector<lti::rgbPixel>;
    }
HANDLE_SIMPLE_HEADER_FILE("ltiVector.h")
    namespace lti {
        %template(dvector) vector<double>;
        %template(fvector) vector<float>;
        %template(ivector) vector<lti::int32>;
        %template(uvector) vector<lti::ubyte>;
        %template(palette) vector<rgbPixel>;
    }
HANDLE_SIMPLE_HEADER_FILE("ltiArray.h")
    namespace lti {
        %template(iarray) array<lti::int32>;
        %template(farray) array<float>;
        %template(darray) array<double>;
        %template(uarray) array<lti::ubyte>;
    }
HANDLE_SIMPLE_HEADER_FILE("ltiGenericMatrix.h")
    #ifdef SWIGPYTHON
    %extend lti::genericMatrix {
        // add index support for python (Warning: this is Python-specific!) 
//    	const T & __getitem__( int index )
//    	{
//    		return self->at(index);
//    	}
    	const lti::genericVector<T> & __getitem__( int index )
    	{
    		return self->getRow(index);
    	}
    	void __setitem__( int index, const genericVector<T> & value )
    	{
    		(*self)[index] = value;
    	}
        int __len__()
        {
            return self->rows();
        }
        // TODO: check
        // The original at()-method makes some problems! is it because of 'inline' ?
        const T & at( int row, int col )
        {
            return self->at(row,col);
        }
        void setAt( int row, int col, const T & value )
        {
    		(*self)[row][col] = value;
        }
    }
    #endif
    namespace lti {
        %template(ugenericMatrix) genericMatrix<lti::ubyte>;
        %template(igenericMatrix) genericMatrix<lti::int32>;
        %template(fgenericMatrix) genericMatrix<float>;
        %template(dgenericMatrix) genericMatrix<double>;
        %template(rgbPixelgenericMatrix) genericMatrix<lti::rgbPixel>;
    }
HANDLE_SIMPLE_HEADER_FILE("ltiMatrix.h")
    namespace lti {
        %template(imatrix) matrix<lti::int32>;
        %template(fmatrix) matrix<float>;
        %template(dmatrix) matrix<double>;
        %template(umatrix) matrix<lti::ubyte>;
        %template(rgbPixelmatrix) matrix<lti::rgbPixel>;
    }
HANDLE_SIMPLE_HEADER_FILE("ltiHTypes.h")
    #ifdef SWIGPYTHON
    %extend lti::hMatrix {
        // add index support for python (Warning: this is Python-specific!) 
    //	const T & __getitem__( int index )
    //	{
    //		return (*self)[index];
    //	}
    //	void __setitem__( int index, const genericVector<T> & value )
    //	{
    //		(*self)[index] = value;
    //	}
    }
    #endif
    namespace lti {
    //    %template(fhPoint3D) hPoint3D<float>;
    //    %template(dhPoint3D) hPoint3D<double>;
    //    %template(fhPoint2D) hPoint2D<float>;
    //    %template(dhPoint2D) hPoint2D<double>;
    
        %template(fhBaseMatrix3D) hMatrix< float,hPoint3D<float> >;
        %template(dhBaseMatrix3D) hMatrix< double,hPoint3D<double> >;
        %template(fhBaseMatrix2D) hMatrix< float,hPoint2D<float> >;
        %template(dhBaseMatrix2D) hMatrix< double,hPoint2D<double> >;
    
        %template(fhMatrix3D) hMatrix3D<float>;
        %template(dhMatrix3D) hMatrix3D<double>;
        %template(fhMatrix2D) hMatrix2D<float>;
        %template(dhMatrix2D) hMatrix2D<double>;
    }


%wrapper %{
namespace { using namespace lti;    // opening namespace using
%}
HANDLE_SIMPLE_HEADER_FILE("ltiHistogram.h")
%wrapper %{
}  // closing namespace using
%}
HANDLE_SIMPLE_HEADER_FILE("ltiImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiContour.h")

/*
#ifdef SWIGRUBY
%include "ltiPointList.h"
HANDLE_SIMPLE_HEADER_FILE("ltiContour.h")
namespace lti {
%template(tpointList_int) tpointList<int>;
}
#endif
*/

// TODO PATCH    
//%include "_objectsFromMask_objectStruct.h"        // has to be included AFTER the definition of borderPoints !!!

#ifndef SWIGRUBY
    // TODO: ok: mit SWIG 1.3.21 !!! und SWIG 1.3.24 + VC7
    %template(list_ioPoints) std::list<lti::ioPoints>;
    %template(list_borderPoints) std::list<lti::borderPoints>;
    %template(list_areaPoints) std::list<lti::areaPoints>;
#endif
    
// TODO: add better tree support !
//%include "ltiTree.h"
//namespace lti {
//    //%template(tree_objectStruct) tree<objectsFromMask_objectStruct>;    // does not work because of a syntactical difference to tree<objectStruct>, unforunately is swig not so clever to handel that :-(
//    %template(tree_objectStruct) tree<objectStruct>;    
//}
////#define node tree_objectStruct_node
//%include "_tree_node.h"

HANDLE_SIMPLE_HEADER_FILE("ltiLinearKernels.h")
    namespace lti {
        %template(ikernel1D) kernel1D<lti::int32>;
        %template(fkernel1D) kernel1D<float>;
        %template(dkernel1D) kernel1D<double>;
        %template(ukernel1D) kernel1D<lti::ubyte>;
        %template(ikernel2D) kernel2D<lti::int32>;
        %template(fkernel2D) kernel2D<float>;
        %template(dkernel2D) kernel2D<double>;
        %template(ukernel2D) kernel2D<lti::ubyte>;
        %template(isepKernel) sepKernel<lti::int32>;
        %template(fsepKernel) sepKernel<float>;
        %template(dsepKernel) sepKernel<double>;
        %template(usepKernel) sepKernel<lti::ubyte>;
    }
HANDLE_SIMPLE_HEADER_FILE("ltiGradientKernels.h")
    namespace lti {
        // TODO %template(igradientKernelX) gradientKernelX<int>;
    }
HANDLE_SIMPLE_HEADER_FILE("ltiHessianKernels.h")
HANDLE_SIMPLE_HEADER_FILE("ltiLaplacianKernel.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSecondDerivativeKernels.h")
    namespace lti {
        %template(iandoKernelXX) andoKernelXX<int>;
        %template(iandoKernelXY) andoKernelXY<int>;
        %template(iandoKernelYY) andoKernelYY<int>;
        %template(fandoKernelXX) andoKernelXX<float>;
        %template(fandoKernelXY) andoKernelXY<float>;
        %template(fandoKernelYY) andoKernelYY<float>;
    }
//HANDLE_SIMPLE_HEADER_FILE("ltiSplitImage.h")          // --> colors.i
//HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageTorgI.h")

%{
//template <typename T> typedef lti::tpointList<T> tpointList<T>;     // kommt erst noch in der zukunft !
//TODO Template ! typedef lti::tpointList tpointList;
template <class T>
class tpointList : lti::tpointList<T>
{
};

template <class T>
class tpoint : lti::tpoint<T>
{
};

template <class T>
class trectangle : lti::trectangle<T>
{
};
/*
*/


typedef lti::pointList pointList;
typedef lti::irectangle irectangle;
typedef lti::rectangle rectangle;
//TODO Template ! typedef lti::trectangle trectangle;
typedef lti::eBoundaryType eBoundaryType;
typedef lti::location location;
%}
    
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++ now stats the wrapping of the ltilib functor header files +++

%include base_functors.i
%include io.i    
%include statistics.i    
%include filters.i
%include mask_op.i
%include colors.i    
%include segmentation.i    
%include edge_detectors.i
//%{
//typedef lti::thresholding thresholding;
//typedef lti::nonMaximaSuppression nonMaximaSuppression;
//// TODO template ! typedef lti::principalComponents principalComponents;
//template <class T>
//class principalComponents : lti::principalComponents<T>
//{
//};
//// TODO template ! kdTree
//template <class T1, class T2, class T3>
//class kdTree : lti::kdTree<T1,T2,T3>
//{
//};
//%}
%include feat_extr.i    
%include morph_op.i    
%include transformations.i
%include manipulation.i    
%include classifiers.i
%include colorspaces.i

////TODO: add better tree support !!!
////#define _tree tree
//#define _tree_objectStruct_node node
//#define _tree tree<objectStruct>
//namespace lti {
//typedef lti::tree<objectStruct>::node tree_objectStruct_node;
//}


// **************************************************************************

HANDLE_SIMPLE_HEADER_FILE("../src/lti_manual.h")

%extend lti::image {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}
%extend lti::matrix<lti::int32> {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}

