#ifndef _LTI_BLOB_EM_TRACKER_H_
#define _LTI_BLOB_EM_TRACKER_H_

#include "ltiBlobEM.h"
#include "ltiObjectsFromMask.h"
#include "ltiDistanceTransform.h"
#include "ltiMatrix.h"
#include "ltiImage.h"
#include "ltiPointList_extra.h"
#include "ltiContour.h"
#include "ltiConvexHull.h"

namespace lti{

    class blobEMTracker: public functor {
        public:
        class parameters : public functor::parameters {
            public:
            
            parameters();
            virtual ~parameters();
            parameters(const parameters&);
            virtual functor::parameters* clone() const;
            //Blobs must be larger than this size to be recognized
            int minSize;
        };
        
        blobEMTracker();
        blobEMTracker(const blobEMTracker&);
        virtual ~blobEMTracker();
        virtual functor* clone() const;
        virtual bool updateParameters();
        const parameters& getParameters() const;
        void apply(const channel8 & , channel8 & , matrix<int> &);
        void drawBlobInChannel(const tpointList<int> &, channel8 &);
                
        private:
        blobEM tracker;
        convexHull cHull;
        unsigned int blobCount;
        distanceTransform dt;
        distanceTransform::parameters dtPar;
        objectsFromMask ofm;
        objectsFromMask::parameters ofmPar;
        std::list< areaPoints > blobList;
        std::vector<blobEM::gaussEllipse> gaussVec;
    };
}

#endif
