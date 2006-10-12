#ifndef _RglobalFeatureExtractor_parameters_h
#define _RglobalFeatureExtractor_parameters_h

namespace lti {
namespace _globalFeatureExtractor {
class RglobalFeatureExtractor_parameters : public lti::_featureExtractor::RfeatureExtractor_parameters
{
public:
     RglobalFeatureExtractor_parameters (  );

     RglobalFeatureExtractor_parameters ( const RglobalFeatureExtractor_parameters &  arg0 );

     ~RglobalFeatureExtractor_parameters (  );

    const char * getTypeName (  ) const;

    RglobalFeatureExtractor_parameters & copy ( const RglobalFeatureExtractor_parameters &  arg0 );

    virtual functor::RglobalFeatureExtractor_parameters * clone (  ) const;

};
}
typedef lti::_globalFeatureExtractor::RglobalFeatureExtractor_parameters globalFeatureExtractor_parameters;
}
#endif

