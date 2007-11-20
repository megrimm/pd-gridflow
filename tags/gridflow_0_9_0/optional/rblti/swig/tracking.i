//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$

%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i

HANDLE_FUNCTOR_WITH_PARAMETERS( kalmanFilter,                 "ltiKalmanFilter.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( kalmanTracker,                "ltiKalmanTracker.h")
//HANDLE_FUNCTOR_WITH_PARAMETERS( blobEM,                       "ltiBlobEM.h")

//These functions give errors in the gcc compilation stage, ignored for now (ie not wrapped)
%ignore write(ioHandler&, const blobEM::blobEM_gaussEllipse&, const bool);
%ignore read(ioHandler&, blobEM::blobEM_gaussEllipse&, const bool);
%ignore write(ioHandler&, const blobEM::blobEM_gaussEllipse&);
%ignore read(ioHandler&, blobEM::blobEM_gaussEllipse&);
namespace lti {
//%rename(init) blobEM::initialize (const channel8 &, const std::vector< blobEM_gaussEllipse > &);
//%rename(init) blobEM::initialize (const channel8 &, const int &);
%rename(init) blobEM::initialize;
}

%{
#include "ltiBlobEM.h"
#define _blobEM blobEM
#define RblobEM_parameters parameters
#define RblobEM_gaussEllipse gaussEllipse
namespace lti {
  typedef lti::blobEM::parameters blobEM_parameters;
  typedef lti::blobEM::gaussEllipse blobEM_gaussEllipse;
}
%}

#define parameters blobEM_parameters
#define gaussEllipse blobEM_gaussEllipse
%include _blobEM_parameters.h
%include _blobEM_gaussEllipse.h
%include ltiBlobEM.h
#undef parameters
#undef gaussEllipse
namespace std {
  %template(vector_gaussEllipse) vector<lti::blobEM_gaussEllipse>;
}

HANDLE_FUNCTOR_WITH_PARAMETERS( blobEMTracker,                "blobEMTracker.h")
