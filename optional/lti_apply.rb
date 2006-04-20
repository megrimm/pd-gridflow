DgenericVector.form = [
[[], ["double (*)(double)"]]
[["GenericVector<double >"], ["double (*)(double)"]]
[["double (*)(double const &)"], []]
[["GenericVector<double >", "double (*)(double const &)"], []]
[["GenericVector<double >", "double (*)(double const &,double const &)"], []]
[["GenericVector<double >"], ["double (*)(double,double)"]]
[["GenericVector<double >", "GenericVector<double >", "double (*)(double const &,double const &)"], []]
[["GenericVector<double >", "GenericVector<double >"], ["double (*)(double,double)"]]
]

FgenericVector.form = [
[[], ["float (*)(float)"]]
[["GenericVector<float >"], ["float (*)(float)"]]
[["float (*)(float const &)"], []]
[["GenericVector<float >", "float (*)(float const &)"], []]
[["GenericVector<float >", "float (*)(float const &,float const &)"], []]
[["GenericVector<float >"], ["float (*)(float,float)"]]
[["GenericVector<float >", "GenericVector<float >", "float (*)(float const &,float const &)"], []]
[["GenericVector<float >", "GenericVector<float >"], ["float (*)(float,float)"]]
]

IgenericVector.form = [
[[], ["int (*)(int)"]]
[["GenericVector<int >"], ["int (*)(int)"]]
[["int (*)(int const &)"], []]
[["GenericVector<int >", "int (*)(int const &)"], []]
[["GenericVector<int >", "int (*)(int const &,int const &)"], []]
[["GenericVector<int >"], ["int (*)(int,int)"]]
[["GenericVector<int >", "GenericVector<int >", "int (*)(int const &,int const &)"], []]
[["GenericVector<int >", "GenericVector<int >"], ["int (*)(int,int)"]]
]

UgenericVector.form = [
[[], ["unsigned char (*)(unsigned char)"]]
[["GenericVector<unsigned char >"], ["unsigned char (*)(unsigned char)"]]
[["unsigned char (*)(unsigned char const &)"], []]
[["GenericVector<unsigned char >", "unsigned char (*)(unsigned char const &)"], []]
[["GenericVector<unsigned char >", "unsigned char (*)(unsigned char const &,unsigned char const &)"], []]
[["GenericVector<unsigned char >"], ["unsigned char (*)(unsigned char,unsigned char)"]]
[["GenericVector<unsigned char >", "GenericVector<unsigned char >", "unsigned char (*)(unsigned char const &,unsigned char const &)"], []]
[["GenericVector<unsigned char >", "GenericVector<unsigned char >"], ["unsigned char (*)(unsigned char,unsigned char)"]]
]

RgbgenericVector.form = [
[[], ["RgbPixel (*)(lti::rgbPixel)"]]
[["GenericVector<lti::rgbPixel >"], ["RgbPixel (*)(lti::rgbPixel)"]]
[["RgbPixel (*)(lti::rgbPixel const &)"], []]
[["GenericVector<lti::rgbPixel >", "RgbPixel (*)(lti::rgbPixel const &)"], []]
[["GenericVector<lti::rgbPixel >", "RgbPixel (*)(lti::rgbPixel const &,lti::rgbPixel const &)"], []]
[["GenericVector<lti::rgbPixel >"], ["RgbPixel (*)(lti::rgbPixel,lti::rgbPixel)"]]
[["GenericVector<lti::rgbPixel >", "GenericVector<lti::rgbPixel >", "RgbPixel (*)(lti::rgbPixel const &,lti::rgbPixel const &)"], []]
[["GenericVector<lti::rgbPixel >", "GenericVector<lti::rgbPixel >"], ["RgbPixel (*)(lti::rgbPixel,lti::rgbPixel)"]]
]

Iarray.form = [
[[], ["int (*)(int)"]]
[["Array<int >"], ["int (*)(int)"]]
[["int (*)(int const &)"], []]
[["Array<int >", "int (*)(int const &)"], []]
[["Array<int >", "int (*)(int const &,int const &)"], []]
[["Array<int >"], ["int (*)(int,int)"]]
[["Array<int >", "Array<int >", "int (*)(int const &,int const &)"], []]
[["Array<int >", "Array<int >"], ["int (*)(int,int)"]]
]

