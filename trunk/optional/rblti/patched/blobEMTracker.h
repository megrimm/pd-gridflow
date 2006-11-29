#ifndef _LTI_BLOB_EM_TRACKER_H_
#define _LTI_BLOB_EM_TRACKER_H_

#include "ltiBlobEM.h"
#include "ltiObjectsFromMask.h"
#include "ltiDistanceTransform.h"
#include "ltiMatrix.h"
#include "ltiImage.h"
#include "ltiPointList_extra.h"
#include "ltiContour.h"

namespace lti{

    class blobEMTracker{
        public:
        class parameters{
            public:
            
            parameters();
            //Blobs must be larger than this size to be recognized
            int minSize;
        };
        
        blobEMTracker();
        void setParameters(const parameters& param);
        void apply(const channel8 &, matrix<int> &);
        
        private:
        blobEM tracker;
        int blobCount;
        distanceTransform dt;
        distanceTransform::parameters dtPar;
        objectsFromMask ofm;
        objectsFromMask::parameters ofmPar;
        std::list< areaPoints > blobList;
        std::vector<blobEM::gaussEllipse> gaussVec;
        parameters params;
    };
}

#endif
