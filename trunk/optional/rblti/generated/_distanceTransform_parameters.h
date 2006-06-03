#ifndef _RdistanceTransform_parameters_h
#define _RdistanceTransform_parameters_h

namespace lti {
namespace _distanceTransform {
class RdistanceTransform_parameters : public lti::_morphology::Rmorphology_parameters
{
public:
    enum eDistanceType {
EightNeighborhood 
,FourNeighborhood 
,EuclideanSqr 
,Euclidean 
,EightSED 
,EightSEDSqr 
,FourSED 
,FourSEDSqr 
};

     RdistanceTransform_parameters (  );

     RdistanceTransform_parameters ( const RdistanceTransform_parameters &  arg0 );

     ~RdistanceTransform_parameters (  );

    const char * getTypeName (  ) const;

    RdistanceTransform_parameters & copy ( const RdistanceTransform_parameters &  arg0 );

    RdistanceTransform_parameters & operator= ( const RdistanceTransform_parameters &  arg0 );

    virtual functor::RdistanceTransform_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    eDistanceType distance  ;
};
}
typedef lti::_distanceTransform::RdistanceTransform_parameters distanceTransform_parameters;
}
#endif

