#ifndef _RorientationMap_parameters_h
#define _RorientationMap_parameters_h

namespace lti {
namespace _orientationMap {
class RorientationMap_parameters : public lti::_transform::Rtransform_parameters
{
public:
    enum eMode {
Ogd 
,Gradient 
};

     RorientationMap_parameters (  );

     RorientationMap_parameters ( const RorientationMap_parameters &  arg0 );

     ~RorientationMap_parameters (  );

    const char * getTypeName (  ) const;

    RorientationMap_parameters & copy ( const RorientationMap_parameters &  arg0 );

    RorientationMap_parameters & operator= ( const RorientationMap_parameters &  arg0 );

    virtual functor::RorientationMap_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    eMode mode  ;
    int size  ;
    double variance  ;
    int localFilterSize  ;
    double localFilterVariance  ;
};
}
typedef lti::_orientationMap::RorientationMap_parameters orientationMap_parameters;
}
#endif

