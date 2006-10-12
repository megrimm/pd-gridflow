#ifndef _RlocalMaxima_parameters_h
#define _RlocalMaxima_parameters_h

namespace lti {
namespace _localMaxima {
class RlocalMaxima_parameters : public lti::_maximumFilter::RmaximumFilter_parameters
{
public:
     RlocalMaxima_parameters (  );

     RlocalMaxima_parameters ( const RlocalMaxima_parameters &  arg0 );

     ~RlocalMaxima_parameters (  );

    const char * getTypeName (  ) const;

    RlocalMaxima_parameters & copy ( const RlocalMaxima_parameters &  arg0 );

    RlocalMaxima_parameters & operator= ( const RlocalMaxima_parameters &  arg0 );

    virtual functor::RlocalMaxima_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    float relativeMinimum  ;
    T noMaxValue  ;
    float hystheresisThreshold  ;
    int maxNumber  ;
};
}
typedef lti::_localMaxima::RlocalMaxima_parameters localMaxima_parameters;
}
#endif

