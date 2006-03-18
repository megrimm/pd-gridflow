#ifndef _RgradientFunctor_parameters_h
#define _RgradientFunctor_parameters_h

namespace lti {
namespace _gradientFunctor {
class RgradientFunctor_parameters : public lti::_transform::Rtransform_parameters
{
public:
    enum eOutputFormat {
Polar 
,Cartesic 
};

    enum eKernelType {
Optimal 
,OGD 
,Difference 
,Roberts 
,Sobel 
,Prewitt 
,Robinson 
,Kirsch 
,Harris 
};

     RgradientFunctor_parameters (  );

     RgradientFunctor_parameters ( const RgradientFunctor_parameters &  arg0 );

     ~RgradientFunctor_parameters (  );

    const char * getTypeName (  ) const;

    RgradientFunctor_parameters & copy ( const RgradientFunctor_parameters &  arg0 );

    RgradientFunctor_parameters & operator= ( const RgradientFunctor_parameters &  arg0 );

    virtual functor::RgradientFunctor_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    eOutputFormat format  ;
    eKernelType kernelType  ;
    int gradientKernelSize  ;
    float ogdVariance  ;
};
}
typedef lti::_gradientFunctor::RgradientFunctor_parameters gradientFunctor_parameters;
}
#endif

