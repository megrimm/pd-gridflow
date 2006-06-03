#ifndef _RthresholdSegmentation_parameters_h
#define _RthresholdSegmentation_parameters_h

namespace lti {
namespace _thresholdSegmentation {
class RthresholdSegmentation_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
     RthresholdSegmentation_parameters ( const bool &  arg0 );

     RthresholdSegmentation_parameters ( const RthresholdSegmentation_parameters &  arg0 );

     ~RthresholdSegmentation_parameters (  );

    const char * getTypeName (  ) const;

    RthresholdSegmentation_parameters & copy ( const RthresholdSegmentation_parameters &  arg0 );

    virtual functor::RthresholdSegmentation_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    float inRegionValue  ;
    float outRegionValue  ;
    float highThreshold  ;
    float lowThreshold  ;
    const float Original  ;
};
}
typedef lti::_thresholdSegmentation::RthresholdSegmentation_parameters thresholdSegmentation_parameters;
}
#endif

