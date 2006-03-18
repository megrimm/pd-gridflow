#ifndef _RboundingBox_parameters_h
#define _RboundingBox_parameters_h

namespace lti {
namespace _boundingBox {
class RboundingBox_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
     RboundingBox_parameters (  );

     RboundingBox_parameters ( const RboundingBox_parameters &  arg0 );

     ~RboundingBox_parameters (  );

    const char * getTypeName (  ) const;

    RboundingBox_parameters & copy ( const RboundingBox_parameters &  arg0 );

    virtual functor::RboundingBox_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool &  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool &  arg1 );

    int borderSize  ;
    bool centerOfGravity  ;
    T backgroundColor  ;
    bool justSuppressBackground  ;
    bool optimalBox  ;
    bool useLengths  ;
};
}
typedef lti::_boundingBox::RboundingBox_parameters boundingBox_parameters;
}
#endif

