#ifndef _RmeanShiftSegmentation_parameters_h
#define _RmeanShiftSegmentation_parameters_h

namespace lti {
namespace _meanShiftSegmentation {
class RmeanShiftSegmentation_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
    enum  {
Quantization  =  0 
,Oversegmentation  =  1 
,Undersegmentation  =  2 
};

    int option  ;
    std::vector< rectangle > rects  ;
    int maxTrial  ;
    int trial2converge  ;
    double classThreshold [3] ;
    int maxTrialRandomColor  ;
    int minRegionSize  ;
    float rectRadius [3] ;
    float autoRadius [3] ;
    float minVar  ;
    enum eSpeedUpType {
NoSpeedup 
,MediumSpeedup 
,HighSpeedup 
};

    bool multivariateNormalKernel  ;
    eSpeedUpType speedup  ;
    double sigmaS  ;
    double sigmaR  ;
    double maxNeighbourColorDistance  ;
    double thresholdConverged  ;
    bool classicAlgorithm  ;
     RmeanShiftSegmentation_parameters (  );

     RmeanShiftSegmentation_parameters ( const RmeanShiftSegmentation_parameters &  arg0 );

     ~RmeanShiftSegmentation_parameters (  );

    const char * getTypeName (  ) const;

    RmeanShiftSegmentation_parameters & copy ( const RmeanShiftSegmentation_parameters &  arg0 );

    RmeanShiftSegmentation_parameters & operator= ( const RmeanShiftSegmentation_parameters &  arg0 );

    virtual functor::RmeanShiftSegmentation_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

};
}
typedef lti::_meanShiftSegmentation::RmeanShiftSegmentation_parameters meanShiftSegmentation_parameters;
}
#endif

