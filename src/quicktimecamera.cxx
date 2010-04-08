/*
	$Id$

	GridFlow
	Copyright (c) 2001-2010 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <QuickTime/QuickTime.h>
#include <QuickTime/Movies.h>
#include <QuickTime/QuickTimeComponents.h>
#include "gridflow.hxx.fcs"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <CoreServices/CoreServices.h>
#include <map>
extern std::map<long,const char *> oserr_table;

typedef ComponentInstance VideoDigitizerComponent, VDC;
typedef ComponentResult   VideoDigitizerError,     VDE;

#if 0
//enum {VDCType='vdig', vdigInterfaceRev=2 };
//enum {ntscIn=0, currentIn=0, palIn, secamIn, ntscReallyIn };
//enum {compositeIn, sVideoIn, rgbComponentIn, rgbComponentSyncIn, yuvComponentIn, yuvComponentSyncIn, tvTunerIn, sdiIn};
//enum {vdPlayThruOff, vdPlayThruOn};
//enum {vdDigitizerBW, vdDigitizerRGB};
//enum {vdBroadcastMode, vdVTRMode};
//enum {vdUseAnyField, vdUseOddField, vdUseEvenField};
//enum {vdTypeBasic, vdTypeAlpha, vdTypeMask, vdTypeKey};
/*enum {digiInDoesNTSC, digiInDoesPAL, digiInDoesSECAM, skip 4,
  digiInDoesGenLock, digiInDoesComposite, digiInDoesSVideo, digiInDoesComponent,
  digiInVTR_Broadcast, digiInDoesColor, digiInDoesBW, skip 17,
  digiInSignalLock};*/
/*bitset {digiOutDoes1, digiOutDoes2, digiOutDoes4,
         digiOutDoes8, digiOutDoes16, digiOutDoes32, 
  digiOutDoesDither, digiOutDoesStretch, digiOutDoesShrink,
  digiOutDoesMask, skip 1, 
  digiOutDoesDouble, digiOutDoesQuad, digiOutDoesQuarter, digiOutDoesSixteenth,
  digiOutDoesRotate, digiOutDoesHorizFlip, digiOutDoesVertFlip, digiOutDoesSkew,
  digiOutDoesBlend, digiOutDoesWarp, digiOutDoesHW_DMA,
  digiOutDoesHWPlayThru, digiOutDoesILUT, digiOutDoesKeyColor,
  digiOutDoesAsyncGrabs, digiOutDoesUnreadableScreenBits, 
  digiOutDoesCompress, digiOutDoesCompressOnly,
  digiOutDoesPlayThruDuringCompress, digiOutDoesCompressPartiallyVisible,
  digiOutDoesNotNeedCopyOfCompressData};*/
/*struct DigitizerInfo {
  short vdigType;
  long inputCapabilityFlags, outputCapabilityFlags;
  long inputCurrentFlags,    outputCurrentFlags;
  short slot;
  GDHandle gdh, maskgdh;
  short minDestHeight, minDestWidth;
  short maxDestHeight, maxDestWidth;
  short blendLevels;
  long reserved;};*/
/*struct VdigType { long digType, reserved;};*/
/*struct VdigTypeList { short count; VdigType list[1];};*/
/*struct VdigBufferRec { PixMapHandle dest; Point location; long reserved;};*/
/*struct VdigBufferRecList {
  short count; MatrixRecordPtr matrix; RgnHandle mask; VdigBufferRec list[1];};*/
//typedef VdigBufferRecList *VdigBufferRecListPtr;
//typedef VdigBufferRecListPtr *VdigBufferRecListHandle;
//typedef CALLBACK_API(void,VdigIntProcPtr)(long flags, long refcon);
//typedef STACK_UPP_TYPE(VdigIntProcPtr);
/*struct VDCompressionList {
  CodecComponent codec; CodecType cType; Str63 typeName, name;
  long formatFlags, compressFlags, reserved;};*/
//typedef VDCompressionList *   VDCompressionListPtr;
//typedef VDCompressionListPtr *VDCompressionListHandle;
/*bitset {
  dmaDepth1, dmaDepth2, dmaDepth4, dmaDepth8, dmaDepth16, dmaDepth32,
  dmaDepth2Gray, dmaDepth4Gray, dmaDepth8Gray};*/
