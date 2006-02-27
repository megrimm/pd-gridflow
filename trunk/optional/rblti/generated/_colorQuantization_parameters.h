#ifndef _RcolorQuantization_parameters_h
#define _RcolorQuantization_parameters_h

namespace lti {
namespace _colorQuantization {
class RcolorQuantization_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RcolorQuantization_parameters (  );

     RcolorQuantization_parameters ( const RcolorQuantization_parameters &  arg0 );

     ~RcolorQuantization_parameters (  );

    const char * getTypeName (  ) const;

    RcolorQuantization_parameters & copy ( const RcolorQuantization_parameters &  arg0 );

    RcolorQuantization_parameters & operator= ( const RcolorQuantization_parameters &  arg0 );

    virtual functor::RcolorQuantization_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int numberOfColors  ;
};
}
typedef lti::_colorQuantization::RcolorQuantization_parameters colorQuantization_parameters;
}
#endif

