#ifndef _RmultiGeometricFeaturesFromMask_parameters_h
#define _RmultiGeometricFeaturesFromMask_parameters_h

namespace lti {
namespace _multiGeometricFeaturesFromMask {
class RmultiGeometricFeaturesFromMask_parameters : public lti::_geometricFeaturesFromMask::RgeometricFeaturesFromMask_parameters
{
public:
     RmultiGeometricFeaturesFromMask_parameters (  );

     RmultiGeometricFeaturesFromMask_parameters ( const RmultiGeometricFeaturesFromMask_parameters &  arg0 );

     ~RmultiGeometricFeaturesFromMask_parameters (  );

    const char * getTypeName (  ) const;

    RmultiGeometricFeaturesFromMask_parameters & copy ( const RmultiGeometricFeaturesFromMask_parameters &  arg0 );

    RmultiGeometricFeaturesFromMask_parameters & operator= ( const RmultiGeometricFeaturesFromMask_parameters &  arg0 );

    virtual functor::RmultiGeometricFeaturesFromMask_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    genericVector< bool > mergeEntries  ;
    ivector maskValues  ;
    bool manualPostProcess  ;
};
}
typedef lti::_multiGeometricFeaturesFromMask::RmultiGeometricFeaturesFromMask_parameters multiGeometricFeaturesFromMask_parameters;
}
#endif