//enum {kVDIGControlledFrameRate=-1};
//bitset {vdDeviceFlagShowInputsAsDevices, vdDeviceFlagHideDevice};
/*bitset {
  vdFlagCaptureStarting, vdFlagCaptureStopping,
  vdFlagCaptureIsForPreview, vdFlagCaptureIsForRecord,
  vdFlagCaptureLowLatency, vdFlagCaptureAlwaysUseTimeBase,
  vdFlagCaptureSetSettingsBegin, vdFlagCaptureSetSettingsEnd};*/
/*\class VDC
VDE VDGetMaxSrcRect   (short inputStd, Rect *maxSrcRect)
VDE VDGetActiveSrcRect(short inputStd, Rect *activeSrcRect)
VDE VD[GS]etDigitizerRect(Rect *digitizerRect)
VDE VDGetVBlankRect(short inputStd, Rect *vBlankRect)
VDE VDGetMaskPixMap(PixMapHandlemaskPixMap)
VDE VDGetPlayThruDestination(PixMapHandle *    dest, Rect *destRect, MatrixRecord *    m, RgnHandle *mask)
VDE VDUseThisCLUT(CTabHandle colorTableHandle)
VDE VD[SG*]etInputGammaValue(Fixed channel1, Fixed channel2, Fixed channel3)
VDE VD[GS]etBrightness(uint16 *)
VDE VD[GS]etContrast(uint16 *)
VDE VD[GS]etHue(uint16 *)
VDE VD[GS]etSharpness(uint16 *)
VDE VD[GS]etSaturation(uint16 *)
VDE VDGrabOneFrame(VDC ci)
VDE VDGetMaxAuxBuffer(PixMapHandle *pm, Rect *r)
VDE VDGetDigitizerInfo(DigitizerInfo *info)
VDE VDGetCurrentFlags(long *inputCurrentFlag, long *outputCurrentFlag)
VDE VD[SG*]etKeyColor(long index)
VDE VDAddKeyColor(long *index)
VDE VDGetNextKeyColor(long  index)
VDE VD[GS]etKeyColorRange(RGBColor minRGB, RGBColor maxRGB)
VDE VDSetDigitizerUserInterrupt(long  flags, VdigIntUPP userInterruptProc, long  refcon)
VDE VD[SG*]etInputColorSpaceMode(short colorSpaceMode)
VDE VD[SG*]etClipState(short clipEnable)
VDE VDSetClipRgn(RgnHandle clipRegion)
VDE VDClearClipRgn(RgnHandle clipRegion)
VDE VDGetCLUTInUse(CTabHandle *colorTableHandle)
VDE VD[SG*]etPLLFilterType(short pllType)
VDE VDGetMaskandValue(uint16 blendLevel, long *mask, long *value)
VDE VDSetMasterBlendLevel(uint16 *blendLevel)
VDE VDSetPlayThruDestination(PixMapHandledest, RectPtr destRect, MatrixRecordPtr m, RgnHandle mask)
VDE VDSetPlayThruOnOff(short state)
VDE VD[SG*]etFieldPreference(short fieldFlag)
VDE VDPreflightDestination(Rect *digitizerRect, PixMap **dest, RectPtr destRect, MatrixRecordPtr m)
VDE VDPreflightGlobalRect(GrafPtr theWindow, Rect *globalRect)
VDE VDSetPlayThruGlobalRect(GrafPtr theWindow, Rect *globalRect)
VDE VDSetInputGammaRecord(VDGamRecPtrinputGammaPtr)
VDE VDGetInputGammaRecord(VDGamRecPtr *inputGammaPtr)
VDE VD[SG]etBlackLevelValue(uint16 *)
VDE VD[SG]etWhiteLevelValue(uint16 *)
VDE VDGetVideoDefaults(uint16 *blackLevel, uint16 *whiteLevel, uint16 *brightness, uint16 *hue, uint16 *saturation, uint16 *contrast, uint16 *sharpness)
VDE VDGetNumberOfInputs(short *inputs)
VDE VDGetInputFormat(short input, short *format)
VDE VD[SG*]etInput(short input)
VDE VDSetInputStandard(short inputStandard)
VDE VDSetupBuffers(VdigBufferRecListHandle bufferList)
VDE VDGrabOneFrameAsync(short buffer)
VDE VDDone(short buffer)
VDE VDSetCompression(OSTypecompressType, short depth, Rect *bounds, CodecQspatialQuality, CodecQtemporalQuality, long  keyFrameRate)
VDE VDCompressOneFrameAsync(VDC ci)
VDE VDCompressDone(UInt8 *queuedFrameCount, Ptr *theData, long *dataSize, UInt8 *similarity, TimeRecord *t)
VDE VDReleaseCompressBuffer(Ptr bufferAddr)
VDE VDGetImageDescription(ImageDescriptionHandle desc)
VDE VDResetCompressSequence(VDC ci)
VDE VDSetCompressionOnOff(Boolean)
VDE VDGetCompressionTypes(VDCompressionListHandle h)
VDE VDSetTimeBase(TimeBase t)
VDE VDSetFrameRate(Fixed framesPerSecond)
VDE VDGetDataRate(long *milliSecPerFrame, Fixed *framesPerSecond, long *bytesPerSecond)
VDE VDGetSoundInputDriver(Str255 soundDriverName)
VDE VDGetDMADepths(long *depthArray, long *preferredDepth)
VDE VDGetPreferredTimeScale(TimeScale *preferred)
VDE VDReleaseAsyncBuffers(VDC ci)
VDE VDSetDataRate(long  bytesPerSecond)
VDE VDGetTimeCode(TimeRecord *atTime, void *timeCodeFormat, void *timeCodeTime)
VDE VDUseSafeBuffers(Boolean useSafeBuffers)
VDE VDGetSoundInputSource(long videoInput, long *soundInput)
VDE VDGetCompressionTime(OSTypecompressionType, short depth, Rect *srcRect, CodecQ *spatialQuality, CodecQ *temporalQuality, ulong *compressTime)
VDE VDSetPreferredPacketSize(long preferredPacketSizeInBytes)
VDE VD[SG*]etPreferredImageDimensions(long  width, long  height)
VDE VDGetInputName(long videoInput, Str255 name)
VDE VDSetDestinationPort(CGrafPtr destPort)
VDE VDGetDeviceNameAndFlags(Str255 outName, UInt32 *outNameFlags)
VDE VDCaptureStateChanging(UInt32inStateFlags)
VDE VDGetUniqueIDs(UInt64 *outDeviceID, UInt64 *outInputID)
VDE VDSelectUniqueIDs(const UInt64 *inDeviceID, const UInt64 *inInputID)
*/
#endif

