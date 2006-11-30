#ifndef _RblobEMTracker_parameters_h
#define _RblobEMTracker_parameters_h

namespace lti {
namespace _blobEMTracker {
class RblobEMTracker_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RblobEMTracker_parameters ();
     ~RblobEMTracker_parameters (  );
     RblobEMTracker_parameters (const RblobEMTracker_parameters&);
     functor::RblobEMTracker_parameters * clone (  ) const;
     int minSize;
};
}
typedef lti::_blobEMTracker::RblobEMTracker_parameters blobEMTracker_parameters;
}
#endif