Farray.form = [
[[], ["float (*)(float)"]]
[["Array<float >"], ["float (*)(float)"]]
[["float (*)(float const &)"], []]
[["Array<float >", "float (*)(float const &)"], []]
[["Array<float >", "float (*)(float const &,float const &)"], []]
[["Array<float >"], ["float (*)(float,float)"]]
[["Array<float >", "Array<float >", "float (*)(float const &,float const &)"], []]
[["Array<float >", "Array<float >"], ["float (*)(float,float)"]]
]

Darray.form = [
[[], ["double (*)(double)"]]
[["Array<double >"], ["double (*)(double)"]]
[["double (*)(double const &)"], []]
[["Array<double >", "double (*)(double const &)"], []]
[["Array<double >", "double (*)(double const &,double const &)"], []]
[["Array<double >"], ["double (*)(double,double)"]]
[["Array<double >", "Array<double >", "double (*)(double const &,double const &)"], []]
[["Array<double >", "Array<double >"], ["double (*)(double,double)"]]
]

Uarray.form = [
[[], ["unsigned char (*)(unsigned char)"]]
[["Array<unsigned char >"], ["unsigned char (*)(unsigned char)"]]
[["unsigned char (*)(unsigned char const &)"], []]
[["Array<unsigned char >", "unsigned char (*)(unsigned char const &)"], []]
[["Array<unsigned char >", "unsigned char (*)(unsigned char const &,unsigned char const &)"], []]
[["Array<unsigned char >"], ["unsigned char (*)(unsigned char,unsigned char)"]]
[["Array<unsigned char >", "Array<unsigned char >", "unsigned char (*)(unsigned char const &,unsigned char const &)"], []]
[["Array<unsigned char >", "Array<unsigned char >"], ["unsigned char (*)(unsigned char,unsigned char)"]]
]

Histogram.form = [
[[], ["double (*)(double)"]]
[["double (*)(double const &)"], []]
]

Fhistogram.form = [
[[], ["float (*)(float)"]]
[["float (*)(float const &)"], []]
]

Ihistogram.form = [
[[], ["int (*)(int)"]]
[["int (*)(int const &)"], []]
]

IsepKernel.form = [
[[], ["int (*)(int)"]]
[["int (*)(int const &)"], []]
[["SepKernel<int >"], ["int (*)(int)"]]
[["SepKernel<int >", "int (*)(int const &)"], []]
]

FsepKernel.form = [
[[], ["float (*)(float)"]]
[["float (*)(float const &)"], []]
[["SepKernel<float >"], ["float (*)(float)"]]
[["SepKernel<float >", "float (*)(float const &)"], []]
]

DsepKernel.form = [
[[], ["double (*)(double)"]]
[["double (*)(double const &)"], []]
[["SepKernel<double >"], ["double (*)(double)"]]
[["SepKernel<double >", "double (*)(double const &)"], []]
]

UsepKernel.form = [
[[], ["unsigned char (*)(unsigned char)"]]
[["unsigned char (*)(unsigned char const &)"], []]
[["SepKernel<unsigned char >"], ["unsigned char (*)(unsigned char)"]]
[["SepKernel<unsigned char >", "unsigned char (*)(unsigned char const &)"], []]
]

Modifier.form = [
[[], ["Fvector"]]
[[], ["Ivector"]]
[[], ["Vector<lti::ubyte >"]]
[[], ["Channel"]]
[[], ["Imatrix"]]
[[], ["Channel8"]]
[[], ["Image"]]
[["Fvector"], ["Fvector"]]
[["Ivector"], ["Ivector"]]
[["Vector<lti::ubyte >"], ["Vector<lti::ubyte >"]]
[["Channel"], ["Channel"]]
[["Imatrix"], ["Imatrix"]]
[["Channel8"], ["Channel8"]]
[["Image"], ["Image"]]
]

IoFunctor.form = [
[[], ["Image"]]
[[], ["Channel8", "Palette"]]
[[], ["Image", "Channel8", "Palette"]]
[["Image"], []]
[["Channel"], []]
[["Channel8", "Palette"], []]
[["Channel8"], []]
]

LoadBMP.form = [
[[], ["Image"]]
[[], ["Channel8", "Palette"]]
[[], ["Channel"]]
[[], ["Image", "Channel8", "Palette"]]
]

