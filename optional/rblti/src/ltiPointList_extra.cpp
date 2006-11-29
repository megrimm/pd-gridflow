#include "ltiPointList.h"

namespace lti{

bool contains(const tpointList<int>& pl, const tpoint<int>& cmp)
    {
        lti::tpointList<int>::const_iterator it;
        for (it = pl.begin(); it != pl.end(); it++)
            {
            if ((*it) == cmp)
                return true;
            }
        //not found, return false 
        return false;
    }

tpoint<int> cog(const tpointList<int>& pl)
    {
    tpoint<int> acc(0,0);
    lti::tpointList<int>::const_iterator it;
        for (it = pl.begin(); it != pl.end(); it++)
            {
            acc.x += (*it).x;
            acc.y += (*it).y;
            }

        //compute the cog
        if (pl.size() != 0)
        {
            acc.x /= pl.size();
            acc.y /= pl.size();
        }
        return acc;
    }
}
