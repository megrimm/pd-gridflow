/*
ToDo: - Try 4-neighbourhood distance for distanceTransform
*/


#include "blobEMTracker.h"

namespace lti {

    blobEMTracker::blobEMTracker() : functor(), tracker(), ofm(), ofmPar(), dt(), dtPar()
    {
        parameters defaultPar;
        setParameters(defaultPar);
        ofmPar.sortByArea = true;
        ofmPar.minSize = 100;
        ofmPar.meltHoles = true;
        ofmPar.level = 0;
        ofm.setParameters(ofmPar);
        dt.setParameters(dtPar);
        blobCount = 0;
    }
    
    blobEMTracker::blobEMTracker(const blobEMTracker& other) : functor()
    {
        functor::copy(other);
        tracker.copy(other.tracker);
        ofm.copy(other.ofm);
        ofmPar.copy(other.ofmPar);
        dt.copy(other.dt);
        dtPar.copy(other.dtPar);
        blobCount = other.blobCount;
        blobList = other.blobList;
        gaussVec = other.gaussVec;
    }
    
    blobEMTracker::~blobEMTracker()
    {
    }
    
    functor* blobEMTracker::clone() const 
    {
        return new blobEMTracker(*this);
    }
    
    bool blobEMTracker::updateParameters()
    {
        const parameters& par = getParameters();
        ofmPar.minSize = par.minSize;
        ofm.setParameters(ofmPar);
        printf("Changing internal objectsFromMask parameter minSize attribute to %d", par.minSize);
        return validParameters();
    }
    
    const blobEMTracker::parameters& blobEMTracker::getParameters() const 
    {
        const parameters* par = dynamic_cast<const parameters*>(&functor::getParameters());
        if(isNull(par)) {
            throw invalidParametersException(getTypeName());
        }
        return *par;
    }
    
    void blobEMTracker::apply(const channel8& inChan, matrix<int>& outMat)
    {
        int newBlobCount;
        
        //Extract the blobs from the image
        ofm.apply(inChan, blobList);
        newBlobCount = blobList.size();
        
        //resize the output matrix according to the number of blobs
        outMat.resize(newBlobCount, 3, 0, false, true);
        
        bool found;
        int i, vLen = gaussVec.size();
        std::list<areaPoints>::const_iterator blobIt;
        areaPoints tempBlob;
        ipoint center1, center2;
        std::vector<blobEM::gaussEllipse>::iterator vecIt;
        //Check if any new blobs appeared in this iteration
        if (newBlobCount > blobCount)
        {
            //Identify the blobs we already had and the new one
            //Two blobs are assumed the same if the center of gravity (cog) of one
            //of them is contained inside the other 
            for (blobIt = blobList.begin(); blobIt != blobList.end(); blobIt++)
            {
                //Compare each ellipse to the ones from objectsFromMask
                tempBlob = (*blobIt);
                i=0;
                found = false;
                while (!found && i< vLen)
                    found = lti::contains(tempBlob, center2.castFrom(gaussVec[i++].center));
                
                if(!found)   //This is a new blob
                {
                    center1 = lti::cog(tempBlob);
                    //Create new gaussEllipse to track the new blob and insert in vector
                    blobEM::gaussEllipse newEll(lti::tpoint<double>((double)center1.x,(double)center1.y), 11., 10., 0.0);
                    gaussVec.push_back(newEll);
                }
            }
        }
        //Check if some blobs disappeared
        else if (newBlobCount < blobCount)
        {
            blobEM::gaussEllipse tempEll;
            //Identify the blob that disappeared
            //Two blobs are assumed the same if the center of gravity (cog) of one
            //of them is contained inside the other
            for (vecIt = gaussVec.begin(); vecIt != gaussVec.end(); vecIt++)
            {
                tempEll = (*vecIt);
                found = false;
                center1.castFrom(tempEll.center);
                
                for(blobIt = blobList.begin(); (blobIt != blobList.end()) && (!found); blobIt++)
                    found = lti::contains((*blobIt), center1);
                
                if(!found) //a blob disappeared, erase it
                    gaussVec.erase(vecIt);
            }
        }
        
        blobCount = newBlobCount;
        //track blobs as usual using blobEM
        channel8 dtChan(inChan.size(),(ubyte)0);
        dt.apply(inChan, dtChan);
        tracker.apply(dtChan, gaussVec);
        
        //Put the centers of the blobs into outMat
        for (vecIt = gaussVec.begin(), i=0; vecIt != gaussVec.end(); vecIt++, i++)
        {
            outMat.at(i,1) = (int) ((*vecIt).center.y);
            outMat.at(i,2) = (int) ((*vecIt).center.x);
        }
    }
    
    //***************************************************************************************************
    //***************************************************************************************************
    
    
    blobEMTracker::parameters::parameters() : functor::parameters()
    {
        minSize=100;
    }
    
    blobEMTracker::parameters::~parameters()
    {
    }
        
    blobEMTracker::parameters::parameters(const parameters& other) : functor::parameters()
    {
        functor::parameters::copy(other);
        minSize = other.minSize;
    }
    
    functor::parameters* blobEMTracker::parameters::clone() const
    {
        return new parameters(*this);
    }
    

}
