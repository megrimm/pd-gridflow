#ifndef _RgeometricFeaturesFromMask_parameters_h
#define _RgeometricFeaturesFromMask_parameters_h

namespace lti {
namespace _geometricFeaturesFromMask {
class RgeometricFeaturesFromMask_parameters : public lti::_fastRelabeling::RfastRelabeling_parameters
{
public:
     RgeometricFeaturesFromMask_parameters (  );

     RgeometricFeaturesFromMask_parameters ( const RgeometricFeaturesFromMask_parameters &  arg0 );

     ~RgeometricFeaturesFromMask_parameters (  );

    const char * getTypeName (  ) const;

    RgeometricFeaturesFromMask_parameters & copy ( const RgeometricFeaturesFromMask_parameters &  arg0 );

    RgeometricFeaturesFromMask_parameters & operator= ( const RgeometricFeaturesFromMask_parameters &  arg0 );

    virtual functor::RgeometricFeaturesFromMask_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int nBest  ;
    int minimumMergedObjectSize  ;
    point minimumDistance  ;
    bool merge  ;
};
}
typedef lti::_geometricFeaturesFromMask::RgeometricFeaturesFromMask_parameters geometricFeaturesFromMask_parameters;
}
#endif

