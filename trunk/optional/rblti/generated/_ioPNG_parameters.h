#ifndef _RioPNG_parameters_h
#define _RioPNG_parameters_h

namespace lti {
namespace _ioPNG {
class RioPNG_parameters : public lti::_ioFunctor::RioFunctor_parameters
{
public:
     RioPNG_parameters (  );

     RioPNG_parameters ( const RioPNG_parameters &  arg0 );

    RioPNG_parameters & copy ( const RioPNG_parameters &  arg0 );

    virtual functor::RioPNG_parameters * clone (  ) const;

    virtual const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int bitsPerPixel  ;
    int quantColors  ;
    bool useAlphaChannel  ;
};
}
typedef lti::_ioPNG::RioPNG_parameters ioPNG_parameters;
}
#endif

