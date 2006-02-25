//******************************************************************************
//
//      project:  pylti
//    copyright:  (c) 2005 by Michael Neuroth
//
//  description:  python modul for ltilib (using SWIG)
//                    misc. ltilib functions and classes
//
//  $Source$
//  $Revision$
//  $Date$
// 
//  $Log$
//  Revision 1.1  2006/02/25 23:43:56  matju
//  0.33.1
//
//  Revision 1.1  2005/12/25 15:41:16  Michael
//  splitted lti.i into different swig interface files
//
//
//******************************************************************************

%module ltimisc

%import utils.i

HANDLE_SIMPLE_HEADER_FILE("ltiTimer.h")
//HANDLE_SIMPLE_HEADER_FILE("ltiProcessInfo.h")

HANDLE_FUNCTOR_WITH_PARAMETERS( serial,                  "ltiSerial.h")

%{
using namespace lti;
%}
