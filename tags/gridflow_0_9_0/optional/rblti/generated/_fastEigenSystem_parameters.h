#ifndef _RfastEigenSystem_parameters_h
#define _RfastEigenSystem_parameters_h

namespace lti {
namespace _fastEigenSystem {
class RfastEigenSystem_parameters : public lti::_linearAlgebraFunctor::RlinearAlgebraFunctor_parameters
{
public:
     RfastEigenSystem_parameters (  );

     RfastEigenSystem_parameters ( const RfastEigenSystem_parameters &  arg0 );

    virtual const char * getTypeName (  ) const;

    RfastEigenSystem_parameters & copy ( const RfastEigenSystem_parameters &  arg0 );

    virtual functor::RfastEigenSystem_parameters * clone (  ) const;

    int dimensions  ;
    bool useRRR  ;
};
}
typedef lti::_fastEigenSystem::RfastEigenSystem_parameters fastEigenSystem_parameters;
}
#endif

