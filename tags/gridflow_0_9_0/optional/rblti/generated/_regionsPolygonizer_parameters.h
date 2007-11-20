#ifndef _RregionsPolygonizer_parameters_h
#define _RregionsPolygonizer_parameters_h

namespace lti {
namespace _regionsPolygonizer {
class RregionsPolygonizer_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RregionsPolygonizer_parameters (  );

     RregionsPolygonizer_parameters ( const RregionsPolygonizer_parameters &  arg0 );

     ~RregionsPolygonizer_parameters (  );

    const char * getTypeName (  ) const;

    RregionsPolygonizer_parameters & copy ( const RregionsPolygonizer_parameters &  arg0 );

    RregionsPolygonizer_parameters & operator= ( const RregionsPolygonizer_parameters &  arg0 );

    virtual functor::RregionsPolygonizer_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    double maxPixelDistance  ;
    bool detectNeighbors  ;
};
}
typedef lti::_regionsPolygonizer::RregionsPolygonizer_parameters regionsPolygonizer_parameters;
}
#endif

