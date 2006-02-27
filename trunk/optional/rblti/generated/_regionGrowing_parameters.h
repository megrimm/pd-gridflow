#ifndef _RregionGrowing_parameters_h
#define _RregionGrowing_parameters_h

namespace lti {
namespace _regionGrowing {
class RregionGrowing_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
    enum eMode {
UseGivenThresholds 
,GetThresholdsRelative 
,GetThresholdsAbsolute 
};

    enum eBackgroundType {
Dark 
,Medium 
,Light 
};

     RregionGrowing_parameters (  );

     RregionGrowing_parameters ( const RregionGrowing_parameters &  arg0 );

     ~RregionGrowing_parameters (  );

    const char * getTypeName (  ) const;

    RregionGrowing_parameters & copy ( const RregionGrowing_parameters &  arg0 );

    virtual functor::RregionGrowing_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    eBackgroundType backgroundType  ;
    bool useGaussKernel  ;
    int localStatisticsKernelSize  ;
    double localStatisticsKernelVariance  ;
    int smoothingKernelSize  ;
    float smoothingThreshold  ;
    trectangle< float > patchPosition  ;
    eMode mode  ;
    int scaleFactor  ;
    float averageThreshold  ;
    trgbPixel< float > averageThresholds  ;
    float edgesThreshold  ;
    vector< tpoint< float > > seedPoints  ;
};
}
typedef lti::_regionGrowing::RregionGrowing_parameters regionGrowing_parameters;
}
#endif

