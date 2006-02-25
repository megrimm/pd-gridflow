require 'rblti'

def ShowImage(img)   
 view = Rblti::ExternViewer.new    
 view.show(img)
end

def LoadImageJPEG(sName)
 img=Rblti::Image.new
 param = Rblti::RioJPEG_parameters.new
 Rblti::_setStdString(param.filename, sName)
 lo = Rblti::LoadJPEG.new    
 lo.setParameters(param)    
 ok = lo.apply(img)    
 if not ok        
  img = nil
 end    
 return img
end

def SegmentImage(img,noOfColors=16)   
    functor = Rblti::KMeansSegmentation.new    
    params = Rblti::RkMeansSegmentation_parameters.new  
    paramsColor = Rblti::RkMColorQuantization_parameters.new    
    paramsColor.numberOfColors = noOfColors    
    params.setQuantParameters(paramsColor)    
    functor.setParameters(params)    
    matrixout = Rblti::Imatrix.new    
    paletteout = Rblti::Palette.new    
    functor.apply(img,matrixout,paletteout)    
    return matrixout,paletteout
end

def ProcessIt(argv)
 sFileName = argv
 img = LoadImageJPEG(sFileName)
 imgseg,palette = SegmentImage(img,16)
 convert = Rblti::UsePalette.new
 imgx = Rblti::Image.new
 convert.apply(imgseg,palette,imgx)
 ShowImage(img)
 ShowImage(imgx)
end

ProcessIt('lemur.jpeg')
