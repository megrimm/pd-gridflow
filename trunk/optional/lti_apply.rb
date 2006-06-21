begin
Modifier.forms = [
[Out[Rblti::Fvector]],
[Out[Rblti::Ivector]],
[Out[Rblti::Uvector]],
[Out[Rblti::Channel]],
[Out[Rblti::Imatrix]],
[Out[Rblti::Channel8]],
[Out[Rblti::Image]],
[In[Rblti::Fvector], Out[Rblti::Fvector]],
[In[Rblti::Ivector], Out[Rblti::Ivector]],
[In[Rblti::Uvector], Out[Rblti::Uvector]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Image]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
IoFunctor.forms = [
[Out[Rblti::Image]],
[Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8], In[Rblti::Palette]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadBMP.forms = [
[Out[Rblti::Image]],
[Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Channel]],
[Out[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SaveBMP.forms = [
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8], In[Rblti::Palette]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadJPEG.forms = [
[Out[Rblti::Image]],
[Out[Rblti::Image], Out["bool"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SaveJPEG.forms = [
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadPNG.forms = [
[Out[Rblti::Image]],
[Out[Rblti::Channel8], Out[Rblti::Palette]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SavePNG.forms = [
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8], In[Rblti::Palette]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadImage.forms = [
[Out[Rblti::Image]],
[Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Channel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SaveImage.forms = [
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8], In[Rblti::Palette]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ChiSquareFunctor.forms = [
[Out["double"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RealFFT.forms = [
[In[Rblti::Fvector], Out[Rblti::Fvector], Out[Rblti::Fvector]],
[In[Rblti::Dvector], Out[Rblti::Dvector], Out[Rblti::Dvector]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
OrientedHLTransform.forms = [
[In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel32]],
[In[Rblti::Channel8], In[Rblti::Channel], Out[Rblti::Channel32]],
[In[Rblti::Image], In[Rblti::Channel], Out[Rblti::Channel32]],
[In[Rblti::Channel32], In[Rblti::Channel], Out[Rblti::Channel32]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GHoughTransform.forms = [
[In[Rblti::Channel], Out["std::list<vector<float > >"]],
[In[Rblti::Channel], In[Rblti::Channel], Out["std::list<vector<float > >"]],
[In["TpointList<int >"], In[Rblti::Channel], Out["std::list<vector<float > >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
OrientationMap.forms = [
[In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MedianFilter.forms = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
KNearestNeighFilter.forms = [
[Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Convolution.forms = [
[Out[Rblti::Channel8]],
[Out[Rblti::Channel]],
[Out[Rblti::Dmatrix]],
[Out[Rblti::Uvector]],
[Out[Rblti::Fvector]],
[Out[Rblti::Dvector]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Dmatrix], Out[Rblti::Dmatrix]],
[In[Rblti::Uvector], Out[Rblti::Uvector]],
[In[Rblti::Fvector], Out[Rblti::Fvector]],
[In[Rblti::Dvector], Out[Rblti::Dvector]],
[In[Rblti::Image], Out[Rblti::Image]],
[Out[Rblti::Image]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GradientFunctor.forms = [
[In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel], Out[Rblti::Channel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ColorContrastGradient.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Float]],
[In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Float]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Float]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ImaskFunctor.forms = [
[In[Rblti::Imatrix], Out[Rblti::Imatrix], In["int (*)(int const &,int const &)"]],
[Out[Rblti::Imatrix], In["int (*)(int const &,int const &)"]],
[In[Rblti::Imatrix], In[Rblti::Imatrix], Out[Rblti::Imatrix], In["int (*)(int const &,int const &,int const &)"]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix], In["int (*)(int const &,int const &,int const &)"]],
[In[Rblti::Imatrix], In[Rblti::Imatrix], Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
DmaskFunctor.forms = [
[In[Rblti::Dmatrix], Out[Rblti::Dmatrix], In["double (*)(double const &,double const &)"]],
[Out[Rblti::Dmatrix], In["double (*)(double const &,double const &)"]],
[In[Rblti::Dmatrix], In[Rblti::Dmatrix], Out[Rblti::Dmatrix], In["double (*)(double const &,double const &,double const &)"]],
[In[Rblti::Dmatrix], Out[Rblti::Dmatrix], In["double (*)(double const &,double const &,double const &)"]],
[In[Rblti::Dmatrix], In[Rblti::Dmatrix], Out[Rblti::Dmatrix]],
[In[Rblti::Dmatrix], Out[Rblti::Dmatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
UmaskFunctor.forms = [
[In[Rblti::Umatrix], Out[Rblti::Umatrix], In["ubyte (*)(unsigned char const &,unsigned char const &)"]],
[Out[Rblti::Umatrix], In["ubyte (*)(unsigned char const &,unsigned char const &)"]],
[In[Rblti::Umatrix], In[Rblti::Umatrix], Out[Rblti::Umatrix], In["ubyte (*)(unsigned char const &,unsigned char const &,unsigned char const &)"]],
[In[Rblti::Umatrix], Out[Rblti::Umatrix], In["ubyte (*)(unsigned char const &,unsigned char const &,unsigned char const &)"]],
[In[Rblti::Umatrix], In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[In[Rblti::Umatrix], Out[Rblti::Umatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ColorQuantization.forms = [
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
KMColorQuantization.forms = [
[In[Rblti::Image], Out[Rblti::Imatrix], Out[Rblti::Palette]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LkmColorQuantization.forms = [
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ComputePalette.forms = [
[In[Rblti::Image], In[Rblti::Channel8], Out[Rblti::Palette]],
[In[Rblti::Image], In[Rblti::Imatrix], Out[Rblti::Palette]],
[In[Rblti::Image], In[Rblti::Channel8], Out[Rblti::Palette], Out["Vector<trgbPixel<float > >"]],
[In[Rblti::Image], In[Rblti::Imatrix], Out[Rblti::Palette], Out["Vector<trgbPixel<float > >"]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector], Out["int"]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Image], In[Rblti::Channel8], Out[Rblti::Palette], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Image], In[Rblti::Imatrix], Out[Rblti::Palette], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out["std::vector<matrix<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out["std::vector<matrix<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Channel], In[Rblti::Imatrix], Out[Rblti::Fvector], Out[Rblti::Fvector], Out[Rblti::Ivector]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["int"], Out["TrgbPixel<float >"], Out[Rblti::Fmatrix], Out["int"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
UsePalette.forms = [
[In[Rblti::Umatrix], Out[Rblti::Image]],
[In[Rblti::Imatrix], Out[Rblti::Image]],
[In[Rblti::Umatrix], In[Rblti::Palette], Out[Rblti::Image]],
[In[Rblti::Imatrix], In[Rblti::Palette], Out[Rblti::Image]],
[In[Rblti::Umatrix], In[Rblti::Fvector], Out[Rblti::Fmatrix]],
[In[Rblti::Imatrix], In[Rblti::Fvector], Out[Rblti::Fmatrix]],
[In[Rblti::Image], Out[Rblti::Umatrix]],
[In[Rblti::Image], Out[Rblti::Imatrix]],
[In[Rblti::Image], In[Rblti::Palette], Out[Rblti::Umatrix]],
[In[Rblti::Image], In[Rblti::Palette], Out[Rblti::Imatrix]],
[In[Rblti::Image], In["kdTree<rgbPixel,int >"], Out[Rblti::Umatrix]],
[In[Rblti::Image], In["kdTree<rgbPixel,int >"], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImage.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageTorgI.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MeanShiftSegmentation.forms = [
[In[Rblti::Image], Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Imatrix]],
[In[Rblti::Image], Out[Rblti::Imatrix], Out[Rblti::Palette]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Imatrix]],
[In[Rblti::Image], Out[Rblti::Image], Out[Rblti::Image], Out[Rblti::Imatrix], Out[Rblti::Palette]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
KMeansSegmentation.forms = [
[In[Rblti::Image], Out[Rblti::Imatrix]],
[In[Rblti::Image], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Imatrix], Out[Rblti::Palette]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
WhiteningSegmentation.forms = [
[In[Rblti::Image], In["principalComponents<float >"], Out[Rblti::Imatrix]],
[In[Rblti::Image], In["DrgbPixel"], In[Rblti::Dmatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CsPresegmentation.forms = [
[In[Rblti::Image], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Thresholding.forms = [
[Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Float], Out[Float], Out["bool"], Out["bool"]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Float], Out[Float], Out["bool"]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Float], Out[Float]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Float]],
[Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Float], Out[Float], Out["bool"], Out["bool"], Out[Float], Out[Float]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Float], Out[Float], Out["bool"], Out["bool"], Out[Float]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Float], Out[Float], Out["bool"], Out["bool"]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Float], Out[Float], Out["bool"]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Float], Out[Float]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Float]],
[In[Rblti::Channel8], Out[Rblti::AreaPoints]],
[In[Rblti::Channel], Out[Rblti::AreaPoints]],
[In[Rblti::Channel8], In[Rblti::AreaPoints], Out[Rblti::AreaPoints]],
[In[Rblti::Channel], In[Rblti::AreaPoints], Out[Rblti::AreaPoints]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
WatershedSegmentation.forms = [
[Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RegionGrowing.forms = [
[Out[Rblti::Channel8]],
[Out[Rblti::Channel]],
[Out[Rblti::Image]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Snake.forms = [
[In[Rblti::Image], Out[Rblti::AreaPoints]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RegionMerge.forms = [
[In[Rblti::Imatrix], In[Rblti::Dmatrix], Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], In[Rblti::Dmatrix], In[Rblti::Dvector], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SimilarityMatrix.forms = [
[In[Rblti::Image], Out["int"], In[Rblti::Imatrix], Out[Rblti::Dmatrix], Out[Rblti::Dmatrix]],
[In[Rblti::Image], In["std::list<areaPoints >"], Out[Rblti::Dmatrix], Out[Rblti::Dmatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
FastRelabeling.forms = [
[Out[Rblti::Channel8]],
[Out[Rblti::Imatrix]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Ivector]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out[Rblti::Ivector]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix], Out[Rblti::Ivector]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Ivector], Out["std::vector<areaPoints >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out["std::vector<areaPoints >"]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out["std::vector<areaPoints >"]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Ivector], Out["int"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out["int"]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out["int"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ObjectsFromMask.forms = [
[In[Rblti::Channel8], Out["std::list<ioPoints >"]],
[In[Rblti::Imatrix], Out["std::list<ioPoints >"]],
[In[Rblti::Channel8], Out["std::list<borderPoints >"]],
[In[Rblti::Imatrix], Out["std::list<borderPoints >"]],
[In[Rblti::Channel8], Out["std::list<areaPoints >"]],
[In[Rblti::Imatrix], Out["std::list<areaPoints >"]],
[In[Rblti::Channel8], Out["std::list<areaPoints >"], Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out["std::list<areaPoints >"], Out[Rblti::Imatrix]],
[In[Rblti::Channel8], Out["tree<objectStruct >"]],
[In[Rblti::Imatrix], Out["tree<objectStruct >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
BackgroundModel.forms = [
[In[Rblti::Image], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MeanshiftTracker.forms = [
[In[Rblti::Image], InOut[Rblti::Rect]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CornerDetector.forms = [
[Out[Rblti::Channel8]],
[Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::PointList]],
[In[Rblti::Channel], Out[Rblti::PointList]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
EdgeDetector.forms = [
[Out[Rblti::Channel8]],
[Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel8]],
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ClassicEdgeDetector.forms = [
[In[Rblti::Channel], Out[Rblti::Channel8]],
[Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CannyEdges.forms = [
[Out[Rblti::Channel8]],
[Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel8], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel]],
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel8], Out[Rblti::Channel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GlobalFeatureExtractor.forms = [
[In[Rblti::Channel], Out[Rblti::Dvector]],
[In[Rblti::Channel8], Out[Rblti::Dvector]],
[In[Rblti::Image], Out[Rblti::Dvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LocalFeatureExtractor.forms = [
[In[Rblti::Channel], In["std::list<location >"], Out["std::list<dvector >"]],
[In[Rblti::Channel8], In["std::list<location >"], Out["std::list<dvector >"]],
[In["std::list<channel >"], In["std::list<location >"], Out["std::list<dvector >"]],
[In[Rblti::Image], In["std::list<location >"], Out["std::list<dvector >"]],
[In[Rblti::Image], In[Rblti::Location], Out[Rblti::Dvector]],
[In[Rblti::Channel], In[Rblti::Location], Out[Rblti::Dvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LocalMoments.forms = [
[In[Rblti::Channel], In[Rblti::Location], Out[Rblti::Dvector]],
[In[Rblti::Image], In[Rblti::Location], Out[Rblti::Dvector]],
[In[Rblti::Image], In["std::list<location >"], Out["std::list<dvector >"]],
[In[Rblti::Channel], In["std::list<location >"], Out["std::list<dvector >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GeometricFeatures.forms = [
[In[Rblti::AreaPoints], Out[Rblti::Dvector]],
[In[Rblti::BorderPoints], Out[Rblti::Dvector]],
[In[Rblti::IoPoints], Out[Rblti::Dvector]],
[In[Rblti::AreaPoints], Out["std::map<std::string,double >"]],
[In[Rblti::BorderPoints], Out["std::map<std::string,double >"]],
[In[Rblti::IoPoints], Out["std::map<std::string,double >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ChromaticityHistogram.forms = [
[In[Rblti::Image], Out[Rblti::Dvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GeometricFeaturesFromMask.forms = [
[In[Rblti::Channel8], Out["std::vector<rectangle >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<rectangle >"]],
[In[Rblti::Channel8], Out["std::vector<geometricFeatureGroup0 >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<geometricFeatureGroup0 >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MultiGeometricFeaturesFromMask.forms = [
[In[Rblti::Channel8], Out["std::vector<std::vector<rectangle > >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<std::vector<rectangle > >"]],
[In[Rblti::Channel8], Out["std::vector<std::vector<geometricFeatureGroup0 > >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<std::vector<geometricFeatureGroup0 > >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Morphology.forms = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Erosion.forms = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[Out[Rblti::Fvector]],
[Out[Rblti::Uvector]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Fvector], Out[Rblti::Fvector]],
[In[Rblti::Uvector], Out[Rblti::Uvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Dilation.forms = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[Out[Rblti::Fvector]],
[Out[Rblti::Uvector]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Fvector], Out[Rblti::Fvector]],
[In[Rblti::Uvector], Out[Rblti::Uvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Skeleton.forms = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Scaling.forms = [
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
[Out[Rblti::Umatrix]],
[In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix]],
[In[Float], Out[Rblti::Image]],
[In[Float], In[Rblti::Image], Out[Rblti::Image]],
[In[Float], Out[Rblti::Umatrix]],
[In[Float], In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[In[Float], Out[Rblti::Fmatrix]],
[In[Float], In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[In[Float], Out[Rblti::Imatrix]],
[In[Float], In[Rblti::Imatrix], Out[Rblti::Imatrix]],
[In["Tpoint<float >"], Out[Rblti::Image]],
[In["Tpoint<float >"], In[Rblti::Image], Out[Rblti::Image]],
[In["Tpoint<float >"], Out[Rblti::Umatrix]],
[In["Tpoint<float >"], In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[In["Tpoint<float >"], Out[Rblti::Fmatrix]],
[In["Tpoint<float >"], In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[In["Tpoint<float >"], Out[Rblti::Imatrix]],
[In["Tpoint<float >"], In[Rblti::Imatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Rotation.forms = [
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
[Out[Rblti::Umatrix]],
[In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out["double"], Out[Rblti::Image]],
[Out["double"], In[Rblti::Image], Out[Rblti::Image]],
[Out["double"], Out[Rblti::Umatrix]],
[Out["double"], In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[Out["double"], Out[Rblti::Fmatrix]],
[Out["double"], In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
FlipImage.forms = [
[Out["Matrix<rgbPixel >"]],
[In["Matrix<rgbPixel >"], Out["Matrix<rgbPixel >"]],
[Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out[Rblti::Umatrix]],
[In[Rblti::Umatrix], Out[Rblti::Umatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
DistanceTransform.forms = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
BorderExtrema.forms = [
[In[Rblti::BorderPoints], Out[Rblti::PolygonPoints], Out[Rblti::PolygonPoints]],
[In[Rblti::BorderPoints], Out[Rblti::PolygonPoints]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
PolygonApproximation.forms = [
[In[Rblti::BorderPoints], Out[Rblti::PolygonPoints]],
[In[Rblti::BorderPoints], In[Rblti::PointList], Out[Rblti::PolygonPoints]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ConvexHull.forms = [
[In[Rblti::PointList], Out[Rblti::PolygonPoints]],
[In[Rblti::IoPoints], Out[Rblti::PolygonPoints]],
[In["TpointList<float >"], Out["TpolygonPoints<float >"]],
[In["TpointList<double >"], Out["TpolygonPoints<double >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSI.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
[In[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToRGB.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSV.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHLS.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToXYZ.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToxyY.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToOCP.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYIQ.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToCIELuv.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYUV.forms = [
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

# 172 Rblti::Channel
# 170 Rblti::Channel8
# 131 Rblti::Image
#  84 Rblti::Imatrix
#  68 Float
#  37 Rblti::Umatrix
#  33 Ubyte
#  28 Rblti::Palette
#  24 Rblti::RgbPixel
#  24 Rblti::Fmatrix
#  23 Rblti::Dmatrix
#  20 Rblti::Ivector
#  19 Rblti::Fvector
#  18 Rblti::Dvector
#  12 Rblti::Uvector
#  11 bool
#   9 Rblti::AreaPoints
#   9 Vector<trgbPixel<float > >
#   8 Tpoint<float >
#   7 Rblti::PolygonPoints
#   7 int
#   7 double
#   6 std::list<location >
#   6 std::list<dvector >
#   6 Rblti::BorderPoints
#   5 Rblti::Channel32
#   5 std::list<areaPoints >
#   4 Rblti::Location
#   4 Rblti::PointList
#   3 std::map<std::string,double >
#   3 std::list<vector<float > >
#   3 Rblti::IoPoints
#   3 std::vector<areaPoints >
#   3 Matrix<rgbPixel >
#   2 std::vector<rectangle >
#   2 int (*)(int const &,int const &)
#   2 std::list<ioPoints >
#   2 std::vector<std::vector<rectangle > >
#   2 int (*)(int const &,int const &,int const &)
#   2 std::vector<geometricFeatureGroup0 >
#   2 kdTree<rgbPixel,int >
#   2 std::vector<std::vector<geometricFeatureGroup0 > >
#   2 ubyte (*)(unsigned char const &,unsigned char const &,unsigned char const &)
#   2 ubyte (*)(unsigned char const &,unsigned char const &)
#   2 std::vector<matrix<float > >
#   2 double (*)(double const &,double const &,double const &)
#   2 std::list<borderPoints >
#   2 tree<objectStruct >
#   2 double (*)(double const &,double const &)
#   1 TpointList<int >
#   1 TpolygonPoints<double >
#   1 TrgbPixel<float >
#   1 TpolygonPoints<float >
#   1 std::list<channel >
#   1 TpointList<float >
#   1 DrgbPixel
#   1 principalComponents<float >
#   1 Rblti::Rect
#   1 TpointList<double >
