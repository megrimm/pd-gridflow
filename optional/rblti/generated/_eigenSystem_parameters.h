#ifndef _ReigenSystem_parameters_h
#define _ReigenSystem_parameters_h

namespace lti {
namespace _eigenSystem {
class ReigenSystem_parameters : public lti::_linearAlgebraFunctor::RlinearAlgebraFunctor_parameters
{
public:
     ReigenSystem_parameters (  );

     ReigenSystem_parameters ( const ReigenSystem_parameters &  arg0 );

    virtual const char * getTypeName (  ) const;

    ReigenSystem_parameters & copy ( const ReigenSystem_parameters &  arg0 );

    virtual functor::ReigenSystem_parameters * clone (  ) const;

    bool sort  ;
};
}
typedef lti::_eigenSystem::ReigenSystem_parameters eigenSystem_parameters;
}
#endif

