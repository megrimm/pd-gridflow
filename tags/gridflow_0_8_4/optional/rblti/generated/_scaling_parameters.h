#ifndef _Rscaling_parameters_h
#define _Rscaling_parameters_h

namespace lti {
namespace _scaling {
class Rscaling_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     Rscaling_parameters (  );

     Rscaling_parameters ( const Rscaling_parameters &  arg0 );

     ~Rscaling_parameters (  );

    const char * getTypeName (  ) const;

    Rscaling_parameters & copy ( const Rscaling_parameters &  arg0 );

    Rscaling_parameters & operator= ( const Rscaling_parameters &  arg0 );

    virtual functor::Rscaling_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    tpoint< float > scale  ;
    eInterpolatorType interpolatorType  ;
};
}
typedef lti::_scaling::Rscaling_parameters scaling_parameters;
}
#endif