SaveBMP.form = [
[["Image"], []]
[["Channel"], []]
[["Channel8", "Palette"], []]
[["Channel8"], []]
]

LoadJPEG.form = [
[[], ["Image"]]
[[], ["Image", "bool"]]
]

SaveJPEG.form = [
[["Image"], []]
[["Channel"], []]
[["Channel8"], []]
]

LoadPNG.form = [
[[], ["Image"]]
[[], ["Channel8", "Palette"]]
]

SavePNG.form = [
[["Image"], []]
[["Channel"], []]
[["Channel8", "Palette"], []]
[["Channel8"], []]
]

LoadImage.form = [
[[], ["Image"]]
[[], ["Channel8", "Palette"]]
[[], ["Channel"]]
]

SaveImage.form = [
[["Image"], []]
[["Channel"], []]
[["Channel8", "Palette"], []]
[["Channel8"], []]
]

ChiSquareFunctor.form = [
[[], ["double"]]
]

RealFFT.form = [
[["Vector<float >"], ["Vector<float >", "Vector<float >"]]
[["Vector<double >"], ["Vector<double >", "Vector<double >"]]
[["Matrix<float >"], ["Matrix<float >", "Matrix<float >"]]
]

OrientedHLTransform.form = [
[["Channel", "Channel"], ["Channel32"]]
[["Channel8", "Channel"], ["Channel32"]]
[["Image", "Channel"], ["Channel32"]]
[["Channel32", "Channel"], ["Channel32"]]
]

GHoughTransform.form = [
[["Channel"], ["std::list<lti::vector<float > >"]]
[["Channel", "Channel"], ["std::list<lti::vector<float > >"]]
[["TpointList<int >", "Channel"], ["std::list<lti::vector<float > >"]]
]

OrientationMap.form = [
[["Channel"], ["Channel", "Channel"]]
[["Channel8"], ["Channel8", "Channel8"]]
]

MedianFilter.form = [
[[], ["Channel"]]
[[], ["Channel8"]]
[["Channel"], ["Channel"]]
[["Channel8"], ["Channel8"]]
]

KNearestNeighFilter.form = [
[[], ["Channel8"]]
[["Channel8"], ["Channel8"]]
[[], ["Imatrix"]]
[["Imatrix"], ["Imatrix"]]
]

Convolution.form = [
[[], ["Channel8"]]
[[], ["Channel"]]
[[], ["Dmatrix"]]
[[], ["Vector<lti::channel8::value_type >"]]
[[], ["Vector<lti::channel::value_type >"]]
[[], ["Dvector"]]
[["Channel8"], ["Channel8"]]
[["Channel"], ["Channel"]]
[["Dmatrix"], ["Dmatrix"]]
[["Vector<lti::channel8::value_type >"], ["Vector<lti::channel8::value_type >"]]
[["Vector<lti::channel::value_type >"], ["Vector<lti::channel::value_type >"]]
[["Dvector"], ["Dvector"]]
[["Image"], ["Image"]]
[[], ["Image"]]
]

GradientFunctor.form = [
[["Channel"], ["Channel", "Channel"]]
[["Channel"], ["Channel"]]
[[], ["Channel"]]
[["Channel8"], ["Channel", "Channel"]]
]

ColorContrastGradient.form = [
[["Image"], ["Channel", "Channel"]]
[["Channel"], ["Channel", "Channel"]]
[["Channel8"], ["Channel", "Channel"]]
[["Channel", "Channel", "Channel"], ["Channel", "Channel"]]
[["Channel", "Channel", "Channel"], ["Channel", "Channel", "float"]]
[["Channel", "Channel"], ["Channel", "Channel", "float"]]
[["Channel", "Channel", "Channel"], ["Channel", "Channel", "Channel", "float"]]
]

ImaskFunctor.form = [
[["Matrix<int >", "int (*)(int const &,int const &)"], ["Matrix<int >"]]
[["int (*)(int const &,int const &)"], ["Matrix<int >"]]
[["Matrix<int >", "Matrix<int >", "int (*)(int const &,int const &,int const &)"], ["Matrix<int >"]]
[["Matrix<int >", "int (*)(int const &,int const &,int const &)"], ["Matrix<int >"]]
[["Matrix<int >", "Matrix<int >"], ["Matrix<int >"]]
[["Matrix<int >"], ["Matrix<int >"]]
]

