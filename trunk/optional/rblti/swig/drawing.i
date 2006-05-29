 HANDLE_SIMPLE_HEADER_FILE("ltiDrawBase.h")
 namespace lti {
 %template(imageDrawBase)       drawBase<rgbPixel>;
 %template(channelDrawBase)     drawBase<float>;
 %template(channel8DrawBase)     drawBase<ubyte>;
 }
 
 HANDLE_SIMPLE_HEADER_FILE("ltiDraw.h")

 namespace lti {
 %template(imageDraw)           draw<rgbPixel>;
 %template(channelDraw)         draw<float>;
 %template(channel8Draw)         draw<ubyte>;
 }