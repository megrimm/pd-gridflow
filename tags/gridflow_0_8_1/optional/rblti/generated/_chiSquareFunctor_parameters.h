#ifndef _RchiSquareFunctor_parameters_h
#define _RchiSquareFunctor_parameters_h

namespace lti {
namespace _chiSquareFunctor {
class RchiSquareFunctor_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RchiSquareFunctor_parameters (  );

     RchiSquareFunctor_parameters ( const RchiSquareFunctor_parameters &  arg0 );

     ~RchiSquareFunctor_parameters (  );

    const char * getTypeName (  ) const;

    RchiSquareFunctor_parameters & copy ( const RchiSquareFunctor_parameters &  arg0 );

    RchiSquareFunctor_parameters & operator= ( const RchiSquareFunctor_parameters &  arg0 );

    virtual functor::RchiSquareFunctor_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    double accuracy  ;
    bool discrete  ;
    bool equidistant  ;
    int maxNumOfIntervalls  ;
    int maxSteps  ;
    double mean  ;
    double minimalWidth  ;
    std::string nameOfInfoFile  ;
    bool saveInfoFile  ;
    bool useBetterMeanAndVar  ;
    double variance  ;
};
}
typedef lti::_chiSquareFunctor::RchiSquareFunctor_parameters chiSquareFunctor_parameters;
}
#endif

