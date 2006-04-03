#ifndef _RcubicSpline_parameters_h
#define _RcubicSpline_parameters_h

namespace lti {
namespace _cubicSpline {
class RcubicSpline_parameters : public lti::_variablySpacedSampleInterpolator::RvariablySpacedSampleInterpolator_parameters
{
public:
     RcubicSpline_parameters (  );

     RcubicSpline_parameters ( const RcubicSpline_parameters &  arg0 );

     ~RcubicSpline_parameters (  );

    const char * getTypeName (  ) const;

    RcubicSpline_parameters & copy ( const RcubicSpline_parameters &  arg0 );

    RcubicSpline_parameters & operator= ( const RcubicSpline_parameters &  arg0 );

    virtual functor::RcubicSpline_parameters * clone (  ) const;

    bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    bool read ( ioHandler &  arg0, const bool &  arg1 );

    bool useNaturalDerivatives  ;
    double derivativeAtFirstPoint  ;
    double derivativeAtLastPoint  ;
    tpointList< T > samplingPoints  ;
};
}
typedef lti::_cubicSpline::RcubicSpline_parameters cubicSpline_parameters;
}
#endif

