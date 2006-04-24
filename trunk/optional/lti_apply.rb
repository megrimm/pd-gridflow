begin
Modifier.form = [
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
IoFunctor.form = [
[Out[Rblti::Image]],
[Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8], In[Rblti::Palette]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadBMP.form = [
[Out[Rblti::Image]],
[Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Channel]],
[Out[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SaveBMP.form = [
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8], In[Rblti::Palette]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadJPEG.form = [
[Out[Rblti::Image]],
[Out[Rblti::Image], Out["bool"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SaveJPEG.form = [
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadPNG.form = [
[Out[Rblti::Image]],
[Out[Rblti::Channel8], Out[Rblti::Palette]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SavePNG.form = [
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8], In[Rblti::Palette]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LoadImage.form = [
[Out[Rblti::Image]],
[Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Channel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SaveImage.form = [
[In[Rblti::Image]],
[In[Rblti::Channel]],
[In[Rblti::Channel8], In[Rblti::Palette]],
[In[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ChiSquareFunctor.form = [
[Out["double"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RealFFT.form = [
[In[Rblti::Fvector], Out[Rblti::Fvector], Out[Rblti::Fvector]],
[In[Rblti::Dvector], Out[Rblti::Dvector], Out[Rblti::Dvector]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
OrientedHLTransform.form = [
[In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel32]],
[In[Rblti::Channel8], In[Rblti::Channel], Out[Rblti::Channel32]],
[In[Rblti::Image], In[Rblti::Channel], Out[Rblti::Channel32]],
[In[Rblti::Channel32], In[Rblti::Channel], Out[Rblti::Channel32]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GHoughTransform.form = [
[In[Rblti::Channel], Out["std::list<vector<float > >"]],
[In[Rblti::Channel], In[Rblti::Channel], Out["std::list<vector<float > >"]],
[In["TpointList<int >"], In[Rblti::Channel], Out["std::list<vector<float > >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
OrientationMap.form = [
[In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MedianFilter.form = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
KNearestNeighFilter.form = [
[Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Convolution.form = [
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
GradientFunctor.form = [
[In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel], Out[Rblti::Channel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ColorContrastGradient.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Float]],
[In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Float]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Float]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ImaskFunctor.form = [
[In[Rblti::Imatrix], Out[Rblti::Imatrix], In["int (*)(int const &,int const &)"]],
[Out[Rblti::Imatrix], In["int (*)(int const &,int const &)"]],
[In[Rblti::Imatrix], In[Rblti::Imatrix], Out[Rblti::Imatrix], In["int (*)(int const &,int const &,int const &)"]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix], In["int (*)(int const &,int const &,int const &)"]],
[In[Rblti::Imatrix], In[Rblti::Imatrix], Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ColorQuantization.form = [
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
KMColorQuantization.form = [
[In[Rblti::Image], Out[Rblti::Imatrix], Out[Rblti::Palette]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LkmColorQuantization.form = [
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ComputePalette.form = [
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
UsePalette.form = [
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
SplitImage.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageTorgI.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MeanShiftSegmentation.form = [
[In[Rblti::Image], Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Imatrix]],
[In[Rblti::Image], Out[Rblti::Imatrix], Out[Rblti::Palette]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Imatrix]],
[In[Rblti::Image], Out[Rblti::Image], Out[Rblti::Image], Out[Rblti::Imatrix], Out[Rblti::Palette]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
KMeansSegmentation.form = [
[In[Rblti::Image], Out[Rblti::Imatrix]],
[In[Rblti::Image], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Imatrix], Out[Rblti::Palette]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Palette]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
WhiteningSegmentation.form = [
[In[Rblti::Image], In["principalComponents<float >"], Out[Rblti::Imatrix]],
[In[Rblti::Image], In["DrgbPixel"], In[Rblti::Dmatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CsPresegmentation.form = [
[In[Rblti::Image], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Thresholding.form = [
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
WatershedSegmentation.form = [
[Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel8], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RegionGrowing.form = [
[Out[Rblti::Channel8]],
[Out[Rblti::Channel]],
[Out[Rblti::Image]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Snake.form = [
[In[Rblti::Image], Out[Rblti::AreaPoints]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
RegionMerge.form = [
[In[Rblti::Imatrix], In[Rblti::Dmatrix], Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], In[Rblti::Dmatrix], In[Rblti::Dvector], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SimilarityMatrix.form = [
[In[Rblti::Image], Out["int"], In[Rblti::Imatrix], Out[Rblti::Dmatrix], Out[Rblti::Dmatrix]],
[In[Rblti::Image], In["std::list<areaPoints >"], Out[Rblti::Dmatrix], Out[Rblti::Dmatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
FastRelabeling.form = [
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
ObjectsFromMask.form = [
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
BackgroundModel.form = [
[In[Rblti::Image], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MeanshiftTracker.form = [
[In[Rblti::Image], Out["Trectangle<int >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CornerDetector.form = [
[Out[Rblti::Channel8]],
[Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::PointList]],
[In[Rblti::Channel], Out[Rblti::PointList]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
EdgeDetector.form = [
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
ClassicEdgeDetector.form = [
[In[Rblti::Channel], Out[Rblti::Channel8]],
[Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CannyEdges.form = [
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
GlobalFeatureExtractor.form = [
[In[Rblti::Channel], Out[Rblti::Dvector]],
[In[Rblti::Channel8], Out[Rblti::Dvector]],
[In[Rblti::Image], Out[Rblti::Dvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LocalFeatureExtractor.form = [
[In[Rblti::Channel], In["std::list<location >"], Out["std::list<dvector >"]],
[In[Rblti::Channel8], In["std::list<location >"], Out["std::list<dvector >"]],
[In["std::list<channel >"], In["std::list<location >"], Out["std::list<dvector >"]],
[In[Rblti::Image], In["std::list<location >"], Out["std::list<dvector >"]],
[In[Rblti::Image], In[Rblti::Location], Out[Rblti::Dvector]],
[In[Rblti::Channel], In[Rblti::Location], Out[Rblti::Dvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LocalMoments.form = [
[In[Rblti::Channel], In[Rblti::Location], Out[Rblti::Dvector]],
[In[Rblti::Image], In[Rblti::Location], Out[Rblti::Dvector]],
[In[Rblti::Image], In["std::list<location >"], Out["std::list<dvector >"]],
[In[Rblti::Channel], In["std::list<location >"], Out["std::list<dvector >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GeometricFeatures.form = [
[In[Rblti::AreaPoints], Out[Rblti::Dvector]],
[In[Rblti::BorderPoints], Out[Rblti::Dvector]],
[In[Rblti::IoPoints], Out[Rblti::Dvector]],
[In[Rblti::AreaPoints], Out["std::map<std::string,double >"]],
[In[Rblti::BorderPoints], Out["std::map<std::string,double >"]],
[In[Rblti::IoPoints], Out["std::map<std::string,double >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ChromaticityHistogram.form = [
[In[Rblti::Image], Out[Rblti::Dvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
GeometricFeaturesFromMask.form = [
[In[Rblti::Channel8], Out["std::vector<rectangle >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<rectangle >"]],
[In[Rblti::Channel8], Out["std::vector<geometricFeatureGroup0 >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<geometricFeatureGroup0 >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MultiGeometricFeaturesFromMask.form = [
[In[Rblti::Channel8], Out["std::vector<std::vector<rectangle > >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<std::vector<rectangle > >"]],
[In[Rblti::Channel8], Out["std::vector<std::vector<geometricFeatureGroup0 > >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<std::vector<geometricFeatureGroup0 > >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Morphology.form = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Erosion.form = [
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
Dilation.form = [
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
Skeleton.form = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Scaling.form = [
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
[Out[Rblti::Umatrix]],
[In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix]],
[Out[Float], Out[Rblti::Image]],
[Out[Float], In[Rblti::Image], Out[Rblti::Image]],
[Out[Float], Out[Rblti::Umatrix]],
[Out[Float], In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[Out[Float], Out[Rblti::Fmatrix]],
[Out[Float], In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out[Float], Out[Rblti::Imatrix]],
[Out[Float], In[Rblti::Imatrix], Out[Rblti::Imatrix]],
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
Rotation.form = [
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
FlipImage.form = [
[Out["Matrix<rgbPixel >"]],
[In["Matrix<rgbPixel >"], Out["Matrix<rgbPixel >"]],
[Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out[Rblti::Umatrix]],
[In[Rblti::Umatrix], Out[Rblti::Umatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
DistanceTransform.form = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
BorderExtrema.form = [
[In[Rblti::BorderPoints], Out[Rblti::PolygonPoints], Out[Rblti::PolygonPoints]],
[In[Rblti::BorderPoints], Out[Rblti::PolygonPoints]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
PolygonApproximation.form = [
[In[Rblti::BorderPoints], Out[Rblti::PolygonPoints]],
[In[Rblti::BorderPoints], In[Rblti::PointList], Out[Rblti::PolygonPoints]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ConvexHull.form = [
[In[Rblti::PointList], Out[Rblti::PolygonPoints]],
[In[Rblti::IoPoints], Out[Rblti::PolygonPoints]],
[In["TpointList<float >"], Out["TpolygonPoints<float >"]],
[In["TpointList<double >"], Out["TpolygonPoints<double >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSI.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
[In[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToRGB.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSV.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHLS.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToXYZ.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToxyY.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToOCP.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYIQ.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToCIELuv.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::RgbPixel], Out[Float], Out[Float], Out[Float]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYUV.form = [
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
#  33 Ubyte
#  28 Rblti::Palette
#  24 Rblti::Fmatrix
#  24 Rblti::Umatrix
#  24 Rblti::RgbPixel
#  20 Rblti::Ivector
#  19 Rblti::Fvector
#  18 Rblti::Dvector
#  12 Rblti::Uvector
#  11 bool
#  10 Rblti::Dmatrix
#   9 Vector<trgbPixel<float > >
#   9 Rblti::AreaPoints
#   8 Tpoint<float >
#   7 double
#   7 int
#   7 Rblti::PolygonPoints
#   6 Rblti::BorderPoints
#   6 std::list<dvector >
#   6 std::list<location >
#   5 std::list<areaPoints >
#   5 Rblti::Channel32
#   4 Rblti::Location
#   4 Rblti::PointList
#   3 Rblti::IoPoints
#   3 std::list<vector<float > >
#   3 std::vector<areaPoints >
#   3 std::map<std::string,double >
#   3 Matrix<rgbPixel >
#   2 std::vector<std::vector<rectangle > >
#   2 int (*)(int const &,int const &,int const &)
#   2 std::vector<matrix<float > >
#   2 std::vector<std::vector<geometricFeatureGroup0 > >
#   2 std::vector<geometricFeatureGroup0 >
#   2 int (*)(int const &,int const &)
#   2 kdTree<rgbPixel,int >
#   2 tree<objectStruct >
#   2 std::list<borderPoints >
#   2 std::list<ioPoints >
#   2 std::vector<rectangle >
#   1 TpointList<float >
#   1 TpolygonPoints<double >
#   1 TpolygonPoints<float >
#   1 Trectangle<int >
#   1 TpointList<int >
#   1 std::list<channel >
#   1 TrgbPixel<float >
#   1 DrgbPixel
#   1 TpointList<double >
#   1 principalComponents<float >
