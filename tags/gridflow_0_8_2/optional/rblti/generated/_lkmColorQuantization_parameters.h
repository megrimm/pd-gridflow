#ifndef _RlkmColorQuantization_parameters_h
#define _RlkmColorQuantization_parameters_h

namespace lti {
namespace _lkmColorQuantization {
class RlkmColorQuantization_parameters : public lti::_colorQuantization::RcolorQuantization_parameters
{
public:
     RlkmColorQuantization_parameters (  );

     RlkmColorQuantization_parameters ( const RlkmColorQuantization_parameters &  arg0 );

     ~RlkmColorQuantization_parameters (  );

    const char * getTypeName (  ) const;

    RlkmColorQuantization_parameters & copy ( const RlkmColorQuantization_parameters &  arg0 );

    RlkmColorQuantization_parameters & operator= ( const RlkmColorQuantization_parameters &  arg0 );

    virtual functor::RlkmColorQuantization_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    double neighbour  ;
    double learnRate  ;
    double shrinkLearnRate  ;
    int maxIterations  ;
};
}
typedef lti::_lkmColorQuantization::RlkmColorQuantization_parameters lkmColorQuantization_parameters;
}
#endif

