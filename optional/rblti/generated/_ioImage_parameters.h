#ifndef _RioImage_parameters_h
#define _RioImage_parameters_h

namespace lti {
namespace _ioImage {
class RioImage_parameters : public lti::_ioFunctor::RioFunctor_parameters
{
public:
     RioImage_parameters (  );

     RioImage_parameters ( const RioImage_parameters &  arg0 );

    RioImage_parameters & copy ( const RioImage_parameters &  arg0 );

    virtual functor::RioImage_parameters * clone (  ) const;

    virtual const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    lti_ioBMP_parameters paramBMP  ;
    lti_ioJPEG_parameters paramJPEG  ;
    lti_ioPNG_parameters paramPNG  ;
};
}
typedef lti::_ioImage::RioImage_parameters ioImage_parameters;
}
#endif

