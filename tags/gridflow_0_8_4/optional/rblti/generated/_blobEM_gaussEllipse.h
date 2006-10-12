#ifndef _RblobEM_gaussEllipse_h
#define _RblobEM_gaussEllipse_h

namespace lti {
namespace _blobEM {
class RblobEM_gaussEllipse
{
public:
    lti::tpoint< double > center  ;
    double lambda1  ;
    double lambda2  ;
    double angle  ;
    bool constrainCenter  ;
    bool constrainShape  ;
    bool constrainArea  ;
    bool constrainAngle  ;
    double centerTolerance  ;
    double shapeTolerance  ;
    double areaTolerance  ;
    double angleTolerance  ;
     RblobEM_gaussEllipse (  );

     RblobEM_gaussEllipse ( lti::tpoint< double >  arg0, double  arg1, double  arg2, double  arg3 );

     ~RblobEM_gaussEllipse (  );

    bool from2x2Covariance ( const lti::matrix< double > &  arg0, const bool &  arg1 );

    bool fromEllipse ( const RblobEM_gaussEllipse &  arg0, const bool &  arg1 );

    bool to2x2Covariance ( lti::matrix< double > &  arg0 ) const;

    
};
}
typedef lti::_blobEM::RblobEM_gaussEllipse blobEM_gaussEllipse;
}
#endif

