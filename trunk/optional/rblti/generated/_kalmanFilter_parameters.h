#ifndef _RkalmanFilter_parameters_h
#define _RkalmanFilter_parameters_h

namespace lti {
namespace _kalmanFilter {
class RkalmanFilter_parameters : public lti::_functor::Rfunctor_parameters
{
public:
     RkalmanFilter_parameters (  );

     RkalmanFilter_parameters ( int  arg0, int  arg1, int  arg2 );

     RkalmanFilter_parameters ( const RkalmanFilter_parameters &  arg0 );

     ~RkalmanFilter_parameters (  );

    const char * getTypeName (  ) const;

    RkalmanFilter_parameters & copy ( const RkalmanFilter_parameters &  arg0 );

    RkalmanFilter_parameters & operator= ( const RkalmanFilter_parameters &  arg0 );

    virtual functor::RkalmanFilter_parameters * clone (  ) const;

    bool consistent (  ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    vector< float > initialSystemState  ;
    matrix< float > dynamicsMatrix  ;
    matrix< float > controlMatrix  ;
    matrix< float > measurementMatrix  ;
    matrix< float > measurementNoiseCovariance  ;
    matrix< float > processNoiseCovariance  ;
    matrix< float > initialErrorCovariance  ;
};
}
typedef lti::_kalmanFilter::RkalmanFilter_parameters kalmanFilter_parameters;
}
#endif

