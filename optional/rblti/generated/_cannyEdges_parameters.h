#ifndef _RcannyEdges_parameters_h
#define _RcannyEdges_parameters_h

namespace lti {
namespace _cannyEdges {
class RcannyEdges_parameters : public lti::_edgeDetector::RedgeDetector_parameters
{
public:
     RcannyEdges_parameters (  );

     RcannyEdges_parameters ( const RcannyEdges_parameters &  arg0 );

     ~RcannyEdges_parameters (  );

    const char * getTypeName (  ) const;

    RcannyEdges_parameters & copy ( const RcannyEdges_parameters &  arg0 );

    RcannyEdges_parameters & operator= ( const RcannyEdges_parameters &  arg0 );

    virtual functor::RcannyEdges_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    float variance  ;
    int kernelSize  ;
    float thresholdMin  ;
    float thresholdMax  ;
    lti_colorContrastGradient_parameters gradientParameters  ;
};
}
typedef lti::_cannyEdges::RcannyEdges_parameters cannyEdges_parameters;
}
#endif

