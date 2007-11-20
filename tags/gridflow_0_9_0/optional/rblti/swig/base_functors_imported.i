//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************
#ifndef SWIGIMPORTED
#ifndef IMPORTMODE
#ifndef RBLTI
//%module base_functors
%module rblti
%include utils.i
%include dep.i
%import basedata.i
#endif
#endif
#endif

// Base Classes    
IMPORT_FUNCTOR_WITH_PARAMETERS( functor,                 "ltiFunctor.h")
IMPORT_FUNCTOR_WITH_PARAMETERS( modifier,                "ltiModifier.h")
IMPORT_FUNCTOR_WITH_PARAMETERS( filter,                  "ltiFilter.h")
IMPORT_FUNCTOR_WITH_PARAMETERS( ioFunctor,               "ltiIOFunctor.h")
