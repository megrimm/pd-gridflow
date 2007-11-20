#$Id$

NOTE: This file (the text below the dotted line)  is slightly
out of date. This folder is a version of pylti that supports
ruby (rblti) and compiles with gcc. Make sure that gcc 3.x
is being used because LTIlib still can't be compiled using
gcc 4.x and because C++ libraries can only be linked together
if they use the same major version of gcc. If you want to use
LTIlib with GridFlow then GridFlow must also be compiled using
gcc 3.x.

------------------8<--------cut-here--------8<------------------
This is pylti, a project to build Python bindings for the ltilib.

Requires:
    * Python 2.4
    * ltilib 1.9.12
    * Microsoft .NET 2003 C++

Optional:
    * swig 1.3.24 or higher

Install:
    * you have to compile your ltilib with the option MFC_Shared_Release=1
    * extract the archive in the misc directory where ltilib is installed.
      This will create a subdirectoy pylti-version-no.
    * open a shell and enter the installation directory (example: ltilib/misc/pylti-0.32), 
      than run python setup.py install to build the wrapper modules and to install these in 
      your python installation.
    * if you want to add new features to pylti you need swig to generate the wrapper code. 
      You need at least a swig version 1.3.24 or higher.
    * optional: to create the generated parameter header files in the generated-directory 
      for the support of the nested parameter-classes: create doxygen XML output for the 
      ltilib (in directory ltilib/xml) and run python gen_ltilib_classes.py
 
License: LGPL

Authors: Michael Neuroth

-----------------------------------------------------------------------------
Supported ltilib classes and functions:

_cannyEdges_parameters
_chromaticityHistogram_parameters
_classicEdgeDetector_parameters
_classifier_outputTemplate
_classifier_outputVector
_classifier_parameters
_colorContrastGradient_parameters
_colorQuantization_parameters
_convolution_parameters
_csPresegmentation_parameters
_dilation_parameters
_distanceTransform_parameters
_edgeDetector_parameters
_erosion_parameters
_externViewer_parameters
_featureExtractor_parameters
_filter_parameters
_functor_parameters
_geometricFeatures_parameters
_getStdString
_globalFeatureExtractor_parameters
_gradientFunctor_parameters
_ioBMP_parameters
_ioFunctor_parameters
_ioImage_parameters
_ioJPEG_parameters
_ioPNG_parameters
_kMColorQuantization_parameters
_kMeansSegmentation_parameters
_localFeatureExtractor_parameters
_localMoments_parameters
_meanShiftSegmentation_parameters
_modifier_parameters
_morphology_parameters
_newclass
_object
_objectsFromMask_objectStruct
_objectsFromMask_parameters
_polygonApproximation_parameters
_regionGrowing_parameters
_segmentation_parameters
_setStdString
_skeleton_parameters
_swig_getattr
_swig_setattr
_swig_setattr_nondynamic
_transform_parameters
_usePalette_parameters
_viewerBase_parameters
_whiteningSegmentation_parameters
areaPoints
barray
bgenericMatrix
bgenericVector
bkernel1D
bkernel2D
bmatrix
borderPoints
cannyEdges
chainCode
channel
channel32
channel8
chromaticityHistogram
classicEdgeDetector
classifier
colorContrastGradient
colorQuantization
convertToSimpleTree
convolution
csPresegmentation
cvar
darray
delete_doubleArray
delete_floatArray
delete_intArray
dgenericMatrix
dgenericVector
dilation
distanceTransform
dkernel1D
dkernel2D
dmatrix
doubleArr
doubleArr_frompointer
doubleArray_getitem
doubleArray_setitem
dpoint
dsepKernel
dvector
edgeDetector
emptyPalette
endianness
erosion
externViewer
fandoKernelXX
fandoKernelXY
fandoKernelYY
farray
featureExtractor
fgenericMatrix
fgenericVector
filter
fkernel1D
fkernel2D
floatArr
floatArr_frompointer
floatArray_getitem
floatArray_setitem
fmatrix
fpoint
fsepKernel
functor
fvector
geometricFeatures
getMyTree
getTestTree
getTestTree2
globalFeatureExtractor
gradientFunctor
hessianKernelXX
hessianKernelXY
hessianKernelYY
histogram
histogram1D
histogram2D
histogram_outerBoundsCell
iandoKernelXX
iandoKernelXY
iandoKernelYY
iarray
igenericMatrix
igenericVector
ikernel1D
ikernel2D
image
imatrix
intArr
intArr_frompointer
intArray_getitem
intArray_setitem
ioBMP
ioFunctor
ioHandler
ioImage
ioJPEG
ioObject
ioPNG
ioPoints
ipoint
ipolygonPoints
isNull
isepKernel
ivector
kMColorQuantization
kMeansSegmentation
laplacianKernel
list_areaPoints
list_borderPoints
list_ioPoints
list_ipoint
loadBMP
loadImage
loadJPEG
loadPNG
localFeatureExtractor
localMoments
mathObject
meanShiftSegmentation
modifier
morphology
new_doubleArray
new_floatArray
new_intArray
notNull
object
objectsFromMask
palette
pointList
polygonApproximation
polygonPoints
read
regionGrowing
rgbPixel
rgbPixelgenericMatrix
rgbPixelmatrix
rgbgenericVector
saveBMP
saveImage
saveJPEG
savePNG
segmentation
simple_tree_double
simple_tree_objectStruct
simple_tree_objectsFromMask_objectStruct
skeleton
splitImage
splitImageToHLS
splitImageToHSI
splitImageToHSV
splitImageToRGB
splitImageToYUV
splitImageTorgI
supervisedInstanceClassifier
transform
usePalette
usepKernel
uvector
viewerBase
whiteningSegmentation
write
