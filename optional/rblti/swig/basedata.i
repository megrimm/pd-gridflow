//******************************************************************************
//
//      project:  pylti
//    copyright:  (c) 2005 by Michael Neuroth
//
//  description:  python modul for ltilib (using SWIG)
//
//******************************************************************************

#ifndef SWIGIMPORTED
//%module basedata
%module rblti
%import utils.i
%include dep.i
#endif

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


using namespace std;

// sollte nach dem include der std_*.i Dateien stehen, ansonsten gibt swig einen Fehlercode zurueck !
//%feature("autodoc","1")

std::string _getStdString(std::string * pStr);
bool _setStdString(std::string * pStr, const std::string & strValue);


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
%}
#endif
HANDLE_SIMPLE_HEADER_FILE("ltiObject.h")
HANDLE_SIMPLE_HEADER_FILE("ltiIoHandler.h")
HANDLE_SIMPLE_HEADER_FILE("ltiBoundaryType.h")
HANDLE_SIMPLE_HEADER_FILE("ltiStatus.h")
HANDLE_SIMPLE_HEADER_FILE("ltiIoObject.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMathObject.h")
HANDLE_SIMPLE_HEADER_FILE("ltiRGBPixel.h")

namespace lti {
%template(frgbPixel) trgbPixel<float>;

}



HANDLE_SIMPLE_HEADER_FILE("ltiPoint.h")

namespace lti {
  %template(ipoint) tpoint<int>;
  %template(fpoint) tpoint<float>;
  %template(dpoint) tpoint<double>;
  %template(ipoint3d) tpoint3D<int>;
  %template(fpoint3d) tpoint3D<float>;
  %template(dpoint3d) tpoint3D<double>;
}


HANDLE_SIMPLE_HEADER_FILE("ltiRectangle.h")
namespace lti {
  %template(irect)  trectangle<int>;
  %template(frect) trectangle<float>;
  %template(drect) trectangle<double>;
}


/*
%wrapper %{
namespace { using namespace lti;    // opening namespace using
%}
*/
/*****************************************************************************/
/*                          MATRIX  starts                                   */
/*****************************************************************************/

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
	
	%rename(substract) genericMatrix<lti::ubyte>::subtract(const matrix<lti::ubyte> &, const matrix<lti::ubyte> &);
	%rename(substract) genericMatrix<lti::ubyte>::subtract(const lti::ubyte);
	%rename(substract) genericMatrix<lti::ubyte>::subtract(const matrix<lti::ubyte> &, const lti::ubyte);
	
    }

/*****************************************************************************/
/*                          MATRIX  ends                                     */
/*****************************************************************************/


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

    //fills result with the content of the pointlist
    void to_matrix(lti::matrix<int> &result){
        if (((result.size()).y != self->size()) || (2 != (result.size()).x))
           result.resize(self->size(),2);

        std::list<lti::tpoint<int> >::const_iterator iter = self->begin() ;
        for(int j=0; iter != self->end(); iter++, j++)
        {
           result.at(j,0)=(*iter).y;
           result.at(j,1)=(*iter).x;
        }
    }

    //Fills the pointlist with the content of source
    void fill(const lti::matrix<int> &source){
        if (0 != self->size())
           self->erase(self->begin(), self->end());
        int sz = (source.size()).x;
        if (2 != sz)
        {
           printf("Pointlist: expected 2 columns in matrix but got %d\n", sz);
           exit (1);
        }
        sz = (source.size()).y;
        for(int j=0; j< sz ; j++)
           self->push_back(lti::tpoint<int>(source.at(j,1), source.at(j,0)));
    }

    lti::tpoint<T> at(int pos){
        std::list<lti::tpoint<T> >::const_iterator iter = self->begin() ;
        int sz = self->size();
        if (pos >= sz) {
           iter = self->end();
           iter--;
        }
        else {
           for(int j=0; (j<pos) && (iter != self->end()); j++)
              iter++;
        }
        return (*iter);
    }
    }
    
    namespace lti {
        %template(pointList) lti::tpointList<int>;
    }