static OSErr callback(ComponentInstanceRecord*, char*, long int, long int*, long int, TimeValue, short int, long int) {
	post("FormatQuickTimeCamera callback");
	return noErr;
}

\class FormatQuickTimeCamera : Format {
  P<Dim> dim;
  uint8 *buf;
  uint8 *buf2;
  VDC vdc;
  int m_newFrame; 
  SeqGrabComponent m_sg;
  SGChannel m_vc;
  SGDeviceList deviceList;
  int nDevices;
  short m_pixelDepth;
  Rect rect;
  GWorldPtr m_srcGWorld;
  PixMapHandle m_pixMap;
  Ptr m_baseAddr;
  long m_rowBytes;
  int m_quality;
  P<BitPacking> bit_packing3;
  \constructor (t_symbol *mode, int device) {
	dim = new Dim(240,320,4);
	_0_colorspace(0,0,gensym("rgba"));
	OSErr e;
	rect.top=rect.left=0;
	rect.bottom=dim->v[0]; rect.right=dim->v[1];
	int n=0, i, j;
	Component c = 0;
	ComponentDescription cd;
	cd.componentType = SeqGrabComponentType;
	cd.componentSubType = 0;
	cd.componentManufacturer = 0;
	cd.componentFlags = 0;
	cd.componentFlagsMask = 0;
	for(;;) {
		c = FindNextComponent(c, &cd);
		if (!c) break;
		ComponentDescription cd2;
		Ptr name=0,info=0,icon=0;
		GetComponentInfo(c,&cd2,&name,&info,&icon);
		//post("Component #%d",n);
		char *t = (char *)&cd.componentType;
		//post(" type='%c%c%c%c'",t[3],t[2],t[1],t[0]);
		t = (char *)&cd.componentSubType;
		//post("  subtype='%c%c%c%c'",t[3],t[2],t[1],t[0]);
		//post("  name=%08x, *name='%*s'",name, *name, name+1);
		//post("  info=%08x, *info='%*s'",info, *name, info+1);
		n++;
	}
	//post("  number of components: %d",n);
	m_sg = OpenDefaultComponent(SeqGrabComponentType, 0);
	if(!m_sg) RAISE("could not open default component");
	e=SGInitialize(m_sg);
	if(e!=noErr) RAISE("could not initialize SG");
	e=SGSetDataRef(m_sg, 0, 0, seqGrabDontMakeMovie);
	if (e!=noErr) RAISE("dataref failed");
	e=SGNewChannel(m_sg, VideoMediaType, &m_vc);
	if(e!=noErr) post("could not make new SG channel");
	e=SGSetChannelBounds(m_vc, &rect);
	if(e!=noErr) post("could not set SG ChannelBounds");
	
	e=SGGetChannelDeviceList(m_vc, sgDeviceListIncludeInputs, &deviceList);
	if (e!=noErr) post("could not get device list");
	else {
		nDevices = (*deviceList)->count;
		//post("  number of available devices: %d", nDevices); 
		//post("  current device: %d", (*deviceList)->selectedIndex);
		//for (int i=0; i<nDevices; i++) post("  Device %d: %s", i, (*deviceList)->entry[i].name);
	}
    
	// treat the device list in reverse order
	device = nDevices-1-device;
	e=SGSetChannelDevice(m_vc, (*deviceList)->entry[device].name);
	if(e!=noErr) RAISE("could not set channel device");
	else {
		char *s1, s2[MAXPDSTRING];
		s1 = (char *)(*deviceList)->entry[device].name;
		for (i=1, j=0; i<=*s1; i++) {
			if (isalnum(s1[i])) s2[j++] = s1[i];
			else if (s1[i] == ' ') s2[j++] = '_';
		}
		s2[j] = '\0';
		name = gensym(s2);
	}

	e=SGSetChannelUsage(m_vc, seqGrabPreview);
	if(e!=noErr) post("could not set SG ChannelUsage");
	e=SGSetDataProc(m_sg,NewSGDataUPP(callback),0);
	if (e!=noErr) post("could not set SG DataProc");
	switch (3) {
	  case 0: e=SGSetChannelPlayFlags(m_vc, channelPlayNormal); break;
	  case 1: e=SGSetChannelPlayFlags(m_vc, channelPlayHighQuality); break;
	  case 2: e=SGSetChannelPlayFlags(m_vc, channelPlayFast); break;
	  case 3: e=SGSetChannelPlayFlags(m_vc, channelPlayAllData); break;
	}

	vdc = SGGetVideoDigitizerComponent(m_vc);

	int dataSize = dim->prod();             
	buf = new uint8[dataSize];
	buf2 = new uint8[dataSize];
	m_rowBytes = dim->prod(1);
	e=QTNewGWorldFromPtr (&m_srcGWorld,k32ARGBPixelFormat,&rect,NULL,NULL,0,buf,m_rowBytes);
	if (0/*yuv*/) {
		int dataSize = dim->prod()*2/4;
		buf = new uint8[dataSize];
		m_rowBytes = dim->prod(1)*2/4;
		e=QTNewGWorldFromPtr (&m_srcGWorld,k422YpCbCr8CodecType,&rect,NULL,NULL,0,buf,m_rowBytes);
	}
	if (e!=noErr) RAISE("error #%d at QTNewGWorldFromPtr",e);
	if (!m_srcGWorld) RAISE("Could not allocate off screen");
	SGSetGWorld(m_sg,(CGrafPtr)m_srcGWorld, NULL);
	e=SGStartRecord(m_sg);
	if (e!=noErr) RAISE("error #%d at SGStartRecord",e);
  }
  ~FormatQuickTimeCamera() {
    if (m_vc) if (::SGDisposeChannel(m_sg, m_vc)) RAISE("SGDisposeChannel");
    if (m_sg) {
      if (::CloseComponent(m_sg)) RAISE("CloseComponent");
      if (m_srcGWorld) ::DisposeGWorld(m_srcGWorld);
    }
  }
  \decl 0 bang ();
  \grin 0 int
  
  \attr t_symbol *name;
  \attr uint16 brightness();
  \attr uint16 contrast();
  \attr uint16 hue();
  \attr uint16 colour();
  \attr t_symbol *colorspace;
};

