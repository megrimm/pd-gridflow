%module ltimath

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
%import "ltiLinConfig.h"
IMPORT_SIMPLE_HEADER_FILE("ltiTypes.h")                 // makes problems with byte type --> lti::byte and windows byte


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


IMPORT_SIMPLE_HEADER_FILE ("ltiObject.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiIoHandler.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiBoundaryType.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiStatus.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiIoObject.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiMathObject.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiFunctor.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiModifier.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiFilter.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiIOFunctor.h")

IMPORT_SIMPLE_HEADER_FILE ("ltiGenericVector.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiVector.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiArray.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiGenericMatrix.h")
IMPORT_SIMPLE_HEADER_FILE ("ltiMatrix.h")



// Base Classes
IMPORT_FUNCTOR_WITH_PARAMETERS( functor,                 "ltiFunctor.h")
IMPORT_FUNCTOR_WITH_PARAMETERS( modifier,                "ltiModifier.h")
IMPORT_FUNCTOR_WITH_PARAMETERS( filter,                  "ltiFilter.h")
IMPORT_FUNCTOR_WITH_PARAMETERS( ioFunctor,               "ltiIOFunctor.h")


//%include base_functors.i

/******************************************************************************************/
/*                                   REAL MATH.I					  */
/******************************************************************************************/

%define HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(func_name,functor_header)
%{
#include functor_header
#define _ ## func_name func_name<float>
#define R ## func_name ## _parameters parameters
namespace lti {
typedef lti:: ## func_name<float> ## ::parameters func_name ## _parameters;
}
%}
#define parameters func_name ## _parameters
%include _ ## func_name ## _parameters.h
%include functor_header
#undef parameters
%enddef

HANDLE_FUNCTOR_WITH_PARAMETERS(linearAlgebraFunctor,    "ltiLinearAlgebraFunctor.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(matrixInversion,     "ltiMatrixInversion.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(choleskyDecomposition,"ltiCholeskyDecomposition.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(eigenSystem,       "ltiEigenSystemBase.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(jacobi,            "ltiEigenSystem.h")

namespace lti {
%template(MatrixInversion) matrixInversion<float>;
%template(CholeskyDecomposition) choleskyDecomposition<float>;
%template(EigenSystem) eigenSystem<float>;
%template(Jacobi) jacobi<float>;
}




