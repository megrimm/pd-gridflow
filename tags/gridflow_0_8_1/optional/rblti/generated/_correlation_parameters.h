#ifndef _Rcorrelation_parameters_h
#define _Rcorrelation_parameters_h

namespace lti {
namespace _correlation {
class Rcorrelation_parameters : public lti::_filter::Rfilter_parameters
{
public:
    enum eMode {
Classic 
,Coefficient 
,C1 
,C2 
,C3 
};

     Rcorrelation_parameters (  );

     Rcorrelation_parameters ( const Rcorrelation_parameters &  arg0 );

    virtual ~Rcorrelation_parameters (  );

    const char * getTypeName (  ) const;

    Rcorrelation_parameters & copy ( const Rcorrelation_parameters &  arg0 );

    virtual functor::Rcorrelation_parameters * clone (  ) const;

    const mathObject & getKernel (  ) const;

    void setKernel ( const mathObject &  arg0 );

    const channel8 & getMask (  ) const;

    void setMask ( const channel8 &  arg0 );

    void setUseMask ( const bool  arg0 );

    const bool & getUseMask (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    const double & getKernelAverage (  ) const;

    const int & getKernelSize (  ) const;

    eMode mode  ;
    mathObject * kernel  ;
    const channel8 * mask  ;
    bool useMask  ;
    
    
};
}
typedef lti::_correlation::Rcorrelation_parameters correlation_parameters;
}
#endif

