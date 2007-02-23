//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i

// Color Quantization and Related Topics
HANDLE_FUNCTOR_WITH_PARAMETERS( colorQuantization,       "ltiColorQuantization.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( kMColorQuantization,     "ltiKMColorQuantization.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( lkmColorQuantization,    "ltiLkmColorQuantization.h")
//TODO HANDLE_FUNCTOR_WITH_PARAMETERS( medianCut,               "ltiMedianCut.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( computePalette,          "ltiComputePalette.h")
%{
  typedef lti::kMColorQuantization::parameters lti_kMColorQuantization_parameters;
  using namespace lti;
%}
#endif
HANDLE_FUNCTOR_WITH_PARAMETERS( usePalette,              "ltiUsePalette.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageTorgI.h")
HANDLE_SIMPLE_HEADER_FILE("ltiMergeImage.h")
