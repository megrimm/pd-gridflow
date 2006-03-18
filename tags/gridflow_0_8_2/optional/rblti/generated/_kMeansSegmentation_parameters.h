#ifndef _RkMeansSegmentation_parameters_h
#define _RkMeansSegmentation_parameters_h

namespace lti {
namespace _kMeansSegmentation {
class RkMeansSegmentation_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
    enum eSmoothFilterType {
Nothing  =  0 
,Median  =  1 
,KNearest  =  2 
};

     RkMeansSegmentation_parameters (  );

     RkMeansSegmentation_parameters ( const int  arg0 );

     RkMeansSegmentation_parameters ( const RkMeansSegmentation_parameters &  arg0 );

     ~RkMeansSegmentation_parameters (  );

    const char * getTypeName (  ) const;

    RkMeansSegmentation_parameters & copy ( const RkMeansSegmentation_parameters &  arg0 );

    RkMeansSegmentation_parameters & operator= ( const RkMeansSegmentation_parameters &  arg0 );

    virtual functor::RkMeansSegmentation_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    lti_kMColorQuantization_parameters quantParameters  ;
    eSmoothFilterType smoothFilter  ;
    int kernelSize  ;
};
}
typedef lti::_kMeansSegmentation::RkMeansSegmentation_parameters kMeansSegmentation_parameters;
}
#endif

