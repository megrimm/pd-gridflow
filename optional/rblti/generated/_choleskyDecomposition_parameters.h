#ifndef _RcholeskyDecomposition_parameters_h
#define _RcholeskyDecomposition_parameters_h

namespace lti {
namespace _choleskyDecomposition {
class RcholeskyDecomposition_parameters : public lti::_choleskyDecomposition::RcholeskyDecomposition_parameters
{
public:
     RcholeskyDecomposition_parameters (  );

     RcholeskyDecomposition_parameters ( const RcholeskyDecomposition_parameters &  arg0 );

     ~RcholeskyDecomposition_parameters (  );

    const char * getTypeName (  ) const;

    RcholeskyDecomposition_parameters & copy ( const RcholeskyDecomposition_parameters &  arg0 );

    RcholeskyDecomposition_parameters & operator= ( const RcholeskyDecomposition_parameters &  arg0 );

    virtual functor::RcholeskyDecomposition_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

};
}
typedef lti::_choleskyDecomposition::RcholeskyDecomposition_parameters choleskyDecomposition_parameters;
}
#endif

