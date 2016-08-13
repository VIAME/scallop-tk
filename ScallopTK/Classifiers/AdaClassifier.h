#ifndef SCALLOP_TK_ADA_CLASSIFIER_H_
#define SCALLOP_TK_ADA_CLASSIFIER_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cmath>

//Opencv
#include <cv.h>

//Scallop Includes
#include "ScallopTK/Classifiers/Classifier.h"
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/TPL/AdaBoost/BoostedCommittee.h"

//------------------------------------------------------------------------------
//                            Constants / Typedefs
//------------------------------------------------------------------------------

typedef CBoostedCommittee SingleAdaTree;

struct SingleAdaClassifier
{
  // ID of classifier object
  std::string ID;
  
  // The type of the classifier ( 0 - main, 1-3 suppression style )
  int Type;
  
  // The adaboost classifier itself
  SingleAdaTree Clsfr;
  
  // Special cases:
  //  - Is the classifier aimed at dollars?
  bool isSandDollar;
  //  - Is the classifier aimed at ALL scallops?
  bool isScallop;
  //  - More specifically, is the classifier aimed at just white scallops?
  bool IsWhite;
  //  - Is the classifier aimed at just brown scallops?
  bool IsBrown;
  //  - Is the classifier aimed at just buried scallops
  bool IsBuried;
};

class AdaClassifier : public Classifier
{

public:

  AdaClassifier() {}
  ~AdaClassifier() {}

  // Load the classifier system from a file
  bool loadClassifiers( const SystemParameters& sysParams,
    ClassifierParameters& clsParams );

  // Classify candidates points according to internal classifier
  //
  // Image should contain the input image
  // Candidates the input candidates to score
  // Positive will contain any candidates with positive classifications
  void classifyCandidates( IplImage* image,
    CandidatePtrVector& candidates,
    CandidatePtrVector& positive );

private:

  // Tier 1 classifeirs
  std::vector< SingleAdaClassifier > MainClassifiers;
  
  // Tier 2 classifiers
  std::vector< SingleAdaClassifier > SuppressionClassifiers;
  
  // Is this system aimed at scallops or something entirely different?
  bool IsScallopDirected;

  // Is the SDSS subsystem active
  bool SDSS;
  
  // Detection threshold
  double Threshold;
};

#endif
