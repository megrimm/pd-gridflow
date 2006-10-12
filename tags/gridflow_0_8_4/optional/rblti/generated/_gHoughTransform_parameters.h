#ifndef _RgHoughTransform_parameters_h
#define _RgHoughTransform_parameters_h

namespace lti {
namespace _gHoughTransform {
class RgHoughTransform_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     RgHoughTransform_parameters (  );

     RgHoughTransform_parameters ( const RgHoughTransform_parameters &  arg0 );

     ~RgHoughTransform_parameters (  );

    const char * getTypeName (  ) const;

    RgHoughTransform_parameters & copy ( const RgHoughTransform_parameters &  arg0 );

    RgHoughTransform_parameters & operator= ( const RgHoughTransform_parameters &  arg0 );

    virtual functor::RgHoughTransform_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    float thresholdLocalMaximum  ;
    int kernelSizeTranslation  ;
    int kernelSizeRotation  ;
    int kernelSizeScale  ;
    int numberOfObjects  ;
    float thresholdEdgePixel  ;
    bool findLocalMaximum  ;
    int disSize  ;
    int disRotation  ;
    float extension  ;
    int numberOfAngleLists  ;
};
}
typedef lti::_gHoughTransform::RgHoughTransform_parameters gHoughTransform_parameters;
}
#endif

