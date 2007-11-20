#ifndef _RblobEM_parameters_h
#define _RblobEM_parameters_h

namespace lti {
namespace _blobEM {
class RblobEM_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RblobEM_parameters (  );

     RblobEM_parameters ( const RblobEM_parameters &  arg0 );

     ~RblobEM_parameters (  );

    const char * getTypeName (  ) const;

    RblobEM_parameters & copy ( const RblobEM_parameters &  arg0 );

    RblobEM_parameters & operator= ( const RblobEM_parameters &  arg0 );

    virtual functor::RblobEM_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    int maxIterations  ;
    double convergenceThreshold  ;
};
}
typedef lti::_blobEM::RblobEM_parameters blobEM_parameters;
}
#endif

