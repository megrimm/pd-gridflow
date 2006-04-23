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
[Out[Rblti::Fvector]],
[Out[Rblti::Ivector]],
[Out["Vector<lti::ubyte >"]],
[Out[Rblti::Channel]],
[Out[Rblti::Imatrix]],
[Out[Rblti::Channel8]],
[Out[Rblti::Image]],
[In[Rblti::Fvector], Out[Rblti::Fvector]],
[In[Rblti::Ivector], Out[Rblti::Ivector]],
[In["Vector<lti::ubyte >"], Out["Vector<lti::ubyte >"]],
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
[In["Vector<float >"], Out["Vector<float >"], Out["Vector<float >"]],
[In["Vector<double >"], Out["Vector<double >"], Out["Vector<double >"]],
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
[In[Rblti::Channel], Out["std::list<lti::vector<float > >"]],
[In[Rblti::Channel], In[Rblti::Channel], Out["std::list<lti::vector<float > >"]],
[In["TpointList<int >"], In[Rblti::Channel], Out["std::list<lti::vector<float > >"]],
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
[Out["Vector<lti::channel8::value_type >"]],
[Out["Vector<lti::channel::value_type >"]],
[Out[Rblti::Dvector]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Dmatrix], Out[Rblti::Dmatrix]],
[In["Vector<lti::channel8::value_type >"], Out["Vector<lti::channel8::value_type >"]],
[In["Vector<lti::channel::value_type >"], Out["Vector<lti::channel::value_type >"]],
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
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out["float"]],
[In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out["float"]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out["float"]],
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
[In[Rblti::Image], In[Rblti::Channel8], Out[Rblti::Palette], Out["Vector<lti::trgbPixel<float > >"]],
[In[Rblti::Image], In[Rblti::Imatrix], Out[Rblti::Palette], Out["Vector<lti::trgbPixel<float > >"]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["Vector<lti::trgbPixel<float > >"], Out[Rblti::Ivector], Out["int"]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["Vector<lti::trgbPixel<float > >"], Out[Rblti::Ivector]],
[In[Rblti::Image], In[Rblti::Channel8], Out[Rblti::Palette], Out["Vector<lti::trgbPixel<float > >"], Out["Vector<int >"]],
[In[Rblti::Image], In[Rblti::Imatrix], Out[Rblti::Palette], Out["Vector<lti::trgbPixel<float > >"], Out["Vector<int >"]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["Vector<lti::trgbPixel<float > >"], Out["std::vector<lti::matrix<float > >"], Out["Vector<int >"]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Imatrix], Out["Vector<lti::trgbPixel<float > >"], Out["std::vector<lti::matrix<float > >"], Out["Vector<int >"]],
[In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Channel], In[Rblti::Imatrix], Out["Vector<lti::trgbPixel<float > >"], Out["Vector<int >"]],
[In[Rblti::Channel], In[Rblti::Imatrix], Out["Vector<float >"], Out["Vector<float >"], Out["Vector<int >"]],
[In[Rblti::Image], In[Rblti::Imatrix], Out["int"], Out["TrgbPixel<float >"], Out[Rblti::Fmatrix], Out["int"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
UsePalette.form = [
[In["Matrix<lti::ubyte >"], Out[Rblti::Image]],
[In[Rblti::Imatrix], Out[Rblti::Image]],
[In["Matrix<lti::ubyte >"], In[Rblti::Palette], Out[Rblti::Image]],
[In[Rblti::Imatrix], In[Rblti::Palette], Out[Rblti::Image]],
[In["Matrix<lti::ubyte >"], In["Vector<float >"], Out[Rblti::Fmatrix]],
[In[Rblti::Imatrix], In["Vector<float >"], Out[Rblti::Fmatrix]],
[In[Rblti::Image], Out["Matrix<lti::ubyte >"]],
[In[Rblti::Image], Out[Rblti::Imatrix]],
[In[Rblti::Image], In[Rblti::Palette], Out["Matrix<lti::ubyte >"]],
[In[Rblti::Image], In[Rblti::Palette], Out[Rblti::Imatrix]],
[In[Rblti::Image], In["kdTree<lti::rgbPixel,int >"], Out["Matrix<lti::ubyte >"]],
[In[Rblti::Image], In["kdTree<lti::rgbPixel,int >"], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImage.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageTorgI.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
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
[In[Rblti::Channel8], Out[Rblti::Channel8], Out["float"], Out["float"], Out["bool"], Out["bool"]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out["float"], Out["float"], Out["bool"]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out["float"], Out["float"]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out["float"]],
[Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel8]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel], Out[Rblti::Channel], Out["float"], Out["float"], Out["bool"], Out["bool"], Out["float"], Out["float"]],
[In[Rblti::Channel], Out[Rblti::Channel], Out["float"], Out["float"], Out["bool"], Out["bool"], Out["float"]],
[In[Rblti::Channel], Out[Rblti::Channel], Out["float"], Out["float"], Out["bool"], Out["bool"]],
[In[Rblti::Channel], Out[Rblti::Channel], Out["float"], Out["float"], Out["bool"]],
[In[Rblti::Channel], Out[Rblti::Channel], Out["float"], Out["float"]],
[In[Rblti::Channel], Out[Rblti::Channel], Out["float"]],
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
[In[Rblti::Image], In["std::list<lti::areaPoints >"], Out[Rblti::Dmatrix], Out[Rblti::Dmatrix], Out[Rblti::Imatrix]],
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
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Ivector], Out["std::vector<lti::areaPoints >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out["std::vector<lti::areaPoints >"]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out["std::vector<lti::areaPoints >"]],
[In[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Ivector], Out["int"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out["int"]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix], Out[Rblti::Ivector], Out["int"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
ObjectsFromMask.form = [
[In[Rblti::Channel8], Out["std::list<lti::ioPoints >"]],
[In[Rblti::Imatrix], Out["std::list<lti::ioPoints >"]],
[In[Rblti::Channel8], Out["std::list<lti::borderPoints >"]],
[In[Rblti::Imatrix], Out["std::list<lti::borderPoints >"]],
[In[Rblti::Channel8], Out["std::list<lti::areaPoints >"]],
[In[Rblti::Imatrix], Out["std::list<lti::areaPoints >"]],
[In[Rblti::Channel8], Out["std::list<lti::areaPoints >"], Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out["std::list<lti::areaPoints >"], Out[Rblti::Imatrix]],
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
[In[Rblti::Channel], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In[Rblti::Channel8], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In["std::list<lti::channel >"], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In[Rblti::Image], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In[Rblti::Image], In[Rblti::Location], Out[Rblti::Dvector]],
[In[Rblti::Channel], In[Rblti::Location], Out[Rblti::Dvector]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
LocalMoments.form = [
[In[Rblti::Channel], In[Rblti::Location], Out[Rblti::Dvector]],
[In[Rblti::Image], In[Rblti::Location], Out[Rblti::Dvector]],
[In[Rblti::Image], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
[In[Rblti::Channel], In["std::list<lti::location >"], Out["std::list<lti::dvector >"]],
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
[In[Rblti::Channel8], Out["std::vector<lti::rectangle >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<lti::rectangle >"]],
[In[Rblti::Channel8], Out["std::vector<geometricFeatureGroup0 >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<geometricFeatureGroup0 >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
MultiGeometricFeaturesFromMask.form = [
[In[Rblti::Channel8], Out["std::vector<std::vector<lti::rectangle > >"]],
[In[Rblti::Channel8], Out[Rblti::Imatrix], Out["std::vector<std::vector<lti::rectangle > >"]],
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
[Out["Vector<lti::channel8::value_type >"]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Fvector], Out[Rblti::Fvector]],
[In["Vector<lti::channel8::value_type >"], Out["Vector<lti::channel8::value_type >"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Dilation.form = [
[Out[Rblti::Channel]],
[Out[Rblti::Channel8]],
[Out[Rblti::Fvector]],
[Out["Vector<lti::channel8::value_type >"]],
[In[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Fvector], Out[Rblti::Fvector]],
[In["Vector<lti::channel8::value_type >"], Out["Vector<lti::channel8::value_type >"]],
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
[Out["Matrix<lti::ubyte >"]],
[In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out[Rblti::Imatrix]],
[In[Rblti::Imatrix], Out[Rblti::Imatrix]],
[Out["float"], Out[Rblti::Image]],
[Out["float"], In[Rblti::Image], Out[Rblti::Image]],
[Out["float"], Out["Matrix<lti::ubyte >"]],
[Out["float"], In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[Out["float"], Out[Rblti::Fmatrix]],
[Out["float"], In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out["float"], Out[Rblti::Imatrix]],
[Out["float"], In[Rblti::Imatrix], Out[Rblti::Imatrix]],
[In["Tpoint<float >"], Out[Rblti::Image]],
[In["Tpoint<float >"], In[Rblti::Image], Out[Rblti::Image]],
[In["Tpoint<float >"], Out["Matrix<lti::ubyte >"]],
[In["Tpoint<float >"], In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[In["Tpoint<float >"], Out[Rblti::Fmatrix]],
[In["Tpoint<float >"], In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[In["Tpoint<float >"], Out[Rblti::Imatrix]],
[In["Tpoint<float >"], In[Rblti::Imatrix], Out[Rblti::Imatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
Rotation.form = [
[Out[Rblti::Image]],
[In[Rblti::Image], Out[Rblti::Image]],
[Out["Matrix<lti::ubyte >"]],
[In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out["double"], Out[Rblti::Image]],
[Out["double"], In[Rblti::Image], Out[Rblti::Image]],
[Out["double"], Out["Matrix<lti::ubyte >"]],
[Out["double"], In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
[Out["double"], Out[Rblti::Fmatrix]],
[Out["double"], In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
FlipImage.form = [
[Out["Matrix<lti::rgbPixel >"]],
[In["Matrix<lti::rgbPixel >"], Out["Matrix<lti::rgbPixel >"]],
[Out[Rblti::Fmatrix]],
[In[Rblti::Fmatrix], Out[Rblti::Fmatrix]],
[Out["Matrix<lti::ubyte >"]],
[In["Matrix<lti::ubyte >"], Out["Matrix<lti::ubyte >"]],
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
Rclassifier.form = [
[In[Rblti::Dvector], Out["Classifier_outputVector"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSI.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
[In[Rblti::RgbPixel]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToRGB.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHSV.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToHLS.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToXYZ.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToxyY.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToOCP.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYIQ.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToCIELuv.form = [
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

begin
SplitImageToYUV.form = [
[In[Rblti::RgbPixel], Out["float"], Out["float"], Out["float"]],
[In[Rblti::RgbPixel], Out["Ubyte"], Out["Ubyte"], Out["Ubyte"]],
[In[Rblti::Image], Out[Rblti::Channel], Out[Rblti::Channel], Out[Rblti::Channel]],
[In[Rblti::Image], Out[Rblti::Channel8], Out[Rblti::Channel8], Out[Rblti::Channel8]],
]; rescue Exception=>e; GridFlow.post "form error: %s", e.inspect end