/*
%wrapper %{
}  // closing namespace using
%}
*/
//#endif

HANDLE_SIMPLE_HEADER_FILE("ltiLocation.h")
HANDLE_SIMPLE_HEADER_FILE("ltiPolygonPoints.h")
HANDLE_SIMPLE_HEADER_FILE("ltiGeometry.h")
    namespace lti {
    //TODO:    %template(iintersection) intersection<ipoint>;
    }
namespace lti {
%ignore genericVector<int>::castFrom(const genericVector<rgbPixel> &);
%ignore genericVector<float>::castFrom(const genericVector<rgbPixel> &);
%ignore genericVector<double>::castFrom(const genericVector<rgbPixel> &);
%ignore genericVector<rgbPixel>::castFrom(const genericVector<float> &);
%ignore genericVector<rgbPixel>::castFrom(const genericVector<double> &);

%ignore genericMatrix<int>::castFrom(const genericMatrix<rgbPixel> &);
%ignore genericMatrix<float>::castFrom(const genericMatrix<rgbPixel> &);
%ignore genericMatrix<double>::castFrom(const genericMatrix<rgbPixel> &);
%ignore genericMatrix<rgbPixel>::castFrom(const genericMatrix<float> &);
%ignore genericMatrix<rgbPixel>::castFrom(const genericMatrix<double> &);
}



HANDLE_SIMPLE_HEADER_FILE("ltiHTypes.h")


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

namespace lti{
%ignore image::image(const int& ,const int& ,const rgbPixel[]);
%ignore channel8::channel8(const int& ,const int& ,const ubyte[]);
%ignore channel::channel(const int& ,const int& ,const float[]);
}
HANDLE_SIMPLE_HEADER_FILE("ltiImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiContour.h")


%template(list_ioPoints) std::list<lti::ioPoints>;
%template(list_borderPoints) std::list<lti::borderPoints>;
%template(list_areaPoints) std::list<lti::areaPoints>;

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
HANDLE_SIMPLE_HEADER_FILE("ltiBinaryKernels.h")

namespace lti {
%template (icityBlockKernel) cityBlockKernel<int>;
%template (ucityBlockKernel) cityBlockKernel<lti::ubyte>;
%template (fcityBlockKernel) cityBlockKernel<float>;
%template (dcityBlockKernel) cityBlockKernel<double>;
}


%{
//template <typename T> typedef lti::tpointList<T> tpointList<T>;     // kommt erst noch in der zukunft !
//TODO Template ! typedef lti::tpointList tpointList;MeanshiftTracker
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
/*
%include base_functors.i
%include io.i    
%include statistics.i    
%include filters.i
%include math.i
%include mask_op.i
%include colors.i    
%include segmentation.i    
%include edge_detectors.i
%include feat_extr.i    
%include morph_op.i    
%include transformations.i
%include manipulation.i    
%include classifiers.i
%include colorspaces.i
%include drawing.i
*/


// **************************************************************************

HANDLE_SIMPLE_HEADER_FILE("../src/lti_manual.h")

%extend lti::image {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}

%extend lti::matrix<lti::int32> {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}

%extend lti::channel {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}

%extend lti::channel8 {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}

%extend lti::channel32 {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}

%extend lti::matrix<double> {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}

%extend lti::matrix<float> {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}

%extend lti::matrix<lti::ubyte> {
  long meat() {return ((unsigned long)(void *)&(self->at(0,0)))>>2;}
}

%extend lti::vector<lti::rgbPixel> {
  long meat() {return ((unsigned long)(void *)&(self->at(0)  ))>>2;}
}


namespace lti {
%extend image {
  image castFromRgbPixelMatrix(matrix<rgbPixel> other)
  {
  lti::rgbPixel pixel(0,0,0);
  self->resize(other.rows(),other.columns(),pixel,false,false);
  int y;
  for (y=0;y<self->rows();y++) {
      (self->getRow(y)).castFrom(other.getRow(y));
      }
  
  return (*self);
  }
}
}



