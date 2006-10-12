#ifndef _Rdilation_parameters_h
#define _Rdilation_parameters_h

namespace lti {
namespace _dilation {
class Rdilation_parameters : public lti::_morphology::Rmorphology_parameters
{
public:
    enum eMode {
Binary 
,Gray 
};

     Rdilation_parameters (  );

     Rdilation_parameters ( const Rdilation_parameters &  arg0 );

     ~Rdilation_parameters (  );

    const char * getTypeName (  ) const;

    Rdilation_parameters & copy ( const Rdilation_parameters &  arg0 );

    virtual functor::Rdilation_parameters * clone (  ) const;

    void setKernel ( const mathObject &  arg0 );

    const mathObject & getKernel (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    eMode mode  ;
    
};
}
typedef lti::_dilation::Rdilation_parameters dilation_parameters;
}
#endif

