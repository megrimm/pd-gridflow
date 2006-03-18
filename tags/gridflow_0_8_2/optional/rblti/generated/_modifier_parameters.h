#ifndef _Rmodifier_parameters_h
#define _Rmodifier_parameters_h

namespace lti {
namespace _modifier {
class Rmodifier_parameters : public lti::_functor::Rfunctor_parameters
{
public:
    const eBoundaryType Black  ;
    const eBoundaryType Mirror  ;
    const eBoundaryType Periodic  ;
    const eBoundaryType Constant  ;
    const eBoundaryType NoBoundary  ;
     Rmodifier_parameters (  );

     Rmodifier_parameters ( const Rmodifier_parameters &  arg0 );

    virtual const char * getTypeName (  ) const;

    Rmodifier_parameters & copy ( const Rmodifier_parameters &  arg0 );

    virtual functor::Rmodifier_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    eBoundaryType boundaryType  ;
};
}
typedef lti::_modifier::Rmodifier_parameters modifier_parameters;
}
#endif

