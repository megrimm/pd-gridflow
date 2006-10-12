#ifndef _RmeanshiftTracker_parameters_h
#define _RmeanshiftTracker_parameters_h

namespace lti {
namespace _meanshiftTracker {
class RmeanshiftTracker_parameters : public lti::_modifier::Rmodifier_parameters
{
public:
    enum eKernelType {
Normal 
,Epanechnikov 
};

     RmeanshiftTracker_parameters (  );

     RmeanshiftTracker_parameters ( const RmeanshiftTracker_parameters &  arg0 );

     ~RmeanshiftTracker_parameters (  );

    const char * getTypeName (  ) const;

    RmeanshiftTracker_parameters & copy ( const RmeanshiftTracker_parameters &  arg0 );

    RmeanshiftTracker_parameters & operator= ( const RmeanshiftTracker_parameters &  arg0 );

    virtual functor::RmeanshiftTracker_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    float precision  ;
    float threshold  ;
    eKernelType kernelType  ;
    float sizeAdaptRatio  ;
};
}
typedef lti::_meanshiftTracker::RmeanshiftTracker_parameters meanshiftTracker_parameters;
}
#endif

