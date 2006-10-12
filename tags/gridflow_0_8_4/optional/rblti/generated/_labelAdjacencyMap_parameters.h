#ifndef _RlabelAdjacencyMap_parameters_h
#define _RlabelAdjacencyMap_parameters_h

namespace lti {
namespace _labelAdjacencyMap {
class RlabelAdjacencyMap_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RlabelAdjacencyMap_parameters (  );

     RlabelAdjacencyMap_parameters ( const RlabelAdjacencyMap_parameters &  arg0 );

     ~RlabelAdjacencyMap_parameters (  );

    const char * getTypeName (  ) const;

    RlabelAdjacencyMap_parameters & copy ( const RlabelAdjacencyMap_parameters &  arg0 );

    RlabelAdjacencyMap_parameters & operator= ( const RlabelAdjacencyMap_parameters &  arg0 );

    virtual functor::RlabelAdjacencyMap_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    bool minColors  ;
    palette thePalette  ;
    int neighborhood  ;
    const palette defaultPalette  ;
};
}
typedef lti::_labelAdjacencyMap::RlabelAdjacencyMap_parameters labelAdjacencyMap_parameters;
}
#endif

