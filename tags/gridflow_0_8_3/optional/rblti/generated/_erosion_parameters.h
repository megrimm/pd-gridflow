#ifndef _Rerosion_parameters_h
#define _Rerosion_parameters_h

namespace lti {
namespace _erosion {
class Rerosion_parameters : public lti::_morphology::Rmorphology_parameters
{
public:
    enum eMode {
Binary 
,Gray 
};

     Rerosion_parameters (  );

     Rerosion_parameters ( const Rerosion_parameters &  arg0 );

     ~Rerosion_parameters (  );

    const char * getTypeName (  ) const;

    Rerosion_parameters & copy ( const Rerosion_parameters &  arg0 );

    virtual functor::Rerosion_parameters * clone (  ) const;

    void setKernel ( const mathObject &  arg0 );

    const mathObject & getKernel (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    eMode mode  ;
    
};
}
typedef lti::_erosion::Rerosion_parameters erosion_parameters;
}
#endif

