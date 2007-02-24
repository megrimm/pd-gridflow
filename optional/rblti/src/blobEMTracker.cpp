/*
ToDo: - Try 4-neighbourhood distance for distanceTransform
*/


#include "blobEMTracker.h"

namespace lti {

    blobEMTracker::blobEMTracker() : functor(), tracker(), ofm(), ofmPar(), dt(), dtPar(), cHull()
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
        cHull.copy(other.cHull);
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
        //printf("Changing internal objectsFromMask parameter minSize attribute to %d\n", par.minSize);
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
    
    void blobEMTracker::apply(const channel8& inChan, channel8& outChan, matrix<int>& outMat)
    {
        unsigned int newBlobCount;
        bool found;
        unsigned int i, vLen = gaussVec.size();
        std::list<areaPoints>::const_iterator blobIt;
        areaPoints tempBlob;
        ipoint center1, center2;
        std::vector<blobEM::gaussEllipse>::iterator vecIt;
        polygonPoints polyContour;
        
        //Preprocessing
        std::list<ioPoints>::iterator tmpBlobIt;
        std::list<ioPoints> tmpBlobList;
        channel8 tmpChan;
        
        ofm.apply(inChan, tmpBlobList);
        
        tmpChan.resize((inChan.size()).y, (inChan.size()).x, 0, false, true);
        //Make sure all blobs are convex
        for (tmpBlobIt = tmpBlobList.begin(); tmpBlobIt != tmpBlobList.end() ; tmpBlobIt++)
        {
            cHull.apply((*tmpBlobIt), polyContour); //get convexhull of contour
            tempBlob.castFrom(polyContour);
            drawBlobInChannel(tempBlob, tmpChan);
        }
        
        //resize the output matrix according to the number of blobs
        ofm.apply(tmpChan, blobList);
        newBlobCount = blobList.size();  // !!!!!!!!!!!!!!!!!!!!!!!!!***************
        
        if (newBlobCount > 0)
            outMat.resize(newBlobCount, 4, 0, false, true);
        else
            outMat.resize(1, 4, 0, false, true);   //output matrix must have at least one row
        
        //Check if any new blobs appeared in this iteration
        if (newBlobCount > blobCount)
        {
            //Identify the blobs we already had and the new one
            //Two blobs are assumed the same if the center of gravity (cog) of one
            //of them is contained inside the other 
            for (blobIt = blobList.begin(); (blobIt != blobList.end() && (newBlobCount != gaussVec.size())); blobIt++)
            {
                //Compare each ellipse to the ones from objectsFromMask
                tempBlob = (*blobIt);
                i=0;
                found = false;
                while (!found && i< vLen)
                {
                    found = lti::contains(tempBlob, center2.castFrom(gaussVec[i].center));
                    if (found)
                        outMat.at(i,3) = (*blobIt).size();
                    i++;
                }
                
                if(!found)   //This is a new blob
                {
                    center1 = lti::cog(tempBlob);
                    //Create new gaussEllipse to track the new blob and insert in vector
                    blobEM::gaussEllipse newEll(lti::tpoint<double>((double)center1.x,(double)center1.y), 11., 10., 0.0);
                    gaussVec.push_back(newEll);
                    outMat.at(gaussVec.size()-1,3) = (*blobIt).size();
                    printf("found new blob at (%d, %d), size is %d, position %d in vector\n", center1.x, center1.y, (*blobIt).size(), (int)gaussVec.size()-1);
                }
            }
            //assert(outMat.size().y == newBlobCount);
        }
        //Check if some blobs disappeared
        else if (newBlobCount < blobCount)
        {


            //Identify the blob that disappeared
            //Two blobs are assumed the same if the center of gravity (cog) of one
            //of them is contained inside the other
            vecIt = gaussVec.begin();
            i=0;
            
            //First, erase any ellipse whose center is not inside a blob
            while ((vecIt != gaussVec.end()) && (0 != gaussVec.size()) && (newBlobCount != gaussVec.size()))
            {
                printf("Currently %d blobs\n", gaussVec.size());
                found = false;
                center1.castFrom((*vecIt).center);

                //Check if the center of the ellipse is inside a blob
                //If not, a blob disappeared, erase it
                if(255 != tmpChan.at(center1))
                {
                    gaussVec.erase(vecIt); 
                    vecIt = gaussVec.begin();
                    i = 0;
                    printf("Blob disappeared, %d left\n", gaussVec.size());
                    if(vecIt == gaussVec.end())
                        printf("Iterator at the end of vector\n");
                }
                else
                { 
                    i++;
                    vecIt++;
                }
            }
            
            
	   //***************************************************************************
            
            //Now, go trough each blob and check if any two ellipse correspond to the same blob
            //this allows detection of merged blobs
            bool cnt;
            for (blobIt = blobList.begin(); (blobIt != blobList.end() && (newBlobCount != gaussVec.size())); blobIt++)
            {
                //Compare each ellipse to the ones from objectsFromMask
                tempBlob = (*blobIt);
                found = false;
                vLen = gaussVec.size();
                
                vecIt = gaussVec.begin();
                while ((vecIt != gaussVec.end()) && (newBlobCount != gaussVec.size()))
                {
                    cnt = lti::contains(tempBlob, center2.castFrom(vecIt->center));
                    if ((!found) && cnt) //found the first ellipse corresponding to this blob
                        found = true;
                    else if (found && cnt)  //found another ellipse corresponding to this blob
                    {
                        //Erase the ellipse
                        gaussVec.erase(vecIt); 
                        vecIt = gaussVec.begin();
                        printf("Blob merged with another, %d left\n", gaussVec.size());
                        if(vecIt == gaussVec.end())
                            printf("Iterator at the end of vector\n");
                    }
                    vecIt++;
                }
            }

	   //***************************************************************************

        }
        
        //assert(newBlobCount == gaussVec.size());
        
        blobCount = newBlobCount;
        //track blobs as usual using blobEM
        channel8 dtChan(inChan.size(),(ubyte)0);
        
        outChan.resize((inChan.size()).y, (inChan.size()).x, 0, false, true);
        std::list<areaPoints>::iterator objiter = blobList.begin();
   
        areaPoints blob;
        areaPoints::iterator ptiter;

        //Draw the blobs in the output channel
        for(; objiter != blobList.end(); objiter++)
        {
            blob = *(objiter);
            ptiter = blob.begin();
            for(; ptiter != blob.end(); ptiter++)
                    outChan.at((*ptiter).y, (*ptiter).x) = 255;
        }

        dt.apply(outChan, dtChan);
        tracker.apply(dtChan, gaussVec);
        
        if (gaussVec.size() != blobList.size())
            printf("wth?? bloblist.size = %d, gaussVec.size = %d\n", blobList.size(), gaussVec.size());
        
        //assert(gaussVec.size() == blobList.size());
        //Put the centers of the blobs into outMat
        for (vecIt = gaussVec.begin(), i=0; vecIt != gaussVec.end(); vecIt++, i++)
        {
            outMat.at(i,1) = (int) ((*vecIt).center.y);
            outMat.at(i,2) = (int) ((*vecIt).center.x);
            
            for (blobIt = blobList.begin(); blobIt != blobList.end(); blobIt++)
            {
                if (contains((*blobIt), center2.castFrom((*vecIt).center)))
                    outMat.at(i,3) = (*blobIt).size();
            }
            outMat.at(i,0) = (long) &(*vecIt);
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
    
    void blobEMTracker::drawBlobInChannel(const tpointList<int>& blob, channel8& chan)
    {
    	lti::tpointList<int>::const_iterator it;
    	
    	for (it = blob.begin(); it != blob.end(); it++)
    	    chan.at((*it).y, (*it).x) = 255;
    }
}
