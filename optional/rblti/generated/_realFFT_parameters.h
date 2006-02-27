#ifndef _RrealFFT_parameters_h
#define _RrealFFT_parameters_h

namespace lti {
namespace _realFFT {
class RrealFFT_parameters : public lti::_transform::Rtransform_parameters
{
public:
    enum eMode {
Cartesic 
,Polar 
};

     RrealFFT_parameters (  );

     RrealFFT_parameters ( const RrealFFT_parameters &  arg0 );

    RrealFFT_parameters & copy ( const RrealFFT_parameters &  arg0 );

    virtual functor::RrealFFT_parameters * clone (  ) const;

    virtual const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    eMode mode  ;
};
}
typedef lti::_realFFT::RrealFFT_parameters realFFT_parameters;
}
#endif

