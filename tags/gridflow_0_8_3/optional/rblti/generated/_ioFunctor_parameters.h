#ifndef _RioFunctor_parameters_h
#define _RioFunctor_parameters_h

namespace lti {
namespace _ioFunctor {
class RioFunctor_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RioFunctor_parameters (  );

     RioFunctor_parameters ( const RioFunctor_parameters &  arg0 );

    RioFunctor_parameters & copy ( const RioFunctor_parameters &  arg0 );

    virtual functor::RioFunctor_parameters * clone (  ) const;

    virtual const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    std::string filename  ;
};
}
typedef lti::_ioFunctor::RioFunctor_parameters ioFunctor_parameters;
}
#endif

