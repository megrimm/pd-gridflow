//******************************************************************************
//
//      project:  pylti
//    copyright:  (c) 2005 by Michael Neuroth
//
//  description:  python modul for ltilib (using SWIG)
//                    ltilib extras functions and classes
//
//  $Source$
//  $Revision$
//  $Date$
// 
//  $Log$
//  Revision 1.6  2006/08/25 23:32:12  heri
//  A bunch of changes to split compilation into several parts (not yet working for some modules)
//
//  Revision 1.5  2006/06/23 18:01:32  heri
//  Fix for 64bit OS compilation error. Basically, just added gcc -fPIC flag.
//
//  Revision 1.1  2006/02/25 23:43:56  matju
//  0.33.1
//
//  Revision 1.1  2005/12/25 15:41:16  Michael
//  splitted lti.i into different swig interface files
//
//
//******************************************************************************

//%include utils.i
#ifndef OLD_COMPILE
%module rblti
%include utils.i
%include dep.i
%import basedata.i
#define IMPORTMODE
%include base_functors.i
#undef IMPORTMODE
#endif


// Color Quantization and Related Topics
HANDLE_FUNCTOR_WITH_PARAMETERS( colorQuantization,       "ltiColorQuantization.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( kMColorQuantization,     "ltiKMColorQuantization.h")
    %{
    typedef lti::kMColorQuantization::parameters lti_kMColorQuantization_parameters;
    %}
HANDLE_FUNCTOR_WITH_PARAMETERS( lkmColorQuantization,    "ltiLkmColorQuantization.h")
//TODO HANDLE_FUNCTOR_WITH_PARAMETERS( medianCut,               "ltiMedianCut.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( computePalette,          "ltiComputePalette.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( usePalette,              "ltiUsePalette.h")

HANDLE_SIMPLE_HEADER_FILE("ltiSplitImage.h")
HANDLE_SIMPLE_HEADER_FILE("ltiSplitImageTorgI.h")

HANDLE_SIMPLE_HEADER_FILE("ltiMergeImage.h")