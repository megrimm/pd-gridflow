#ifndef _RfastLineExtraction_parameters_h
#define _RfastLineExtraction_parameters_h

namespace lti {
namespace _fastLineExtraction {
class RfastLineExtraction_parameters : public lti::_featureExtractor::RfeatureExtractor_parameters
{
public:
     RfastLineExtraction_parameters (  );

     RfastLineExtraction_parameters ( const RfastLineExtraction_parameters &  arg0 );

     ~RfastLineExtraction_parameters (  );

    const char * getTypeName (  ) const;

    RfastLineExtraction_parameters & copy ( const RfastLineExtraction_parameters &  arg0 );

    RfastLineExtraction_parameters & operator= ( const RfastLineExtraction_parameters &  arg0 );

    functor::RfastLineExtraction_parameters * clone (  ) const;

    bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    bool read ( ioHandler &  arg0, const bool  arg1 );

    float maxQuantizationError  ;
    int segmentTolerance  ;
    int maxSegmentGap  ;
    int minLineLen  ;
    int minSegmLen  ;
};
}
typedef lti::_fastLineExtraction::RfastLineExtraction_parameters fastLineExtraction_parameters;
}
#endif

