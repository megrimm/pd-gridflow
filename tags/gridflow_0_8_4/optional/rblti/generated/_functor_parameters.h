#ifndef _Rfunctor_parameters_h
#define _Rfunctor_parameters_h

namespace lti {
namespace _functor {
class Rfunctor_parameters : public lti::ioObject
{
public:
     Rfunctor_parameters (  );

     Rfunctor_parameters ( const Rfunctor_parameters &  arg0 );

    virtual ~Rfunctor_parameters (  );

    Rfunctor_parameters & copy ( const Rfunctor_parameters &  arg0 );

    virtual Rfunctor_parameters * clone (  ) const = 0;

    const char * getTypeName (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    
};
}
typedef lti::_functor::Rfunctor_parameters functor_parameters;
}
#endif

