#ifndef _RdistanceTransform_sedMask_h
#define _RdistanceTransform_sedMask_h

namespace lti {
namespace _distanceTransform {
class RdistanceTransform_sedMask
{
public:
     RdistanceTransform_sedMask ( const point  arg0, int  arg1 );

     ~RdistanceTransform_sedMask (  );

    void filter ( matrix< point > &  arg0, const point &  arg1 ) const;

    
    
    
};
}
typedef lti::_distanceTransform::RdistanceTransform_sedMask distanceTransform_sedMask;
}
#endif

