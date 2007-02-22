//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

//%include utils.i



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
