#ifndef _RgeometricTransform_parameters_h
#define _RgeometricTransform_parameters_h

namespace lti {
namespace _geometricTransform {
class RgeometricTransform_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
    void invert (  );

    void scale ( const tpoint< double > &  arg0 );

    void scale ( const tpoint3D< double > &  arg0 );

    void scale ( const double &  arg0 );

    void shift ( const tpoint< double > &  arg0 );

    void shift ( const tpoint3D< double > &  arg0 );

    void rotate ( const vector< double > &  arg0, const vector< double > &  arg1, const double &  arg2 );

    void rotate ( const tpoint3D< double > &  arg0, const tpoint3D< double > &  arg1, const double &  arg2 );

    void rotate ( const double &  arg0 );

    void clear ( void  arg0 );

     RgeometricTransform_parameters (  );

     RgeometricTransform_parameters ( const RgeometricTransform_parameters &  arg0 );

     ~RgeometricTransform_parameters (  );

    const char * getTypeName (  ) const;

    RgeometricTransform_parameters & copy ( const RgeometricTransform_parameters &  arg0 );

    RgeometricTransform_parameters & operator= ( const RgeometricTransform_parameters &  arg0 );

    virtual functor::RgeometricTransform_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    hMatrix3D< float > transMatrix  ;
    bool keepDimensions  ;
    eInterpolatorType interpolator  ;
};
}
typedef lti::_geometricTransform::RgeometricTransform_parameters geometricTransform_parameters;
}
#endif

