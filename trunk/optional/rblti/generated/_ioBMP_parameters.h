#ifndef _RioBMP_parameters_h
#define _RioBMP_parameters_h

namespace lti {
namespace _ioBMP {
class RioBMP_parameters : public lti::_ioFunctor::RioFunctor_parameters
{
public:
     RioBMP_parameters (  );

     RioBMP_parameters ( const RioBMP_parameters &  arg0 );

    RioBMP_parameters & copy ( const RioBMP_parameters &  arg0 );

    virtual functor::RioBMP_parameters * clone (  ) const;

    virtual const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    bool compression  ;
    int bitsPerPixel  ;
    int quantColors  ;
};
}
typedef lti::_ioBMP::RioBMP_parameters ioBMP_parameters;
}
#endif

