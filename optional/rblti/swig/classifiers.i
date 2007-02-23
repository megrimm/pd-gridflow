//******************************************************************************
// Rblti, Copyright 2005,2006 by Michael Neuroth and Heri Andria
//  $Id$
//******************************************************************************

%module rblti
%include utils.i
%include dep.i
%import basedata.i
%include base_functors_imported.i
%{
using namespace lti;
typedef std::ostream ostream;
%}

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
