#ifndef _Rconvolution_parameters_h
#define _Rconvolution_parameters_h

namespace lti {
namespace _convolution {
class Rconvolution_parameters : public lti::_filter::Rfilter_parameters
{
public:
     Rconvolution_parameters (  );

     Rconvolution_parameters ( const Rconvolution_parameters &  arg0 );

    virtual ~Rconvolution_parameters (  );

    const char * getTypeName (  ) const;

    Rconvolution_parameters & copy ( const Rconvolution_parameters &  arg0 );

    virtual functor::Rconvolution_parameters * clone (  ) const;

    const mathObject & getKernel (  ) const;

    void setKernel ( const mathObject &  arg0 );

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    
};
}
typedef lti::_convolution::Rconvolution_parameters convolution_parameters;
}
#endif

