#ifndef _RlocalMoments_parameters_h
#define _RlocalMoments_parameters_h

namespace lti {
namespace _localMoments {
class RlocalMoments_parameters : public lti::_localFeatureExtractor::RlocalFeatureExtractor_parameters
{
public:
     RlocalMoments_parameters (  );

     RlocalMoments_parameters ( const RlocalMoments_parameters &  arg0 );

     ~RlocalMoments_parameters (  );

    const char * getTypeName (  ) const;

    RlocalMoments_parameters & copy ( const RlocalMoments_parameters &  arg0 );

    RlocalMoments_parameters & operator= ( const RlocalMoments_parameters &  arg0 );

    virtual functor::RlocalMoments_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    eSamplingMethod samplingMethod  ;
    bool calculateMean  ;
    bool calculateVariance  ;
    bool calculateSkew  ;
    bool calculateKurtosis  ;
    float scale  ;
    int gridRadius  ;
    int numCircles  ;
    int numRays  ;
};
}
typedef lti::_localMoments::RlocalMoments_parameters localMoments_parameters;
}
#endif

