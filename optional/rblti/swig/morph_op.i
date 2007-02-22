//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
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
