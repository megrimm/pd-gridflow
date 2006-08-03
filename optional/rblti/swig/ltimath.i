%module ltimath
%include utils.i
%include dep.i
%import basedata.i
#define IMPORTMODE
%include base_functors.i
#undef IMPORTMODE

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

HANDLE_FUNCTOR_WITH_PARAMETERS(linearAlgebraFunctor,            "ltiLinearAlgebraFunctor.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(matrixInversion,        "ltiMatrixInversion.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(choleskyDecomposition,  "ltiCholeskyDecomposition.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(eigenSystem,            "ltiEigenSystemBase.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(jacobi,                 "ltiEigenSystem.h")

namespace lti {
%template(MatrixInversion) matrixInversion<float>;
%template(CholeskyDecomposition) choleskyDecomposition<float>;
%template(EigenSystem) eigenSystem<float>;
%template(Jacobi) jacobi<float>;
}





