#ifndef _RlocalFeatureExtractor_parameters_h
#define _RlocalFeatureExtractor_parameters_h

namespace lti {
namespace _localFeatureExtractor {
class RlocalFeatureExtractor_parameters : public lti::_featureExtractor::RfeatureExtractor_parameters
{
public:
    enum eSamplingMethod {
FixedRaySampling 
,FixedGridSampling 
};

     RlocalFeatureExtractor_parameters (  );

     RlocalFeatureExtractor_parameters ( const RlocalFeatureExtractor_parameters &  arg0 );

     ~RlocalFeatureExtractor_parameters (  );

    const char * getTypeName (  ) const;

    RlocalFeatureExtractor_parameters & copy ( const RlocalFeatureExtractor_parameters &  arg0 );

    RlocalFeatureExtractor_parameters & operator= ( const RlocalFeatureExtractor_parameters &  arg0 );

    virtual functor::RlocalFeatureExtractor_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

};
}
typedef lti::_localFeatureExtractor::RlocalFeatureExtractor_parameters localFeatureExtractor_parameters;
}
#endif

