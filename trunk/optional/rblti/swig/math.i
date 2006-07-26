//******************************************************************************
//
//      project:  pylti
//    copyright:  (c) 2005 by Michael Neuroth
//                (c) 2006 Mathieu Bouchard & Heri Andria
//
//  $Source$
//  $Revision$
//  $Date$
// 
//  $Log$
//  Revision 1.3  2006/07/26 16:27:17  matju
//  attempt to un-!@#$ template...
//
//  Revision 1.2  2006/07/26 15:40:13  heri
//  Required parent class.
//
//  Revision 1.1  2006/07/26 14:49:12  matju
//  added eigenSystem etc (don't work)
//
//
//******************************************************************************

//%include utils.i

%define HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(func_name,functor_header)
%{
#include functor_header
#define _ ## func_name func_name
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

HANDLE_FUNCTOR_WITH_PARAMETERS(linearAlgebraFunctor, "ltiLinearAlgebraFunctor.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(matrixInversion,"ltiMatrixInversion.h")
HANDLE_FUNCTOR_TEMPLATE_WITH_PARAMETERS(eigenSystem,    "ltiEigenSystem.h")

/*
namespace lti {
%template(MatrixInversionF) matrixInversion<float>;
%template(MatrixInversionD) matrixInversion<double>;
%template(EigenSystemF) eigenSystem<float>;
%template(EigenSystemD) eigenSystem<double>;
}
*/
