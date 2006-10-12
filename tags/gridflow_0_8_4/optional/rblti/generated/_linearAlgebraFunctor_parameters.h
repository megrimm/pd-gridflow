#ifndef _RlinearAlgebraFunctor_parameters_h
#define _RlinearAlgebraFunctor_parameters_h

namespace lti {
namespace _linearAlgebraFunctor {
class RlinearAlgebraFunctor_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RlinearAlgebraFunctor_parameters (  );

     RlinearAlgebraFunctor_parameters ( const RlinearAlgebraFunctor_parameters &  arg0 );

    virtual ~RlinearAlgebraFunctor_parameters (  );

    const char * getTypeName (  ) const;

    RlinearAlgebraFunctor_parameters & copy ( const RlinearAlgebraFunctor_parameters &  arg0 );

    virtual functor::RlinearAlgebraFunctor_parameters * clone (  ) const;

    bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    bool read ( ioHandler &  arg0, const bool  arg1 );

};
}
typedef lti::_linearAlgebraFunctor::RlinearAlgebraFunctor_parameters linearAlgebraFunctor_parameters;
}
#endif

