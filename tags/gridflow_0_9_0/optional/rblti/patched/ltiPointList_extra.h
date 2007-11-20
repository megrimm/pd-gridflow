#ifndef _LTI_POINTLIST_EXTRA_H
#define _LTI_POINTLIST_EXTRA_H
#include "ltiPointList.h"

namespace lti{

bool contains(const tpointList<int>&, const tpoint<int>&);
tpoint<int> cog(const tpointList<int>& );
}

#endif
