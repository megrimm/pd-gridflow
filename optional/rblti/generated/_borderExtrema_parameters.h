#ifndef _RborderExtrema_parameters_h
#define _RborderExtrema_parameters_h

namespace lti {
namespace _borderExtrema {
class RborderExtrema_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     RborderExtrema_parameters (  );

     RborderExtrema_parameters ( const RborderExtrema_parameters &  arg0 );

     ~RborderExtrema_parameters (  );

    const char * getTypeName (  ) const;

    RborderExtrema_parameters & copy ( const RborderExtrema_parameters &  arg0 );

    RborderExtrema_parameters & operator= ( const RborderExtrema_parameters &  arg0 );

    virtual functor::RborderExtrema_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int minTolerance  ;
    tpoint< double > center  ;
};
}
typedef lti::_borderExtrema::RborderExtrema_parameters borderExtrema_parameters;
}
#endif