ColorQuantization.form = [
[["Image"], ["Channel8", "Palette"]]
[[], ["Image"]]
[["Image"], ["Image"]]
]

KMColorQuantization.form = [
[["Image"], ["Matrix<int >", "Palette"]]
[["Image"], ["Channel8", "Palette"]]
[[], ["Image"]]
[["Image"], ["Image"]]
]

LkmColorQuantization.form = [
[["Image"], ["Channel8", "Palette"]]
[[], ["Image"]]
[["Image"], ["Image"]]
]

ComputePalette.form = [
[["Image", "Channel8"], ["Palette"]]
[["Image", "Matrix<int >"], ["Palette"]]
[["Image", "Channel8"], ["Palette", "Vector<lti::trgbPixel<float > >"]]
[["Image", "Matrix<int >"], ["Palette", "Vector<lti::trgbPixel<float > >"]]
[["Image", "Matrix<int >"], ["Vector<lti::trgbPixel<float > >", "Ivector", "int"]]
[["Image", "Matrix<int >"], ["Vector<lti::trgbPixel<float > >", "Ivector"]]
[["Image", "Channel8"], ["Palette", "Vector<lti::trgbPixel<float > >", "Vector<int >"]]
[["Image", "Matrix<int >"], ["Palette", "Vector<lti::trgbPixel<float > >", "Vector<int >"]]
[["Image", "Matrix<int >"], ["Vector<lti::trgbPixel<float > >", "std::vector<lti::matrix<float > >", "Vector<int >"]]
[["Channel", "Channel", "Channel", "Matrix<int >"], ["Vector<lti::trgbPixel<float > >", "std::vector<lti::matrix<float > >", "Vector<int >"]]
[["Channel", "Channel", "Channel", "Matrix<int >"], ["Vector<lti::trgbPixel<float > >", "Vector<int >"]]
[["Channel", "Matrix<int >"], ["Vector<float >", "Vector<float >", "Vector<int >"]]
[["Image", "Matrix<int >"], ["int", "TrgbPixel<float >", "Matrix<float >", "int"]]
]

UsePalette.form = [
[["Matrix<lti::ubyte >"], ["Image"]]
[["Matrix<int >"], ["Image"]]
[["Matrix<lti::ubyte >", "Palette"], ["Image"]]
[["Matrix<int >", "Palette"], ["Image"]]
[["Matrix<lti::ubyte >", "Vector<float >"], ["Fmatrix"]]
[["Matrix<int >", "Vector<float >"], ["Fmatrix"]]
[["Image"], ["Matrix<lti::ubyte >"]]
[["Image"], ["Matrix<int >"]]
[["Image", "Palette"], ["Matrix<lti::ubyte >"]]
[["Image", "Palette"], ["Matrix<int >"]]
[["Image", "kdTree<lti::rgbPixel,int >"], ["Matrix<lti::ubyte >"]]
[["Image", "kdTree<lti::rgbPixel,int >"], ["Matrix<int >"]]
]

SplitImage.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
]

SplitImageTorgI.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
]

MeanShiftSegmentation.form = [
[["Image"], ["Image"]]
[["Image"], ["Imatrix"]]
[["Image"], ["Imatrix", "Palette"]]
[["Channel8", "Channel8", "Channel8"], ["Imatrix"]]
[["Image"], ["Image", "Image", "Imatrix", "Palette"]]
]

KMeansSegmentation.form = [
[["Image"], ["Matrix<int >"]]
[["Image"], ["Channel8"]]
[["Image"], ["Matrix<int >", "Palette"]]
[["Image"], ["Channel8", "Palette"]]
]

WhiteningSegmentation.form = [
[["Image", "principalComponents<float >"], ["Imatrix"]]
[["Image", "DrgbPixel", "Dmatrix"], ["Imatrix"]]
]

CsPresegmentation.form = [
[["Image"], ["Channel8"]]
]

