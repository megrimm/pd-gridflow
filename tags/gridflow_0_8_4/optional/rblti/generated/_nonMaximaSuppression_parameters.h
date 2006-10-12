#ifndef _RnonMaximaSuppression_parameters_h
#define _RnonMaximaSuppression_parameters_h

namespace lti {
namespace _nonMaximaSuppression {
class RnonMaximaSuppression_parameters : public lti::_transform::Rtransform_parameters
{
public:
    bool fillGaps  ;
    ubyte endPointValue  ;
    ubyte gapValue  ;
    int numGapHints  ;
    int maxGapLength  ;
     RnonMaximaSuppression_parameters (  );

     RnonMaximaSuppression_parameters ( const RnonMaximaSuppression_parameters &  arg0 );

     ~RnonMaximaSuppression_parameters (  );

    const char * getTypeName (  ) const;

    RnonMaximaSuppression_parameters & copy ( const RnonMaximaSuppression_parameters &  arg0 );

    RnonMaximaSuppression_parameters & operator= ( const RnonMaximaSuppression_parameters &  arg0 );

    virtual functor::RnonMaximaSuppression_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    float thresholdMin  ;
    bool indirectThresholdMin  ;
    float thresholdMax  ;
    bool indirectThresholdMax  ;
    ubyte background  ;
    ubyte edgeValue  ;
    bool checkAngles  ;
    int gradientHistogramSize  ;
};
}
typedef lti::_nonMaximaSuppression::RnonMaximaSuppression_parameters nonMaximaSuppression_parameters;
}
#endif

