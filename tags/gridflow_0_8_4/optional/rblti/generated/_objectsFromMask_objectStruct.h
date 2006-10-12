#ifndef _RobjectsFromMask_objectStruct_h
#define _RobjectsFromMask_objectStruct_h

namespace lti {
namespace _objectsFromMask {
class RobjectsFromMask_objectStruct : public lti::ioObject
{
public:
    virtual bool write ( ioHandler &  arg0, const bool  arg1 ) const;

    virtual bool read ( ioHandler &  arg0, const bool  arg1 );

    ioPoints ioPointList  ;
    areaPoints areaPointList  ;
    borderPoints borderPointList  ;
};
}
typedef lti::_objectsFromMask::RobjectsFromMask_objectStruct objectsFromMask_objectStruct;
}
#endif