Thresholding.form = [
[[], ["Channel8"]]
[["Channel8"], ["Channel8"]]
[["Channel8"], ["Channel8", "float", "float", "bool", "bool"]]
[["Channel8"], ["Channel8", "float", "float", "bool"]]
[["Channel8"], ["Channel8", "float", "float"]]
[["Channel8"], ["Channel8", "float"]]
[[], ["Channel"]]
[["Channel"], ["Channel8"]]
[["Channel"], ["Channel"]]
[["Channel"], ["Channel", "float", "float", "bool", "bool", "float", "float"]]
[["Channel"], ["Channel", "float", "float", "bool", "bool", "float"]]
[["Channel"], ["Channel", "float", "float", "bool", "bool"]]
[["Channel"], ["Channel", "float", "float", "bool"]]
[["Channel"], ["Channel", "float", "float"]]
[["Channel"], ["Channel", "float"]]
[["Channel8"], ["AreaPoints"]]
[["Channel"], ["AreaPoints"]]
[["Channel8", "AreaPoints"], ["AreaPoints"]]
[["Channel", "AreaPoints"], ["AreaPoints"]]
]

WatershedSegmentation.form = [
[[], ["Channel8"]]
[["Channel8"], ["Channel8"]]
[["Channel8"], ["Matrix<int >"]]
]

RegionGrowing.form = [
[[], ["Channel8"]]
[[], ["Channel"]]
[[], ["Image"]]
[["Channel8"], ["Channel8"]]
[["Channel"], ["Channel8"]]
[["Image"], ["Channel8"]]
]

Snake.form = [
[["Image"], ["AreaPoints"]]
]

RegionMerge.form = [
[["Imatrix", "Dmatrix"], ["Imatrix"]]
[["Imatrix", "Dmatrix", "Dvector"], ["Imatrix"]]
]

SimilarityMatrix.form = [
[["Image", "Imatrix"], ["int", "Dmatrix", "Dmatrix"]]
[["Image", "std::list<lti::areaPoints >"], ["Dmatrix", "Dmatrix", "Imatrix"]]
]

FastRelabeling.form = [
[[], ["Channel8"]]
[[], ["Imatrix"]]
[["Channel8"], ["Channel8"]]
[["Channel8"], ["Imatrix"]]
[["Imatrix"], ["Imatrix"]]
[["Channel8"], ["Channel8", "Ivector"]]
[["Channel8"], ["Imatrix", "Ivector"]]
[["Imatrix"], ["Imatrix", "Ivector"]]
[["Channel8"], ["Channel8", "Ivector", "std::vector<lti::areaPoints >"]]
[["Channel8"], ["Imatrix", "Ivector", "std::vector<lti::areaPoints >"]]
[["Imatrix"], ["Imatrix", "Ivector", "std::vector<lti::areaPoints >"]]
[["Channel8"], ["Channel8", "Ivector", "int"]]
[["Channel8"], ["Imatrix", "Ivector", "int"]]
[["Imatrix"], ["Imatrix", "Ivector", "int"]]
]

ObjectsFromMask.form = [
[["Channel8"], ["std::list<lti::ioPoints >"]]
[["Matrix<int >"], ["std::list<lti::ioPoints >"]]
[["Channel8"], ["std::list<lti::borderPoints >"]]
[["Imatrix"], ["std::list<lti::borderPoints >"]]
[["Channel8"], ["std::list<lti::areaPoints >"]]
[["Matrix<int >"], ["std::list<lti::areaPoints >"]]
[["Channel8"], ["std::list<lti::areaPoints >", "Matrix<int >"]]
[["Matrix<int >"], ["std::list<lti::areaPoints >", "Matrix<int >"]]
[["Channel8"], ["tree<objectStruct >"]]
[["Matrix<int >"], ["tree<objectStruct >"]]
]

BackgroundModel.form = [
[["Image"], ["Channel"]]
[["Image"], ["Channel8"]]
]

MeanshiftTracker.form = [
[["Image"], ["Trectangle<int >"]]
]

CornerDetector.form = [
[[], ["Channel8"]]
[[], ["Channel"]]
[["Channel8"], ["Channel8"]]
[["Channel"], ["Channel"]]
[["Channel8"], ["PointList"]]
[["Channel"], ["PointList"]]
]

EdgeDetector.form = [
[[], ["Channel8"]]
[[], ["Channel"]]
[["Channel8"], ["Channel8"]]
[["Channel"], ["Channel"]]
[["Channel"], ["Channel8"]]
[[], ["Image"]]
[["Image"], ["Image"]]
[["Image"], ["Channel8"]]
]

