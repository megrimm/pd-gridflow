//******************************************************************************
//
//      project:  pylti
//    copyright:  (c) 2005 by Michael Neuroth
//
//  description:  python modul for ltilib (using SWIG)
//                    ltilib extras functions and classes
//
//  $Source$
//  $Revision$
//  $Date$
// 
//  $Log$
//  Revision 1.1  2006/02/25 23:43:56  matju
//  0.33.1
//
//  Revision 1.1  2005/12/25 15:41:16  Michael
//  splitted lti.i into different swig interface files
//
//
//******************************************************************************

//%include utils.i

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
