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

namespace ScallopTK
{

class AdaClassifier : public Classifier
{

public:

  AdaClassifier() {}
  ~AdaClassifier() {}

  // Load the classifier system from a file
  bool loadClassifiers(
    const SystemParameters& sysParams,
    const ClassifierParameters& clsParams );

  // Classify candidates points according to internal classifier
  //
  // Image should contain the input image
  // Candidates the input candidates to score
  // Positive will contain any candidates with positive classifications
  void classifyCandidates( cv::Mat image,
    CandidatePtrVector& candidates,
    CandidatePtrVector& positive );

  // Does this classifier require feature extraction?
  bool requiresFeatures() { return true; }

  // Does this classifier have anything to do with scallop detection?
  bool detectsScallops() { return isScallopDirected; }

  // Extract training samples
  //
  // Image should contain the input image
  // Candidates the input candidates to match
  // GroundTruth the groundtruth candidates
  virtual void extractSamples( cv::Mat image,
    CandidatePtrVector& candidates,
    CandidatePtrVector& groundTruth );

  // Number of individual output classes this classifier has
  virtual int outputClassCount()
    { return mainClassifiers.size(); }
  virtual int suppressionClassCount()
    { return suppressionClassifiers.size(); }

  // Get information about each bin that this classifier outputs
  ClassifierIDLabel* getLabel( int label )
    { return &mainClassifiers[label]; }
  ClassifierIDLabel* getSuppressionLabel( int label )
    { return &suppressionClassifiers[label]; };

private:

  typedef CBoostedCommittee SingleAdaTree;
  
  class SingleAdaClassifier : public ClassifierIDLabel
  {
  public:

    // The type of the classifier ( 0 - main, 1-3 suppression style )
    int type;

    // The adaboost decesion tree itself
    SingleAdaTree adaTree;
  };

  typedef std::vector< SingleAdaClassifier > AdaVector;

  // Helper function
  int classifyCandidate( cv::Mat image, Candidate* candidate );

  // Tier 1 classifeirs
  AdaVector mainClassifiers;
  
  // Tier 2 classifiers
  AdaVector suppressionClassifiers;
  
  // Is this system aimed at scallops or something entirely different?
  bool isScallopDirected;

  // Detection threshold
  double threshold;

  // Training keep percent
  double trainingPercentKeep;

  // Output file
  std::string outputList;
};

}

#endif
