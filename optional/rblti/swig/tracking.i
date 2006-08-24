 
HANDLE_FUNCTOR_WITH_PARAMETERS( kalmanFilter,                 "ltiKalmanFilter.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( kalmanTracker,                "ltiKalmanTracker.h")

//HANDLE_FUNCTOR_WITH_PARAMETERS( blobEM,                       "ltiBlobEM.h")

//These functions give errors in the gcc compilation stage, ignored for now (ie not wrapped)
%ignore write(ioHandler&, const blobEM::blobEM_gaussEllipse&, const bool);
%ignore read(ioHandler&, blobEM::blobEM_gaussEllipse&, const bool);
%ignore write(ioHandler&, const blobEM::blobEM_gaussEllipse&);
%ignore read(ioHandler&, blobEM::blobEM_gaussEllipse&);

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
//#undef gaussEllipse

