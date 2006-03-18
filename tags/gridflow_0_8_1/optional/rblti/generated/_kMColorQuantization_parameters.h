#ifndef _RkMColorQuantization_parameters_h
#define _RkMColorQuantization_parameters_h

namespace lti {
namespace _kMColorQuantization {
class RkMColorQuantization_parameters : public lti::_colorQuantization::RcolorQuantization_parameters
{
public:
     RkMColorQuantization_parameters (  );

     RkMColorQuantization_parameters ( const RkMColorQuantization_parameters &  arg0 );

     ~RkMColorQuantization_parameters (  );

    const char * getTypeName (  ) const;

    RkMColorQuantization_parameters & copy ( const RkMColorQuantization_parameters &  arg0 );

    RkMColorQuantization_parameters & operator= ( const RkMColorQuantization_parameters &  arg0 );

    virtual functor::RkMColorQuantization_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int maximalNumberOfIterations  ;
    float thresholdDeltaPalette  ;
};
}
typedef lti::_kMColorQuantization::RkMColorQuantization_parameters kMColorQuantization_parameters;
}
#endif

