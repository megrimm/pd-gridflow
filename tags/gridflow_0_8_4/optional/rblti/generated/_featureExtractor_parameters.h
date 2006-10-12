#ifndef _RfeatureExtractor_parameters_h
#define _RfeatureExtractor_parameters_h

namespace lti {
namespace _featureExtractor {
class RfeatureExtractor_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RfeatureExtractor_parameters (  );

     RfeatureExtractor_parameters ( const RfeatureExtractor_parameters &  arg0 );

     ~RfeatureExtractor_parameters (  );

    const char * getTypeName (  ) const;

    RfeatureExtractor_parameters & copy ( const RfeatureExtractor_parameters &  arg0 );

    virtual functor::RfeatureExtractor_parameters * clone (  ) const;

};
}
typedef lti::_featureExtractor::RfeatureExtractor_parameters featureExtractor_parameters;
}
#endif

