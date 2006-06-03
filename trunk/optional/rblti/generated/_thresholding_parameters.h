#ifndef _Rthresholding_parameters_h
#define _Rthresholding_parameters_h

namespace lti {
namespace _thresholding {
class Rthresholding_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     Rthresholding_parameters (  );

     Rthresholding_parameters ( const Rthresholding_parameters &  arg0 );

     ~Rthresholding_parameters (  );

    const char * getTypeName (  ) const;

    Rthresholding_parameters & copy ( const Rthresholding_parameters &  arg0 );

    virtual functor::Rthresholding_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    bool keepOutRegion  ;
    bool keepInRegion  ;
    float inRegionValue  ;
    float outRegionValue  ;
    float highThreshold  ;
    float lowThreshold  ;
};
}
typedef lti::_thresholding::Rthresholding_parameters thresholding_parameters;
}
#endif

