#ifndef _RviewerBase_parameters_h
#define _RviewerBase_parameters_h

namespace lti {
namespace _viewerBase {
class RviewerBase_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RviewerBase_parameters (  );

     RviewerBase_parameters ( const RviewerBase_parameters &  arg0 );

    virtual ~RviewerBase_parameters (  );

    RviewerBase_parameters & copy ( const RviewerBase_parameters &  arg0 );

    RviewerBase_parameters & operator= ( const RviewerBase_parameters &  arg0 );

    virtual functor::RviewerBase_parameters * clone (  ) const;

    const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    std::string title  ;
};
}
typedef lti::_viewerBase::RviewerBase_parameters viewerBase_parameters;
}
#endif

