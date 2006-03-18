#ifndef _RwhiteningSegmentation_parameters_h
#define _RwhiteningSegmentation_parameters_h

namespace lti {
namespace _whiteningSegmentation {
class RwhiteningSegmentation_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
     RwhiteningSegmentation_parameters (  );

     RwhiteningSegmentation_parameters ( const RwhiteningSegmentation_parameters &  arg0 );

     ~RwhiteningSegmentation_parameters (  );

    const char * getTypeName (  ) const;

    RwhiteningSegmentation_parameters & copy ( const RwhiteningSegmentation_parameters &  arg0 );

    RwhiteningSegmentation_parameters & operator= ( const RwhiteningSegmentation_parameters &  arg0 );

    virtual functor::RwhiteningSegmentation_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    lti_kMeansSegmentation_parameters quantNormal  ;
    lti_kMeansSegmentation_parameters quantTransformed  ;
    int minRegionSize  ;
};
}
typedef lti::_whiteningSegmentation::RwhiteningSegmentation_parameters whiteningSegmentation_parameters;
}
#endif

