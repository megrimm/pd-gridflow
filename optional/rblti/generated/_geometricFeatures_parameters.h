#ifndef _RgeometricFeatures_parameters_h
#define _RgeometricFeatures_parameters_h

namespace lti {
namespace _geometricFeatures {
class RgeometricFeatures_parameters : public lti::_globalFeatureExtractor::RglobalFeatureExtractor_parameters
{
public:
    enum eBoundaryDefinition {
CentralBoundary 
,OuterBoundary 
,Approximation 
};

     RgeometricFeatures_parameters (  );

     RgeometricFeatures_parameters ( const RgeometricFeatures_parameters &  arg0 );

     ~RgeometricFeatures_parameters (  );

    const char * getTypeName (  ) const;

    RgeometricFeatures_parameters & copy ( const RgeometricFeatures_parameters &  arg0 );

    virtual functor::RgeometricFeatures_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    bool calcFeatureGroup1  ;
    bool calcFeatureGroup2  ;
    bool calcFeatureGroup3  ;
    bool calcFeatureGroup4  ;
    eBoundaryDefinition boundaryDefinition  ;
};
}
typedef lti::_geometricFeatures::RgeometricFeatures_parameters geometricFeatures_parameters;
}
#endif

