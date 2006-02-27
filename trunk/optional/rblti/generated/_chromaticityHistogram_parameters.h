#ifndef _RchromaticityHistogram_parameters_h
#define _RchromaticityHistogram_parameters_h

namespace lti {
namespace _chromaticityHistogram {
class RchromaticityHistogram_parameters : public lti::_globalFeatureExtractor::RglobalFeatureExtractor_parameters
{
public:
    enum eNormType {
NoNorm 
,L1 
,L2 
};

     RchromaticityHistogram_parameters (  );

     RchromaticityHistogram_parameters ( const RchromaticityHistogram_parameters &  arg0 );

     ~RchromaticityHistogram_parameters (  );

    const char * getTypeName (  ) const;

    RchromaticityHistogram_parameters & copy ( const RchromaticityHistogram_parameters &  arg0 );

    virtual functor::RchromaticityHistogram_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    float verticalNeighbourhood  ;
    float horizontalNeighbourhood  ;
    eNormType norm  ;
    int smoothingKernelSize  ;
    int greenCells  ;
    int redCells  ;
    rgbPixel backgroundColor  ;
};
}
typedef lti::_chromaticityHistogram::RchromaticityHistogram_parameters chromaticityHistogram_parameters;
}
#endif

