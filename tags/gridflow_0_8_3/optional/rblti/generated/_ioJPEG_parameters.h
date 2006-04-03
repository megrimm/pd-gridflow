#ifndef _RioJPEG_parameters_h
#define _RioJPEG_parameters_h

namespace lti {
namespace _ioJPEG {
class RioJPEG_parameters : public lti::_ioFunctor::RioFunctor_parameters
{
public:
     RioJPEG_parameters (  );

     RioJPEG_parameters ( const RioJPEG_parameters &  arg0 );

    RioJPEG_parameters & copy ( const RioJPEG_parameters &  arg0 );

    virtual functor::RioJPEG_parameters * clone (  ) const;

    virtual const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int quality  ;
    bool progressive  ;
    std::string comment  ;
    int rowsPerRestart  ;
};
}
typedef lti::_ioJPEG::RioJPEG_parameters ioJPEG_parameters;
}
#endif

