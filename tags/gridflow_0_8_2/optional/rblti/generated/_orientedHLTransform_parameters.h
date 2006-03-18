#ifndef _RorientedHLTransform_parameters_h
#define _RorientedHLTransform_parameters_h

namespace lti {
namespace _orientedHLTransform {
class RorientedHLTransform_parameters : public lti::_transform::Rtransform_parameters
{
public:
    enum eAccumulationMode {
Classic 
,Gradient 
};

     RorientedHLTransform_parameters (  );

     RorientedHLTransform_parameters ( const RorientedHLTransform_parameters &  arg0 );

     ~RorientedHLTransform_parameters (  );

    const char * getTypeName (  ) const;

    RorientedHLTransform_parameters & copy ( const RorientedHLTransform_parameters &  arg0 );

    RorientedHLTransform_parameters & operator= ( const RorientedHLTransform_parameters &  arg0 );

    virtual functor::RorientedHLTransform_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    trectangle< int > transformationArea  ;
    int baseValue  ;
    int accuracy  ;
    int range  ;
    eAccumulationMode accumulationMode  ;
};
}
typedef lti::_orientedHLTransform::RorientedHLTransform_parameters orientedHLTransform_parameters;
}
#endif

