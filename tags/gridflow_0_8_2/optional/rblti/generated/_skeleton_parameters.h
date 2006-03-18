#ifndef _Rskeleton_parameters_h
#define _Rskeleton_parameters_h

namespace lti {
namespace _skeleton {
class Rskeleton_parameters : public lti::_transform::Rtransform_parameters
{
public:
    enum eKernelType {
CityBlock 
,ChessBoard 
,Euclidean 
};

     Rskeleton_parameters (  );

     Rskeleton_parameters ( const Rskeleton_parameters &  arg0 );

     ~Rskeleton_parameters (  );

    const char * getTypeName (  ) const;

    Rskeleton_parameters & copy ( const Rskeleton_parameters &  arg0 );

    virtual functor::Rskeleton_parameters * clone (  ) const;

    eKernelType kernelType  ;
    int formPointsValue  ;
    int jPointsValue  ;
};
}
typedef lti::_skeleton::Rskeleton_parameters skeleton_parameters;
}
#endif

