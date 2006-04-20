begin
DgenericVector.form = [
[Out["double (*)(double)"]],
[In["GenericVector<double >"], Out["double (*)(double)"]],
[In["double (*)(double const &)"]],
[In["GenericVector<double >"], In["double (*)(double const &)"]],
[In["GenericVector<double >"], In["double (*)(double const &,double const &)"]],
[In["GenericVector<double >"], Out["double (*)(double,double)"]],
[In["GenericVector<double >"], In["GenericVector<double >"], In["double (*)(double const &,double const &)"]],
[In["GenericVector<double >"], In["GenericVector<double >"], Out["double (*)(double,double)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
FgenericVector.form = [
[Out["float (*)(float)"]],
[In["GenericVector<float >"], Out["float (*)(float)"]],
[In["float (*)(float const &)"]],
[In["GenericVector<float >"], In["float (*)(float const &)"]],
[In["GenericVector<float >"], In["float (*)(float const &,float const &)"]],
[In["GenericVector<float >"], Out["float (*)(float,float)"]],
[In["GenericVector<float >"], In["GenericVector<float >"], In["float (*)(float const &,float const &)"]],
[In["GenericVector<float >"], In["GenericVector<float >"], Out["float (*)(float,float)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
IgenericVector.form = [
[Out["int (*)(int)"]],
[In["GenericVector<int >"], Out["int (*)(int)"]],
[In["int (*)(int const &)"]],
[In["GenericVector<int >"], In["int (*)(int const &)"]],
[In["GenericVector<int >"], In["int (*)(int const &,int const &)"]],
[In["GenericVector<int >"], Out["int (*)(int,int)"]],
[In["GenericVector<int >"], In["GenericVector<int >"], In["int (*)(int const &,int const &)"]],
[In["GenericVector<int >"], In["GenericVector<int >"], Out["int (*)(int,int)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
UgenericVector.form = [
[Out["unsigned char (*)(unsigned char)"]],
[In["GenericVector<unsigned char >"], Out["unsigned char (*)(unsigned char)"]],
[In["unsigned char (*)(unsigned char const &)"]],
[In["GenericVector<unsigned char >"], In["unsigned char (*)(unsigned char const &)"]],
[In["GenericVector<unsigned char >"], In["unsigned char (*)(unsigned char const &,unsigned char const &)"]],
[In["GenericVector<unsigned char >"], Out["unsigned char (*)(unsigned char,unsigned char)"]],
[In["GenericVector<unsigned char >"], In["GenericVector<unsigned char >"], In["unsigned char (*)(unsigned char const &,unsigned char const &)"]],
[In["GenericVector<unsigned char >"], In["GenericVector<unsigned char >"], Out["unsigned char (*)(unsigned char,unsigned char)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RgbgenericVector.form = [
[Out["RgbPixel (*)(lti::rgbPixel)"]],
[In["GenericVector<lti::rgbPixel >"], Out["RgbPixel (*)(lti::rgbPixel)"]],
[In["RgbPixel (*)(lti::rgbPixel const &)"]],
[In["GenericVector<lti::rgbPixel >"], In["RgbPixel (*)(lti::rgbPixel const &)"]],
[In["GenericVector<lti::rgbPixel >"], In["RgbPixel (*)(lti::rgbPixel const &,lti::rgbPixel const &)"]],
[In["GenericVector<lti::rgbPixel >"], Out["RgbPixel (*)(lti::rgbPixel,lti::rgbPixel)"]],
[In["GenericVector<lti::rgbPixel >"], In["GenericVector<lti::rgbPixel >"], In["RgbPixel (*)(lti::rgbPixel const &,lti::rgbPixel const &)"]],
[In["GenericVector<lti::rgbPixel >"], In["GenericVector<lti::rgbPixel >"], Out["RgbPixel (*)(lti::rgbPixel,lti::rgbPixel)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Iarray.form = [
[Out["int (*)(int)"]],
[In["Array<int >"], Out["int (*)(int)"]],
[In["int (*)(int const &)"]],
[In["Array<int >"], In["int (*)(int const &)"]],
[In["Array<int >"], In["int (*)(int const &,int const &)"]],
[In["Array<int >"], Out["int (*)(int,int)"]],
[In["Array<int >"], In["Array<int >"], In["int (*)(int const &,int const &)"]],
[In["Array<int >"], In["Array<int >"], Out["int (*)(int,int)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Farray.form = [
[Out["float (*)(float)"]],
[In["Array<float >"], Out["float (*)(float)"]],
[In["float (*)(float const &)"]],
[In["Array<float >"], In["float (*)(float const &)"]],
[In["Array<float >"], In["float (*)(float const &,float const &)"]],
[In["Array<float >"], Out["float (*)(float,float)"]],
[In["Array<float >"], In["Array<float >"], In["float (*)(float const &,float const &)"]],
[In["Array<float >"], In["Array<float >"], Out["float (*)(float,float)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Darray.form = [
[Out["double (*)(double)"]],
[In["Array<double >"], Out["double (*)(double)"]],
[In["double (*)(double const &)"]],
[In["Array<double >"], In["double (*)(double const &)"]],
[In["Array<double >"], In["double (*)(double const &,double const &)"]],
[In["Array<double >"], Out["double (*)(double,double)"]],
[In["Array<double >"], In["Array<double >"], In["double (*)(double const &,double const &)"]],
[In["Array<double >"], In["Array<double >"], Out["double (*)(double,double)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Uarray.form = [
[Out["unsigned char (*)(unsigned char)"]],
[In["Array<unsigned char >"], Out["unsigned char (*)(unsigned char)"]],
[In["unsigned char (*)(unsigned char const &)"]],
[In["Array<unsigned char >"], In["unsigned char (*)(unsigned char const &)"]],
[In["Array<unsigned char >"], In["unsigned char (*)(unsigned char const &,unsigned char const &)"]],
[In["Array<unsigned char >"], Out["unsigned char (*)(unsigned char,unsigned char)"]],
[In["Array<unsigned char >"], In["Array<unsigned char >"], In["unsigned char (*)(unsigned char const &,unsigned char const &)"]],
[In["Array<unsigned char >"], In["Array<unsigned char >"], Out["unsigned char (*)(unsigned char,unsigned char)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Histogram.form = [
[Out["double (*)(double)"]],
[In["double (*)(double const &)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Fhistogram.form = [
[Out["float (*)(float)"]],
[In["float (*)(float const &)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Ihistogram.form = [
[Out["int (*)(int)"]],
[In["int (*)(int const &)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
IsepKernel.form = [
[Out["int (*)(int)"]],
[In["int (*)(int const &)"]],
[In["SepKernel<int >"], Out["int (*)(int)"]],
[In["SepKernel<int >"], In["int (*)(int const &)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
FsepKernel.form = [
[Out["float (*)(float)"]],
[In["float (*)(float const &)"]],
[In["SepKernel<float >"], Out["float (*)(float)"]],
[In["SepKernel<float >"], In["float (*)(float const &)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
DsepKernel.form = [
[Out["double (*)(double)"]],
[In["double (*)(double const &)"]],
[In["SepKernel<double >"], Out["double (*)(double)"]],
[In["SepKernel<double >"], In["double (*)(double const &)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
UsepKernel.form = [
[Out["unsigned char (*)(unsigned char)"]],
[In["unsigned char (*)(unsigned char const &)"]],
[In["SepKernel<unsigned char >"], Out["unsigned char (*)(unsigned char)"]],
[In["SepKernel<unsigned char >"], In["unsigned char (*)(unsigned char const &)"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Modifier.form = [
[Out["Fvector"]],
[Out["Ivector"]],
[Out["Vector<lti::ubyte >"]],
[Out["Channel"]],
[Out["Imatrix"]],
[Out["Channel8"]],
[Out["Image"]],
[In["Fvector"], Out["Fvector"]],
[In["Ivector"], Out["Ivector"]],
[In["Vector<lti::ubyte >"], Out["Vector<lti::ubyte >"]],
[In["Channel"], Out["Channel"]],
[In["Imatrix"], Out["Imatrix"]],
[In["Channel8"], Out["Channel8"]],
[In["Image"], Out["Image"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
IoFunctor.form = [
[Out["Image"]],
[Out["Channel8"], Out["Palette"]],
[Out["Image"], Out["Channel8"], Out["Palette"]],
[In["Image"]],
[In["Channel"]],
[In["Channel8"], In["Palette"]],
[In["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadBMP.form = [
[Out["Image"]],
[Out["Channel8"], Out["Palette"]],
[Out["Channel"]],
[Out["Image"], Out["Channel8"], Out["Palette"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SaveBMP.form = [
[In["Image"]],
[In["Channel"]],
[In["Channel8"], In["Palette"]],
[In["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadJPEG.form = [
[Out["Image"]],
[Out["Image"], Out["bool"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SaveJPEG.form = [
[In["Image"]],
[In["Channel"]],
[In["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadPNG.form = [
[Out["Image"]],
[Out["Channel8"], Out["Palette"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SavePNG.form = [
[In["Image"]],
[In["Channel"]],
[In["Channel8"], In["Palette"]],
[In["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadImage.form = [
[Out["Image"]],
[Out["Channel8"], Out["Palette"]],
[Out["Channel"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SaveImage.form = [
[In["Image"]],
[In["Channel"]],
[In["Channel8"], In["Palette"]],
[In["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ChiSquareFunctor.form = [
[Out["double"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RealFFT.form = [
[In["Vector<float >"], Out["Vector<float >"], Out["Vector<float >"]],
[In["Vector<double >"], Out["Vector<double >"], Out["Vector<double >"]],
[In["Matrix<float >"], Out["Matrix<float >"], Out["Matrix<float >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
OrientedHLTransform.form = [
[In["Channel"], In["Channel"], Out["Channel32"]],
[In["Channel8"], In["Channel"], Out["Channel32"]],
[In["Image"], In["Channel"], Out["Channel32"]],
[In["Channel32"], In["Channel"], Out["Channel32"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GHoughTransform.form = [
[In["Channel"], Out["std::list<lti::vector<float > >"]],
[In["Channel"], In["Channel"], Out["std::list<lti::vector<float > >"]],
[In["TpointList<int >"], In["Channel"], Out["std::list<lti::vector<float > >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
OrientationMap.form = [
[In["Channel"], Out["Channel"], Out["Channel"]],
[In["Channel8"], Out["Channel8"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MedianFilter.form = [
[Out["Channel"]],
[Out["Channel8"]],
[In["Channel"], Out["Channel"]],
[In["Channel8"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
KNearestNeighFilter.form = [
[Out["Channel8"]],
[In["Channel8"], Out["Channel8"]],
[Out["Imatrix"]],
[In["Imatrix"], Out["Imatrix"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Convolution.form = [
[Out["Channel8"]],
[Out["Channel"]],
[Out["Dmatrix"]],
[Out["Vector<lti::channel8::value_type >"]],
[Out["Vector<lti::channel::value_type >"]],
[Out["Dvector"]],
[In["Channel8"], Out["Channel8"]],
[In["Channel"], Out["Channel"]],
[In["Dmatrix"], Out["Dmatrix"]],
[In["Vector<lti::channel8::value_type >"], Out["Vector<lti::channel8::value_type >"]],
[In["Vector<lti::channel::value_type >"], Out["Vector<lti::channel::value_type >"]],
[In["Dvector"], Out["Dvector"]],
[In["Image"], Out["Image"]],
[Out["Image"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GradientFunctor.form = [
[In["Channel"], Out["Channel"], Out["Channel"]],
[In["Channel"], Out["Channel"]],
[Out["Channel"]],
[In["Channel8"], Out["Channel"], Out["Channel"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ColorContrastGradient.form = [
[In["Image"], Out["Channel"], Out["Channel"]],
[In["Channel"], Out["Channel"], Out["Channel"]],
[In["Channel8"], Out["Channel"], Out["Channel"]],
[In["Channel"], In["Channel"], In["Channel"], Out["Channel"], Out["Channel"]],
[In["Channel"], In["Channel"], In["Channel"], Out["Channel"], Out["Channel"], Out["float"]],
[In["Channel"], In["Channel"], Out["Channel"], Out["Channel"], Out["float"]],
[In["Channel"], In["Channel"], In["Channel"], Out["Channel"], Out["Channel"], Out["Channel"], Out["float"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ImaskFunctor.form = [
[In["Matrix<int >"], Out["Matrix<int >"], In["int (*)(int const &,int const &)"]],
[Out["Matrix<int >"], In["int (*)(int const &,int const &)"]],
[In["Matrix<int >"], In["Matrix<int >"], Out["Matrix<int >"], In["int (*)(int const &,int const &,int const &)"]],
[In["Matrix<int >"], Out["Matrix<int >"], In["int (*)(int const &,int const &,int const &)"]],
[In["Matrix<int >"], In["Matrix<int >"], Out["Matrix<int >"]],
[In["Matrix<int >"], Out["Matrix<int >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ColorQuantization.form = [
[In["Image"], Out["Channel8"], Out["Palette"]],
[Out["Image"]],
[In["Image"], Out["Image"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
KMColorQuantization.form = [
[In["Image"], Out["Matrix<int >"], Out["Palette"]],
[In["Image"], Out["Channel8"], Out["Palette"]],
[Out["Image"]],
[In["Image"], Out["Image"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LkmColorQuantization.form = [
[In["Image"], Out["Channel8"], Out["Palette"]],
[Out["Image"]],
[In["Image"], Out["Image"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ComputePalette.form = [
[In["Image"], In["Channel8"], Out["Palette"]],
[In["Image"], In["Matrix<int >"], Out["Palette"]],
[In["Image"], In["Channel8"], Out["Palette"], Out["Vector<lti::trgbPixel<float > >"]],
[In["Image"], In["Matrix<int >"], Out["Palette"], Out["Vector<lti::trgbPixel<float > >"]],
[In["Image"], In["Matrix<int >"], Out["Vector<lti::trgbPixel<float > >"], Out["Ivector"], Out["int"]],
[In["Image"], In["Matrix<int >"], Out["Vector<lti::trgbPixel<float > >"], Out["Ivector"]],
[In["Image"], In["Channel8"], Out["Palette"], Out["Vector<lti::trgbPixel<float > >"], Out["Vector<int >"]],
[In["Image"], In["Matrix<int >"], Out["Palette"], Out["Vector<lti::trgbPixel<float > >"], Out["Vector<int >"]],
[In["Image"], In["Matrix<int >"], Out["Vector<lti::trgbPixel<float > >"], Out["std::vector<lti::matrix<float > >"], Out["Vector<int >"]],
[In["Channel"], In["Channel"], In["Channel"], In["Matrix<int >"], Out["Vector<lti::trgbPixel<float > >"], Out["std::vector<lti::matrix<float > >"], Out["Vector<int >"]],
[In["Channel"], In["Channel"], In["Channel"], In["Matrix<int >"], Out["Vector<lti::trgbPixel<float > >"], Out["Vector<int >"]],
[In["Channel"], In["Matrix<int >"], Out["Vector<float >"], Out["Vector<float >"], Out["Vector<int >"]],
[In["Image"], In["Matrix<int >"], Out["int"], Out["TrgbPixel<float >"], Out["Matrix<float >"], Out["int"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
UsePalette.form = [
[In["Matrix<lti::ubyte >"], Out["Image"]],
[In["Matrix<int >"], Out["Image"]],
[In["Matrix<lti::ubyte >"], In["Palette"], Out["Image"]],
[In["Matrix<int >"], In["Palette"], Out["Image"]],
[In["Matrix<lti::ubyte >"], In["Vector<float >"], Out["Fmatrix"]],
[In["Matrix<int >"], In["Vector<float >"], Out["Fmatrix"]],
[In["Image"], Out["Matrix<lti::ubyte >"]],
[In["Image"], Out["Matrix<int >"]],
[In["Image"], In["Palette"], Out["Matrix<lti::ubyte >"]],
[In["Image"], In["Palette"], Out["Matrix<int >"]],
[In["Image"], In["kdTree<lti::rgbPixel,int >"], Out["Matrix<lti::ubyte >"]],
[In["Image"], In["kdTree<lti::rgbPixel,int >"], Out["Matrix<int >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImage.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageTorgI.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MeanShiftSegmentation.form = [
[In["Image"], Out["Image"]],
[In["Image"], Out["Imatrix"]],
[In["Image"], Out["Imatrix"], Out["Palette"]],
[In["Channel8"], In["Channel8"], In["Channel8"], Out["Imatrix"]],
[In["Image"], Out["Image"], Out["Image"], Out["Imatrix"], Out["Palette"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
KMeansSegmentation.form = [
[In["Image"], Out["Matrix<int >"]],
[In["Image"], Out["Channel8"]],
[In["Image"], Out["Matrix<int >"], Out["Palette"]],
[In["Image"], Out["Channel8"], Out["Palette"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
WhiteningSegmentation.form = [
[In["Image"], In["principalComponents<float >"], Out["Imatrix"]],
[In["Image"], In["DrgbPixel"], In["Dmatrix"], Out["Imatrix"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CsPresegmentation.form = [
[In["Image"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Thresholding.form = [
[Out["Channel8"]],
[In["Channel8"], Out["Channel8"]],
[In["Channel8"], Out["Channel8"], Out["float"], Out["float"], Out["bool"], Out["bool"]],
[In["Channel8"], Out["Channel8"], Out["float"], Out["float"], Out["bool"]],
[In["Channel8"], Out["Channel8"], Out["float"], Out["float"]],
[In["Channel8"], Out["Channel8"], Out["float"]],
[Out["Channel"]],
[In["Channel"], Out["Channel8"]],
[In["Channel"], Out["Channel"]],
[In["Channel"], Out["Channel"], Out["float"], Out["float"], Out["bool"], Out["bool"], Out["float"], Out["float"]],
[In["Channel"], Out["Channel"], Out["float"], Out["float"], Out["bool"], Out["bool"], Out["float"]],
[In["Channel"], Out["Channel"], Out["float"], Out["float"], Out["bool"], Out["bool"]],
[In["Channel"], Out["Channel"], Out["float"], Out["float"], Out["bool"]],
[In["Channel"], Out["Channel"], Out["float"], Out["float"]],
[In["Channel"], Out["Channel"], Out["float"]],
[In["Channel8"], Out["AreaPoints"]],
[In["Channel"], Out["AreaPoints"]],
[In["Channel8"], In["AreaPoints"], Out["AreaPoints"]],
[In["Channel"], In["AreaPoints"], Out["AreaPoints"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
WatershedSegmentation.form = [
[Out["Channel8"]],
[In["Channel8"], Out["Channel8"]],
[In["Channel8"], Out["Matrix<int >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RegionGrowing.form = [
[Out["Channel8"]],
[Out["Channel"]],
[Out["Image"]],
[In["Channel8"], Out["Channel8"]],
[In["Channel"], Out["Channel8"]],
[In["Image"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Snake.form = [
[In["Image"], Out["AreaPoints"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RegionMerge.form = [
[In["Imatrix"], In["Dmatrix"], Out["Imatrix"]],
[In["Imatrix"], In["Dmatrix"], In["Dvector"], Out["Imatrix"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SimilarityMatrix.form = [
[In["Image"], Out["int"], In["Imatrix"], Out["Dmatrix"], Out["Dmatrix"]],
[In["Image"], In["std::list<lti::areaPoints >"], Out["Dmatrix"], Out["Dmatrix"], Out["Imatrix"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
FastRelabeling.form = [
[Out["Channel8"]],
[Out["Imatrix"]],
[In["Channel8"], Out["Channel8"]],
[In["Channel8"], Out["Imatrix"]],
[In["Imatrix"], Out["Imatrix"]],
[In["Channel8"], Out["Channel8"], Out["Ivector"]],
[In["Channel8"], Out["Imatrix"], Out["Ivector"]],
[In["Imatrix"], Out["Imatrix"], Out["Ivector"]],
[In["Channel8"], Out["Channel8"], Out["Ivector"], Out["std::vector<lti::areaPoints >"]],
[In["Channel8"], Out["Imatrix"], Out["Ivector"], Out["std::vector<lti::areaPoints >"]],
[In["Imatrix"], Out["Imatrix"], Out["Ivector"], Out["std::vector<lti::areaPoints >"]],
[In["Channel8"], Out["Channel8"], Out["Ivector"], Out["int"]],
[In["Channel8"], Out["Imatrix"], Out["Ivector"], Out["int"]],
[In["Imatrix"], Out["Imatrix"], Out["Ivector"], Out["int"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ObjectsFromMask.form = [
[In["Channel8"], Out["std::list<lti::ioPoints >"]],
[In["Matrix<int >"], Out["std::list<lti::ioPoints >"]],
[In["Channel8"], Out["std::list<lti::borderPoints >"]],
[In["Imatrix"], Out["std::list<lti::borderPoints >"]],
[In["Channel8"], Out["std::list<lti::areaPoints >"]],
[In["Matrix<int >"], Out["std::list<lti::areaPoints >"]],
[In["Channel8"], Out["std::list<lti::areaPoints >"], Out["Matrix<int >"]],
[In["Matrix<int >"], Out["std::list<lti::areaPoints >"], Out["Matrix<int >"]],
[In["Channel8"], Out["tree<objectStruct >"]],
[In["Matrix<int >"], Out["tree<objectStruct >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
BackgroundModel.form = [
[In["Image"], Out["Channel"]],
[In["Image"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MeanshiftTracker.form = [
[In["Image"], Out["Trectangle<int >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CornerDetector.form = [
[Out["Channel8"]],
[Out["Channel"]],
[In["Channel8"], Out["Channel8"]],
[In["Channel"], Out["Channel"]],
[In["Channel8"], Out["PointList"]],
[In["Channel"], Out["PointList"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
EdgeDetector.form = [
[Out["Channel8"]],
[Out["Channel"]],
[In["Channel8"], Out["Channel8"]],
[In["Channel"], Out["Channel"]],
[In["Channel"], Out["Channel8"]],
[Out["Image"]],
[In["Image"], Out["Image"]],
[In["Image"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ClassicEdgeDetector.form = [
[In["Channel"], Out["Channel8"]],
[Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CannyEdges.form = [
[Out["Channel8"]],
[Out["Channel"]],
[In["Channel8"], Out["Channel8"]],
[In["Channel8"], Out["Channel8"], Out["Channel"]],
[In["Channel"], Out["Channel"]],
[In["Channel"], Out["Channel8"]],
[In["Channel"], Out["Channel8"], Out["Channel"]],
[In["Image"], Out["Channel8"]],
[In["Image"], Out["Channel8"], Out["Channel"]],
[Out["Image"]],
[In["Image"], Out["Image"]],
[In["Channel"], In["Channel"], In["Channel"], Out["Channel8"], Out["Channel"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GlobalFeatureExtractor.form = [
[In["Channel"], Out["Dvector"]],
[In["Channel8"], Out["Dvector"]],
[In["Image"], Out["Dvector"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LocalFeatureExtractor.form = [
[In["Channel"], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In["Channel8"], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In["std::list<lti::channel >"], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In["Image"], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In["Image"], In["Location"], Out["Dvector"]],
[In["Channel"], In["Location"], Out["Dvector"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LocalMoments.form = [
[In["Channel"], In["Location"], Out["Dvector"]],
[In["Image"], In["Location"], Out["Dvector"]],
[In["Image"], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In["Channel"], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GeometricFeatures.form = [
[In["AreaPoints"], Out["Dvector"]],
[In["BorderPoints"], Out["Dvector"]],
[In["IoPoints"], Out["Dvector"]],
[In["AreaPoints"], Out["std::map<std::string,double >"]],
[In["BorderPoints"], Out["std::map<std::string,double >"]],
[In["IoPoints"], Out["std::map<std::string,double >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ChromaticityHistogram.form = [
[In["Image"], Out["Dvector"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GeometricFeaturesFromMask.form = [
[In["Channel8"], Out["std::vector<lti::rectangle >"]],
[In["Channel8"], Out["Imatrix"], Out["std::vector<lti::rectangle >"]],
[In["Channel8"], Out["std::vector<geometricFeatureGroup0 >"]],
[In["Channel8"], Out["Imatrix"], Out["std::vector<geometricFeatureGroup0 >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MultiGeometricFeaturesFromMask.form = [
[In["Channel8"], Out["std::vector<std::vector<lti::rectangle > >"]],
[In["Channel8"], Out["Imatrix"], Out["std::vector<std::vector<lti::rectangle > >"]],
[In["Channel8"], Out["std::vector<std::vector<geometricFeatureGroup0 > >"]],
[In["Channel8"], Out["Imatrix"], Out["std::vector<std::vector<geometricFeatureGroup0 > >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Morphology.form = [
[Out["Channel"]],
[Out["Channel8"]],
[In["Channel"], Out["Channel"]],
[In["Channel8"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Erosion.form = [
[Out["Channel"]],
[Out["Channel8"]],
[Out["Fvector"]],
[Out["Vector<lti::channel8::value_type >"]],
[In["Channel"], Out["Channel"]],
[In["Channel8"], Out["Channel8"]],
[In["Fvector"], Out["Fvector"]],
[In["Vector<lti::channel8::value_type >"], Out["Vector<lti::channel8::value_type >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Dilation.form = [
[Out["Channel"]],
[Out["Channel8"]],
[Out["Fvector"]],
[Out["Vector<lti::channel8::value_type >"]],
[In["Channel"], Out["Channel"]],
[In["Channel8"], Out["Channel8"]],
[In["Fvector"], Out["Fvector"]],
[In["Vector<lti::channel8::value_type >"], Out["Vector<lti::channel8::value_type >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Skeleton.form = [
[Out["Channel"]],
[Out["Channel8"]],
[In["Channel"], Out["Channel"]],
[In["Channel8"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Scaling.form = [
[Out["Image"]],
[In["Image"], Out["Image"]],
[Out["Matrix<lti::ubyte >"]],
[In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[Out["Matrix<float >"]],
[In["Matrix<float >"], Out["Matrix<float >"]],
[Out["Matrix<int >"]],
[In["Matrix<int >"], Out["Matrix<int >"]],
[Out["float"], Out["Image"]],
[Out["float"], In["Image"], Out["Image"]],
[Out["float"], Out["Matrix<lti::ubyte >"]],
[Out["float"], In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[Out["float"], Out["Matrix<float >"]],
[Out["float"], In["Matrix<float >"], Out["Matrix<float >"]],
[Out["float"], Out["Matrix<int >"]],
[Out["float"], In["Matrix<int >"], Out["Matrix<int >"]],
[In["Tpoint<float >"], Out["Image"]],
[In["Tpoint<float >"], In["Image"], Out["Image"]],
[In["Tpoint<float >"], Out["Matrix<lti::ubyte >"]],
[In["Tpoint<float >"], In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[In["Tpoint<float >"], Out["Matrix<float >"]],
[In["Tpoint<float >"], In["Matrix<float >"], Out["Matrix<float >"]],
[In["Tpoint<float >"], Out["Matrix<int >"]],
[In["Tpoint<float >"], In["Matrix<int >"], Out["Matrix<int >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Rotation.form = [
[Out["Image"]],
[In["Image"], Out["Image"]],
[Out["Matrix<lti::ubyte >"]],
[In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[Out["Matrix<float >"]],
[In["Matrix<float >"], Out["Matrix<float >"]],
[Out["double"], Out["Image"]],
[Out["double"], In["Image"], Out["Image"]],
[Out["double"], Out["Matrix<lti::ubyte >"]],
[Out["double"], In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[Out["double"], Out["Matrix<float >"]],
[Out["double"], In["Matrix<float >"], Out["Matrix<float >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
FlipImage.form = [
[Out["Matrix<lti::rgbPixel >"]],
[In["Matrix<lti::rgbPixel >"], Out["Matrix<lti::rgbPixel >"]],
[Out["Matrix<float >"]],
[In["Matrix<float >"], Out["Matrix<float >"]],
[Out["Matrix<lti::ubyte >"]],
[In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
DistanceTransform.form = [
[Out["Channel"]],
[Out["Channel8"]],
[In["Channel"], Out["Channel"]],
[In["Channel8"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
BorderExtrema.form = [
[In["BorderPoints"], Out["PolygonPoints"], Out["PolygonPoints"]],
[In["BorderPoints"], Out["PolygonPoints"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
PolygonApproximation.form = [
[In["BorderPoints"], Out["PolygonPoints"]],
[In["BorderPoints"], In["PointList"], Out["PolygonPoints"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ConvexHull.form = [
[In["PointList"], Out["PolygonPoints"]],
[In["IoPoints"], Out["PolygonPoints"]],
[In["TpointList<float >"], Out["TpolygonPoints<float >"]],
[In["TpointList<double >"], Out["TpolygonPoints<double >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Rclassifier.form = [
[In["Dvector"], Out["Classifier_outputVector"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSI.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
[In["RgbPixel"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToRGB.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSV.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHLS.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToXYZ.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToxyY.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToOCP.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYIQ.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToCIELuv.form = [
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYUV.form = [
[In["RgbPixel"], Out["float"], Out["float"], Out["float"]],
[In["RgbPixel"], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
[In["Image"], Out["Channel"], Out["Channel"], Out["Channel"]],
[In["Image"], Out["Channel8"], Out["Channel8"], Out["Channel8"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

