//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

//%include utils.i
#ifndef OLD_COMPILE
%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i
#endif


IMPORT_SIMPLE_HEADER_FILE("ltiSplitImage.h")
IMPORT_SIMPLE_HEADER_FILE("ltiMergeImage.h")

// Color Spaces
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++ now stats the wrapping of other ltilib header files +++
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToHSI.h")
%extend lti::splitImageToHSI {
    // transfer the HSI value as a rgbPixel (TODO: maybe better as a Python tuple? How?)
    lti::rgbPixel apply( const rgbPixel &pixel )
    {
        lti::ubyte H, S, I;
        self->apply( pixel, H, S, I );
        return lti::rgbPixel( H, S, I );
    }
/* TODO --> does not work !
    int[3] apply( const rgbPixel &pixel )
    {
        int ret[3];
        ubyte H, S, I;
        self->apply( pixel, H, S, I );
        ret[0] = H;
        ret[1] = S;
        ret[2] = I;
        return ret;
    }
*/    
}
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToRGB.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToHSV.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToHLS.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageTorgI.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToXYZ.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToxyY.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToOCP.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToYIQ.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToCIELuv.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageToYUV.h")


HANDLE_SIMPLE_HEADER_FILE("ltiMergeRGBToImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergeHSVToImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergeHLSToImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergergIToImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergeXYZToImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergexyYToImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergeOCPToImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergeYIQToImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergeCIELuvToImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergeYUVToImage.h")


