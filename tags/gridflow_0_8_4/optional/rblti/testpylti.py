# very simple test script for pylti

import pylti as lti
import sys

def PrintUsage():    
    print "usage: python testpylti.py <image-name>"

def ShowImage(img):    
    view = lti.externViewer()    
    view.show(img)

def LoadImageJPEG(sName):    
    img = lti.image()    
    param = lti.RioJPEG_parameters()        
    lti._setStdString(param.filename,sName)       
    #param.filename = sName
    lo = lti.loadJPEG()    
    lo.setParameters(param)    
    ok = lo.apply(img)    
    if not ok:        
        img = None    
    return img

def SegmentImage(img,noOfColors=16):    
    functor = lti.kMeansSegmentation()    
    params = lti.RkMeansSegmentation_parameters()    
    paramsColor = lti.RkMColorQuantization_parameters()    
    paramsColor.numberOfColors = noOfColors    
    params.setQuantParameters(paramsColor)    
    functor.setParameters(params)    
    matrixout = lti.imatrix()    
    paletteout = lti.palette()    
    functor.apply(img,matrixout,paletteout)    
    return matrixout,paletteout

def ProcessIt(argv):    
    if len(argv)>1:        
        sFileName = argv[1]        
        img = LoadImageJPEG(sFileName)        
        imgseg,palette = SegmentImage(img,16)        
        convert = lti.usePalette()                
        imgx = lti.image()        
        convert.apply(imgseg,palette,imgx)	
        ShowImage(img)
        ShowImage(imgx)

if __name__=="__main__":    
    if len(sys.argv)<2:        
        PrintUsage()    
    else:        
        ProcessIt(sys.argv)
