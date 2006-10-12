#ifndef _RfastLineExtraction_segmEntry_h
#define _RfastLineExtraction_segmEntry_h

namespace lti {
namespace _fastLineExtraction {
class RfastLineExtraction_segmEntry
{
public:
    ipoint start  ;
    ipoint end  ;
    int len  ;
    int used  ;
};
}
typedef lti::_fastLineExtraction::RfastLineExtraction_segmEntry fastLineExtraction_segmEntry;
}
#endif

