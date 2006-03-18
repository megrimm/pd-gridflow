#ifndef _RvariablySpacedSamplesInterpolator_parameters_h
#define _RvariablySpacedSamplesInterpolator_parameters_h

namespace lti {
namespace _variablySpacedSamplesInterpolator {
class RvariablySpacedSamplesInterpolator_parameters : public lti::_interpolator::Rinterpolator_parameters
{
public:
     RvariablySpacedSamplesInterpolator_parameters (  );

     RvariablySpacedSamplesInterpolator_parameters ( const RvariablySpacedSamplesInterpolator_parameters &  arg0 );

     ~RvariablySpacedSamplesInterpolator_parameters (  );

    const char * getTypeName (  ) const;

    RvariablySpacedSamplesInterpolator_parameters & copy ( const RvariablySpacedSamplesInterpolator_parameters &  arg0 );

    RvariablySpacedSamplesInterpolator_parameters & operator= ( const RvariablySpacedSamplesInterpolator_parameters &  arg0 );

    virtual functor::RvariablySpacedSamplesInterpolator_parameters * clone (  ) const = 0;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

};
}
typedef lti::_variablySpacedSamplesInterpolator::RvariablySpacedSamplesInterpolator_parameters variablySpacedSamplesInterpolator_parameters;
}
#endif

