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
//  Revision 1.1  2006/07/26 14:49:12  matju
//  added eigenSystem etc (don't work)
//
//
//******************************************************************************

//%include utils.i

HANDLE_FUNCTOR_WITH_PARAMETERS(matrixInversion,"ltiMatrixInversion.h")
HANDLE_FUNCTOR_WITH_PARAMETERS(eigenSystem,    "ltiEigenSystem.h")

namespace lti {
%template(MatrixInversionF) matrixInversion<float>;
%template(MatrixInversionD) matrixInversion<double>;
%template(EigenSystemF) eigenSystem<float>;
%template(EigenSystemD) eigenSystem<double>;
}
