#ifndef _RpolygonApproximation_parameters_h
#define _RpolygonApproximation_parameters_h

namespace lti {
namespace _polygonApproximation {
class RpolygonApproximation_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
     RpolygonApproximation_parameters (  );

     RpolygonApproximation_parameters ( const RpolygonApproximation_parameters &  arg0 );

     ~RpolygonApproximation_parameters (  );

    const char * getTypeName (  ) const;

    RpolygonApproximation_parameters & copy ( const RpolygonApproximation_parameters &  arg0 );

    RpolygonApproximation_parameters & operator= ( const RpolygonApproximation_parameters &  arg0 );

    virtual functor::RpolygonApproximation_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int minStep  ;
    bool searchMaxDistance  ;
    double maxDistance  ;
    bool closed  ;
};
}
typedef lti::_polygonApproximation::RpolygonApproximation_parameters polygonApproximation_parameters;
}
#endif

