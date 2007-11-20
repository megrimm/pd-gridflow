#ifndef _Rjacobi_parameters_h
#define _Rjacobi_parameters_h

namespace lti {
namespace _jacobi {
class Rjacobi_parameters : public lti::_eigenSystem::ReigenSystem_parameters
{
public:
     Rjacobi_parameters (  );

     Rjacobi_parameters ( const Rjacobi_parameters &  arg0 );

    virtual const char * getTypeName (  ) const;

    Rjacobi_parameters & copy ( const Rjacobi_parameters &  arg0 );

    virtual functor::Rjacobi_parameters * clone (  ) const;

    int dimensions  ;
};
}
typedef lti::_jacobi::Rjacobi_parameters jacobi_parameters;
}
#endif

