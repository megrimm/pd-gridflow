#ifndef _RmatrixInversion_parameters_h
#define _RmatrixInversion_parameters_h

namespace lti {
namespace _matrixInversion {
class RmatrixInversion_parameters : public lti::_linearAlgebraFunctor::RlinearAlgebraFunctor_parameters
{
public:
    enum algorithmType {
LUD  =  0 
,SVD  =  1 
};

     RmatrixInversion_parameters ( void  arg0 );

     RmatrixInversion_parameters ( const RmatrixInversion_parameters &  arg0 );

     ~RmatrixInversion_parameters (  );

    const char * getTypeName (  ) const;

    RmatrixInversion_parameters & copy ( const RmatrixInversion_parameters &  arg0 );

    virtual functor::RmatrixInversion_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    algorithmType method  ;
};
}
typedef lti::_matrixInversion::RmatrixInversion_parameters matrixInversion_parameters;
}
#endif

