#ifndef _RcolorContrastGradient_parameters_h
#define _RcolorContrastGradient_parameters_h

namespace lti {
namespace _colorContrastGradient {
class RcolorContrastGradient_parameters : public lti::_gradientFunctor::RgradientFunctor_parameters
{
public:
    enum eContrastFormat {
MDD 
,Contrast 
,Maximum 
};

     RcolorContrastGradient_parameters (  );

     RcolorContrastGradient_parameters ( const RcolorContrastGradient_parameters &  arg0 );

     ~RcolorContrastGradient_parameters (  );

    const char * getTypeName (  ) const;

    RcolorContrastGradient_parameters & copy ( const RcolorContrastGradient_parameters &  arg0 );

    RcolorContrastGradient_parameters & operator= ( const RcolorContrastGradient_parameters &  arg0 );

    virtual functor::RcolorContrastGradient_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    eContrastFormat contrastFormat  ;
};
}
typedef lti::_colorContrastGradient::RcolorContrastGradient_parameters colorContrastGradient_parameters;
}
#endif

