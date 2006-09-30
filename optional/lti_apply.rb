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
[Out[Rblti::Double]],
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
[In[Rblti::PointList], In[Rblti::Channel], Out["std::list<vector<float > >"]],
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
[Out[Rblti::Umatrix]],
[Out[Rblti::Fmatrix]],
[Out[Rblti::Dmatrix]],
[Out[Rblti::Uvector]],
[Out[Rblti::Fvector]],
[Out[Rblti::Dvector]],
[In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
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
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Float]],
[In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Float]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Float]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MaximumFilter.forms = [
[Out[Rblti::Fmatrix]],
[Out[Rblti::Fvector]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[In[Rblti::Fvector], Out[Rblti::Fvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MatrixInversion.forms = [
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out[Rblti::Fmatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
CholeskyDecomposition.forms = [
[Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out[Rblti::Fmatrix], In["CholeskyDecomposition_parameters::eTriangularType"]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix], In["CholeskyDecomposition_parameters::eTriangularType"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
EigenSystem.forms = [
[In[Rblti::Fmatrix], Out[Rblti::Fvector], Out[Rblti::Fmatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Jacobi.forms = [
[In[Rblti::Fmatrix], Out[Rblti::Fvector], Out[Rblti::Fmatrix]],
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
[In[Rblti::Image], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector], In[Rblti::Integer]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Image], In[Rblti::Channel8], Out[Rblti::Palette], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Image], In[Rblti::Imatrix], Out[Rblti::Palette], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out["std::vector<matrix<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out["std::vector<matrix<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Imatrix], Out["Vector<trgbPixel<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Channel], In[Rblti::Imatrix], Out[Rblti::Fvector], Out[Rblti::Fvector], Out[Rblti::Ivector]],
[In[Rblti::Image], In[Rblti::Imatrix], In[Rblti::Integer], Out["TrgbPixel<float >"], Out[Rblti::Fmatrix], Out[Rblti::Integer]],
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
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageTorgI.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergeImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
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
[In[Rblti::Channel8], Out[Rblti::Channel8], In[Rblti::Float], In[Rblti::Float], In["bool"], In["bool"]],
[In[Rblti::Channel8], Out[Rblti::Channel8], In[Rblti::Float], In[Rblti::Float], In["bool"]],
[In[Rblti::Channel8], Out[Rblti::Channel8], In[Rblti::Float], In[Rblti::Float]],
[In[Rblti::Channel8], Out[Rblti::Channel8], In[Rblti::Float]],
[Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel], In[Rblti::Float], In[Rblti::Float], In["bool"], In["bool"], In[Rblti::Float], In[Rblti::Float]],
[In[Rblti::Channel], Out[Rblti::Channel], In[Rblti::Float], In[Rblti::Float], In["bool"], In["bool"], In[Rblti::Float]],
[In[Rblti::Channel], Out[Rblti::Channel], In[Rblti::Float], In[Rblti::Float], In["bool"], In["bool"]],
[In[Rblti::Channel], Out[Rblti::Channel], In[Rblti::Float], In[Rblti::Float], In["bool"]],
[In[Rblti::Channel], Out[Rblti::Channel], In[Rblti::Float], In[Rblti::Float]],
[In[Rblti::Channel], Out[Rblti::Channel], In[Rblti::Float]],
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
[In[Rblti::Image], In[Rblti::Integer], In[Rblti::Imatrix], Out[Rblti::Dmatrix], Out[Rblti::Dmatrix]],
[In[Rblti::Image], In[Rblti::List_areaPoints], Out[Rblti::Dmatrix], Out[Rblti::Dmatrix], Out[Rblti::Imatrix]],
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
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Ivector], Out[Rblti::Integer]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out[Rblti::Integer]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out[Rblti::Integer]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ObjectsFromMask.forms = [
[In[Rblti::Channel8], Out[Rblti::List_ioPoints]],
[In[Rblti::Imatrix], Out[Rblti::List_ioPoints]],
[In[Rblti::Channel8], Out[Rblti::List_borderPoints]],
[In[Rblti::Imatrix], Out[Rblti::List_borderPoints]],
[In[Rblti::Channel8], Out[Rblti::List_areaPoints]],
[In[Rblti::Imatrix], Out[Rblti::List_areaPoints]],
[In[Rblti::Channel8], Out[Rblti::List_areaPoints], Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out[Rblti::List_areaPoints], Out[Rblti::Imatrix]],
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
[In[Rblti::Image], InOut[Rblti::Irect]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MaskImage.forms = [
[In[Rblti::Image], In[Rblti::Channel8], In[Rblti::Palette], Out[Rblti::Image]],
[In[Rblti::Image], In[Rblti::Imatrix], In[Rblti::Palette], Out[Rblti::Image]],
[In[Rblti::Image], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Image], In[Rblti::Imatrix], Out[Rblti::Image]],
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
HarrisCorners.forms = [
[Out[Rblti::Channel8]],
[Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::PointList]],
[In[Rblti::Channel], Out[Rblti::PointList]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Float], Out[Rblti::PointList]],
[In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Float]],
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
LocalMaxima.forms = [
[Out[Rblti::Fmatrix]],
[Out[Rblti::Fvector]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix], Out[Rblti::PointList]],
[In[Rblti::Fmatrix], Out[Rblti::PointList]],
[In[Rblti::Fvector], Out[Rblti::Fvector]],
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
[In[Rblti::Float], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Image], Out[Rblti::Image]],
[In[Rblti::Float], Out[Rblti::Umatrix]],
[In[Rblti::Float], In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[In[Rblti::Float], Out[Rblti::Fmatrix]],
[In[Rblti::Float], In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[In[Rblti::Float], Out[Rblti::Imatrix]],
[In[Rblti::Float], In[Rblti::Imatrix], Out[Rblti::Imatrix]],
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
[In[Rblti::Double], Out[Rblti::Image]],
[In[Rblti::Double], In[Rblti::Image], Out[Rblti::Image]],
[In[Rblti::Double], Out[Rblti::Umatrix]],
[In[Rblti::Double], In[Rblti::Umatrix], Out[Rblti::Umatrix]],
[In[Rblti::Double], Out[Rblti::Fmatrix]],
[In[Rblti::Double], In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
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
RegionsPolygonizer.forms = [
[In[Rblti::Imatrix], Out[Rblti::Vector_polygonPoints]],
[In[Rblti::Imatrix], Out[Rblti::Vector_polygonPoints], Out[Rblti::Vector_borderPoints], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSI.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
[In[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToRGB.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSV.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHLS.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToXYZ.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToxyY.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToOCP.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYIQ.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToCIELuv.forms = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYUV.forms = [
[In[Rblti::RgbPixel], Out[Rblti::Float], Out[Rblti::Float], Out[Rblti::Float]],
[In[Rblti::RgbPixel], Out[Rblti::Ubyte], Out[Rblti::Ubyte], Out[Rblti::Ubyte]],
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergeRGBToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergeHSVToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergeHLSToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergergIToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergeXYZToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergexyYToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergeOCPToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergeYIQToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergeCIELuvToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MergeYUVToImage.forms = [
[In[Rblti::Fmatrix], In[Rblti::Fmatrix], In[Rblti::Fmatrix], Out[Rblti::Image]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
[In[Rblti::Float], In[Rblti::Float], In[Rblti::Float], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
[In[Rblti::Ubyte], In[Rblti::Ubyte], In[Rblti::Ubyte], Out[Rblti::RgbPixel]],
[In[Rblti::Channel8], In[Rblti::Channel8], In[Rblti::Channel8], Out[Rblti::Image]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

