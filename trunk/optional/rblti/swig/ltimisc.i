//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

%module ltimisc

%import utils.i

HANDLE_SIMPLE_HEADER_FILE("ltiTimer.h")
//HANDLE_SIMPLE_HEADER_FILE("ltiProcessInfo.h")

HANDLE_FUNCTOR_WITH_PARAMETERS( serial,                  "ltiSerial.h")

%{
using namespace lti;
%}
