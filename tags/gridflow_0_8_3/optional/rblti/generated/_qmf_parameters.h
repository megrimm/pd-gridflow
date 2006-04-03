#ifndef _Rqmf_parameters_h
#define _Rqmf_parameters_h

namespace lti {
namespace _qmf {
class Rqmf_parameters : public lti::_filter::Rfilter_parameters
{
public:
     Rqmf_parameters (  );

     Rqmf_parameters ( const Rqmf_parameters &  arg0 );

     ~Rqmf_parameters (  );

    const char * getTypeName (  ) const;

    Rqmf_parameters & copy ( const Rqmf_parameters &  arg0 );

    Rqmf_parameters & operator= ( const Rqmf_parameters &  arg0 );

    virtual functor::Rqmf_parameters * clone (  ) const;

    const mathObject & getKernel (  ) const;

    void setKernel ( const mathObject &  arg0 );

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    kernel1D< float > kernel  ;
    int levels  ;
};
}
typedef lti::_qmf::Rqmf_parameters qmf_parameters;
}
#endif

