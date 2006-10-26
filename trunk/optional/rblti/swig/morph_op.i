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
//  Revision 1.3  2006/10/26 18:28:38  heri
//  Compilation in separate modules now working.
//  basedata and base_functors MUST be loaded before any other modules, otherwise you get a segfault.
//
//  Revision 1.2  2006/08/25 23:32:12  heri
//  A bunch of changes to split compilation into several parts (not yet working for some modules)
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
%include base_functors_imported.i

IMPORT_FUNCTOR_WITH_PARAMETERS( transform,               "ltiTransform.h")
#endif

// Morphological Operators    
HANDLE_FUNCTOR_WITH_PARAMETERS( morphology,              "ltiMorphology.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( erosion,                 "ltiErosion.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( dilation,                "ltiDilation.h")
HANDLE_FUNCTOR_WITH_PARAMETERS( skeleton,                "ltiSkeleton.h")
