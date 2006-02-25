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
//  Revision 1.1  2006/02/25 23:43:56  matju
//  0.33.1
//
//  Revision 1.1  2005/12/25 15:41:16  Michael
//  splitted lti.i into different swig interface files
//
//
//******************************************************************************

//%include utils.i

// Classifiers
    
%{
#include "ltiClassifier.h"
#define _classifier classifier
#define Rclassifier_parameters parameters
#define Rclassifier_outputTemplate outputTemplate
#define Rclassifier_outputVector outputVector
namespace lti {
typedef lti::classifier::parameters classifier_parameters;
typedef lti::classifier::outputTemplate classifier_outputTemplate;
typedef lti::classifier::outputVector classifier_outputVector;
}
%}
#define parameters classifier_parameters
#define outputTemplate classifier_outputTemplate
#define outputVector classifier_outputVector
%include "_classifier_outputVector.h"
%include "_classifier_outputTemplate.h"
%include "_classifier_parameters.h"
%include "ltiClassifier.h"
#undef parameters

//HANDLE_FUNCTOR_WITH_PARAMETERS( supervisedInstanceClassifier, "ltiSupervisedInstanceClassifier.h")
%{
#include "ltiSupervisedInstanceClassifier.h"
#define _supervisedInstanceClassifier supervisedInstanceClassifier
#define _supervisedInstanceClassifier_parameters parameters
namespace lti {
typedef lti::supervisedInstanceClassifier::parameters supervisedInstanceClassifier_parameters;
}
%}
#define parameters supervisedInstanceClassifier_parameters
//%include "_supervisedInstanceClassifier_parameters.h"
%include "ltiSupervisedInstanceClassifier.h"
#undef parameters

//#define parameters decisionTree_parameters
//%include "_decisionTree_parameters.h"
//%include "ltiDecisionTree.h"
//#undef parameters
