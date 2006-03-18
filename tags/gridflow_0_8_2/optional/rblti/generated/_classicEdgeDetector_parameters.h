#ifndef _RclassicEdgeDetector_parameters_h
#define _RclassicEdgeDetector_parameters_h

namespace lti {
namespace _classicEdgeDetector {
class RclassicEdgeDetector_parameters : public lti::_edgeDetector::RedgeDetector_parameters
{
public:
    enum eMaximaSearchMethod {
NonMaximaSuppression 
,Thresholding 
};

     RclassicEdgeDetector_parameters (  );

     RclassicEdgeDetector_parameters ( const RclassicEdgeDetector_parameters &  arg0 );

     ~RclassicEdgeDetector_parameters (  );

    const char * getTypeName (  ) const;

    RclassicEdgeDetector_parameters & copy ( const RclassicEdgeDetector_parameters &  arg0 );

    RclassicEdgeDetector_parameters & operator= ( const RclassicEdgeDetector_parameters &  arg0 );

    virtual functor::RclassicEdgeDetector_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    void setMaximaSearchMethod ( const thresholding &  arg0 );

    void setMaximaSearchMethod ( const nonMaximaSuppression &  arg0 );

    void attachMaximaSearchMethod ( thresholding *  arg0 );

    void attachMaximaSearchMethod ( nonMaximaSuppression *  arg0 );

    void useExternalMaximaSearchMethod ( thresholding *  arg0 );

    void useExternalMaximaSearchMethod ( nonMaximaSuppression *  arg0 );

    lti_gradientFunctor_parameters gradientParameters  ;
    
    
    
    
};
}
typedef lti::_classicEdgeDetector::RclassicEdgeDetector_parameters classicEdgeDetector_parameters;
}
#endif

