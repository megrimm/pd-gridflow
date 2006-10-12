#ifndef _Rinterpolator_parameters_h
#define _Rinterpolator_parameters_h

namespace lti {
namespace _interpolator {
class Rinterpolator_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     Rinterpolator_parameters (  );

     Rinterpolator_parameters ( const eBoundaryType  arg0 );

     Rinterpolator_parameters ( const Rinterpolator_parameters &  arg0 );

     ~Rinterpolator_parameters (  );

    const char * getTypeName (  ) const;

    Rinterpolator_parameters & copy ( const Rinterpolator_parameters &  arg0 );

    Rinterpolator_parameters & operator= ( const Rinterpolator_parameters &  arg0 );

    virtual functor::Rinterpolator_parameters * clone (  ) const = 0;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    eBoundaryType boundaryType  ;
};
}
typedef lti::_interpolator::Rinterpolator_parameters interpolator_parameters;
}
#endif

