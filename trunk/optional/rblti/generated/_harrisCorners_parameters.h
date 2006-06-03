#ifndef _RharrisCorners_parameters_h
#define _RharrisCorners_parameters_h

namespace lti {
namespace _harrisCorners {
class RharrisCorners_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     RharrisCorners_parameters (  );

     RharrisCorners_parameters ( const RharrisCorners_parameters &  arg0 );

     ~RharrisCorners_parameters (  );

    const char * getTypeName (  ) const;

    RharrisCorners_parameters & copy ( const RharrisCorners_parameters &  arg0 );

    RharrisCorners_parameters & operator= ( const RharrisCorners_parameters &  arg0 );

    virtual functor::RharrisCorners_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    float variance  ;
    int kernelSize  ;
    int maximumCorners  ;
    float scale  ;
    lti_localMaxima< float >_parameters localMaximaParameters  ;
    lti_gradientFunctor_parameters gradientFunctorParameters  ;
};
}
typedef lti::_harrisCorners::RharrisCorners_parameters harrisCorners_parameters;
}
#endif

