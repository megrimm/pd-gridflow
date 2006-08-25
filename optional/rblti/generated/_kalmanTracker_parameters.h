#ifndef _RkalmanTracker_parameters_h
#define _RkalmanTracker_parameters_h

namespace lti {
namespace _kalmanTracker {
class RkalmanTracker_parameters : public lti::_kalmanFilter::RkalmanFilter_parameters
{
public:
     RkalmanTracker_parameters ( const int &  arg0 );

     RkalmanTracker_parameters ( const RkalmanTracker_parameters &  arg0 );

     ~RkalmanTracker_parameters (  );

    const char * getTypeName (  ) const;

    RkalmanTracker_parameters & copy ( const RkalmanTracker_parameters &  arg0 );

    RkalmanTracker_parameters & operator= ( const RkalmanTracker_parameters &  arg0 );

    virtual lti::functor::RkalmanTracker_parameters * clone (  ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    vector< float > initialSystemState  ;
    matrix< float > measurementNoiseCovariance  ;
    matrix< float > processNoiseCovariance  ;
    matrix< float > initialErrorCovariance  ;
    bool autoInitialize  ;
};
}
typedef lti::_kalmanTracker::RkalmanTracker_parameters kalmanTracker_parameters;
}
#endif

