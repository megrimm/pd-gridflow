#ifndef _RobjectsFromMask_parameters_h
#define _RobjectsFromMask_parameters_h

namespace lti {
namespace _objectsFromMask {
class RobjectsFromMask_parameters : public lti::_segmentation::Rsegmentation_parameters
{
public:
     RobjectsFromMask_parameters (  );

     RobjectsFromMask_parameters ( const RobjectsFromMask_parameters &  arg0 );

     ~RobjectsFromMask_parameters (  );

    const char * getTypeName (  ) const;

    RobjectsFromMask_parameters & copy ( const RobjectsFromMask_parameters &  arg0 );

    virtual functor::RobjectsFromMask_parameters * clone (  ) const;

    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    int threshold  ;
    bool assumeLabeledMask  ;
    int minSize  ;
    int level  ;
    bool meltHoles  ;
    bool sortObjects  ;
    bool sortByArea  ;
    lti::ioPoints ioSearchAreaList  ;
};
}
typedef lti::_objectsFromMask::RobjectsFromMask_parameters objectsFromMask_parameters;
}
#endif