ClassicEdgeDetector.form = [
[["Channel"], ["Channel8"]]
[[], ["Channel8"]]
]

CannyEdges.form = [
[[], ["Channel8"]]
[[], ["Channel"]]
[["Channel8"], ["Channel8"]]
[["Channel8"], ["Channel8", "Channel"]]
[["Channel"], ["Channel"]]
[["Channel"], ["Channel8"]]
[["Channel"], ["Channel8", "Channel"]]
[["Image"], ["Channel8"]]
[["Image"], ["Channel8", "Channel"]]
[[], ["Image"]]
[["Image"], ["Image"]]
[["Channel", "Channel", "Channel"], ["Channel8", "Channel"]]
]

GlobalFeatureExtractor.form = [
[["Channel"], ["Dvector"]]
[["Channel8"], ["Dvector"]]
[["Image"], ["Dvector"]]
]

LocalFeatureExtractor.form = [
[["Channel", "std::list<lti::location >"], ["std::list<lti::dvector >"]]
[["Channel8", "std::list<lti::location >"], ["std::list<lti::dvector >"]]
[["std::list<lti::channel >", "std::list<lti::location >"], ["std::list<lti::dvector >"]]
[["Image", "std::list<lti::location >"], ["std::list<lti::dvector >"]]
[["Image", "Location"], ["Dvector"]]
[["Channel", "Location"], ["Dvector"]]
]

LocalMoments.form = [
[["Channel", "Location"], ["Dvector"]]
[["Image", "Location"], ["Dvector"]]
[["Image", "std::list<lti::location >"], ["std::list<lti::dvector >"]]
[["Channel", "std::list<lti::location >"], ["std::list<lti::dvector >"]]
]

GeometricFeatures.form = [
[["AreaPoints"], ["Dvector"]]
[["BorderPoints"], ["Dvector"]]
[["IoPoints"], ["Dvector"]]
[["AreaPoints"], ["std::map<std::string,double >"]]
[["BorderPoints"], ["std::map<std::string,double >"]]
[["IoPoints"], ["std::map<std::string,double >"]]
]

ChromaticityHistogram.form = [
[["Image"], ["Dvector"]]
]

GeometricFeaturesFromMask.form = [
[["Channel8"], ["std::vector<lti::rectangle >"]]
[["Channel8"], ["Imatrix", "std::vector<lti::rectangle >"]]
[["Channel8"], ["std::vector<geometricFeatureGroup0 >"]]
[["Channel8"], ["Imatrix", "std::vector<geometricFeatureGroup0 >"]]
]

MultiGeometricFeaturesFromMask.form = [
[["Channel8"], ["std::vector<std::vector<lti::rectangle > >"]]
[["Channel8"], ["Imatrix", "std::vector<std::vector<lti::rectangle > >"]]
[["Channel8"], ["std::vector<std::vector<geometricFeatureGroup0 > >"]]
[["Channel8"], ["Imatrix", "std::vector<std::vector<geometricFeatureGroup0 > >"]]
]

Morphology.form = [
[[], ["Channel"]]
[[], ["Channel8"]]
[["Channel"], ["Channel"]]
[["Channel8"], ["Channel8"]]
]

Erosion.form = [
[[], ["Channel"]]
[[], ["Channel8"]]
[[], ["Fvector"]]
[[], ["Vector<lti::channel8::value_type >"]]
[["Channel"], ["Channel"]]
[["Channel8"], ["Channel8"]]
[["Fvector"], ["Fvector"]]
[["Vector<lti::channel8::value_type >"], ["Vector<lti::channel8::value_type >"]]
]

Dilation.form = [
[[], ["Channel"]]
[[], ["Channel8"]]
[[], ["Fvector"]]
[[], ["Vector<lti::channel8::value_type >"]]
[["Channel"], ["Channel"]]
[["Channel8"], ["Channel8"]]
[["Fvector"], ["Fvector"]]
[["Vector<lti::channel8::value_type >"], ["Vector<lti::channel8::value_type >"]]
]

Skeleton.form = [
[[], ["Channel"]]
[[], ["Channel8"]]
[["Channel"], ["Channel"]]
[["Channel8"], ["Channel8"]]
]

