#ifndef _RfastRelabeling_parameters_h
#define _RfastRelabeling_parameters_h

namespace lti {
namespace _fastRelabeling {
class RfastRelabeling_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
    int minThreshold  ;
    int maxThreshold  ;
    bool assumeLabeledMask  ;
    bool fourNeighborhood  ;
    bool sortSize  ;
    int minimumObjectSize  ;
     RfastRelabeling_parameters (  );

     RfastRelabeling_parameters ( const RfastRelabeling_parameters &  arg0 );

     ~RfastRelabeling_parameters (  );

    const char * getTypeName (  ) const;

    RfastRelabeling_parameters & copy ( const RfastRelabeling_parameters &  arg0 );

    RfastRelabeling_parameters & operator= ( const RfastRelabeling_parameters &  arg0 );

    virtual functor::RfastRelabeling_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

};
}
typedef lti::_fastRelabeling::RfastRelabeling_parameters fastRelabeling_parameters;
}
#endif

