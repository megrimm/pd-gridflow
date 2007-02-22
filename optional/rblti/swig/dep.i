//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$

// for the access to c arrays
%include "carrays.i"
%array_functions(float,floatArray)
%array_functions(double,doubleArray)
%array_functions(int,intArray)
%array_class(float,floatArr)
%array_class(double,doubleArr)
%array_class(int,intArr)

%{
#include "ltiTypes.h"
#include "ltiObject.h"
#include "ltiIoHandler.h"
#include "ltiBoundaryType.h"
#include "ltiStatus.h"
#include "ltiIoObject.h"
#include "ltiMathObject.h"
#include "ltiRGBPixel.h"
#include "ltiPoint.h"
#include "ltiRectangle.h"
#include "ltiGenericVector.h"
#include "ltiVector.h"
#include "ltiArray.h"
#include "ltiGenericMatrix.h"
#include "ltiMatrix.h"
#include "ltiPointList.h"
#include "ltiLocation.h"
#include "ltiPolygonPoints.h"
#include "ltiGeometry.h"
#include "ltiHTypes.h"
#include "ltiHistogram.h"
#include "ltiImage.h"
#include "ltiContour.h"
#include "ltiLinearKernels.h"
#include "ltiGradientKernels.h"
#include "ltiHessianKernels.h"
#include "ltiLaplacianKernel.h"
#include "ltiSecondDerivativeKernels.h"
#include "ltiBinaryKernels.h"

%}