\def 0 colorspace (t_symbol *colorspace) { /* y yuv rgb rgba magic */
	string c = colorspace->s_name;
	if (c=="y"    ) {} else
	//if (c=="yuv"  ) {} else
	if (c=="rgb"  ) {} else
	if (c=="rgba" ) {} else
	//if (c=="magic") {} else
	   RAISE("got '%s' but supported colorspaces are: rgb rgba",c.data());
	uint32 masks[4]={0x0000ff00,0x00ff0000,0xff000000,0x00000000};
	bit_packing3 = new BitPacking(is_le(),4,3,masks);
	//bit_packing4 = new BitPacking(is_le(),bytes,4,masks);
	this->colorspace=gensym(c.data());
	dim = new Dim(dim->v[0],dim->v[1],c=="y"?1:c=="rgba"?4:3);
}

static int nn(int c) {return c?c:' ';}

\def 0 bang () {
	GridOutlet out(this,0,dim);
	string cs = colorspace->s_name;
	int sy = dim->v[0];
	int sx = dim->v[1];
	uint8 rgb[sx*4+4]; // with extra padding in case of odd size...
	uint8 b2[ sx*3+3];
	if (cs=="y") {
		for(int y=0; y<sy; y++) {
		        bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
			for (int x=0,xx=0; x<sx; x+=2,xx+=6) {
				b2[x+0] = (76*rgb[xx+0]+150*rgb[xx+1]+29*rgb[xx+2])>>8;
				b2[x+1] = (76*rgb[xx+3]+150*rgb[xx+4]+29*rgb[xx+5])>>8;
			}
			out.send(sx,b2);
		}
	} else if (cs=="rgb") {
		int n = dim->prod()/3;
		/*for (int i=0,j=0; i<n; i+=4,j+=3) {
			buf2[j+0] = buf[i+0];
			buf2[j+1] = buf[i+1];
			buf2[j+2] = buf[i+2];
		}*/
		//bit_packing3->unpack(sx,buf+y*sx*bit_packing3->bytes,rgb);
		bit_packing3->unpack(n,buf,buf2);
		out.send(dim->prod(),buf2);
	} else if (cs=="rgba") { // does this really work on PPC ?
		int n = dim->prod()/4;
		for (int i=0; i<n; i++) ((uint32 *)buf2)[i] = (((uint32 *)buf)[i] >> 8) | 0xff000000;
		out.send(dim->prod(),buf2);
	} else
		RAISE("colorspace problem");
	SGIdle(m_sg);
}

unsigned short val;
\def uint16 brightness ()             { VDGetBrightness(vdc,&val); return val; }
\def 0 brightness (uint16 brightness) { VDSetBrightness(vdc,&brightness); }
\def uint16 contrast ()               { VDGetContrast(vdc,&val); return val; }
\def 0 contrast (uint16 contrast)     { VDSetContrast(vdc,&contrast);}
\def uint16 hue ()                    { VDGetHue(vdc,&val); return val; }
\def 0 hue (uint16 hue)               { VDSetHue(vdc,&hue);}
\def uint16 colour ()                 { VDGetSaturation(vdc,&val); return val; }
\def 0 colour (uint16 colour)         { VDSetSaturation(vdc,&colour);}

GRID_INLET(0) {RAISE("Unimplemented. Sorry.");} GRID_END
\end class FormatQuickTimeCamera {install_format("#io.quicktimecamera",4,"");}

void startup_quicktimecamera () {
	\startall
}