Scaling.form = [
[[], ["Image"]]
[["Image"], ["Image"]]
[[], ["Matrix<lti::ubyte >"]]
[["Matrix<lti::ubyte >"], ["Matrix<lti::ubyte >"]]
[[], ["Matrix<float >"]]
[["Matrix<float >"], ["Matrix<float >"]]
[[], ["Matrix<int >"]]
[["Matrix<int >"], ["Matrix<int >"]]
[[], ["float", "Image"]]
[["Image"], ["float", "Image"]]
[[], ["float", "Matrix<lti::ubyte >"]]
[["Matrix<lti::ubyte >"], ["float", "Matrix<lti::ubyte >"]]
[[], ["float", "Matrix<float >"]]
[["Matrix<float >"], ["float", "Matrix<float >"]]
[[], ["float", "Matrix<int >"]]
[["Matrix<int >"], ["float", "Matrix<int >"]]
[["Tpoint<float >"], ["Image"]]
[["Tpoint<float >", "Image"], ["Image"]]
[["Tpoint<float >"], ["Matrix<lti::ubyte >"]]
[["Tpoint<float >", "Matrix<lti::ubyte >"], ["Matrix<lti::ubyte >"]]
[["Tpoint<float >"], ["Matrix<float >"]]
[["Tpoint<float >", "Matrix<float >"], ["Matrix<float >"]]
[["Tpoint<float >"], ["Matrix<int >"]]
[["Tpoint<float >", "Matrix<int >"], ["Matrix<int >"]]
]

Rotation.form = [
[[], ["Image"]]
[["Image"], ["Image"]]
[[], ["Matrix<lti::ubyte >"]]
[["Matrix<lti::ubyte >"], ["Matrix<lti::ubyte >"]]
[[], ["Matrix<float >"]]
[["Matrix<float >"], ["Matrix<float >"]]
[[], ["double", "Image"]]
[["Image"], ["double", "Image"]]
[[], ["double", "Matrix<lti::ubyte >"]]
[["Matrix<lti::ubyte >"], ["double", "Matrix<lti::ubyte >"]]
[[], ["double", "Matrix<float >"]]
[["Matrix<float >"], ["double", "Matrix<float >"]]
]

FlipImage.form = [
[[], ["Matrix<lti::rgbPixel >"]]
[["Matrix<lti::rgbPixel >"], ["Matrix<lti::rgbPixel >"]]
[[], ["Matrix<float >"]]
[["Matrix<float >"], ["Matrix<float >"]]
[[], ["Matrix<lti::ubyte >"]]
[["Matrix<lti::ubyte >"], ["Matrix<lti::ubyte >"]]
]

DistanceTransform.form = [
[[], ["Channel"]]
[[], ["Channel8"]]
[["Channel"], ["Channel"]]
[["Channel8"], ["Channel8"]]
]

BorderExtrema.form = [
[["BorderPoints"], ["PolygonPoints", "PolygonPoints"]]
[["BorderPoints"], ["PolygonPoints"]]
]

PolygonApproximation.form = [
[["BorderPoints"], ["PolygonPoints"]]
[["BorderPoints", "PointList"], ["PolygonPoints"]]
]

ConvexHull.form = [
[["PointList"], ["PolygonPoints"]]
[["IoPoints"], ["PolygonPoints"]]
[["TpointList<float >"], ["TpolygonPoints<float >"]]
[["TpointList<double >"], ["TpolygonPoints<double >"]]
]

Rclassifier.form = [
[["Dvector"], ["Classifier_outputVector"]]
]

SplitImageToHSI.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
[["RgbPixel"], []]
]

SplitImageToRGB.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel", "Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["Image"], ["Channel8", "Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
]

SplitImageToHSV.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
]

SplitImageToHLS.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
]

SplitImageToXYZ.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
]

SplitImageToxyY.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
]

SplitImageToOCP.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
]

SplitImageToYIQ.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
]

SplitImageToCIELuv.form = [
[["Image"], ["Channel", "Channel", "Channel"]]
[["RgbPixel"], ["float", "float", "float"]]
]

SplitImageToYUV.form = [
[["RgbPixel"], ["float", "float", "float"]]
[["RgbPixel"], ["Ubyte", "Ubyte", "Ubyte"]]
[["Image"], ["Channel", "Channel", "Channel"]]
[["Image"], ["Channel8", "Channel8", "Channel8"]]
