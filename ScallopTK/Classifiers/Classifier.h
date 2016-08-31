#ifndef SCALLOP_TK_CLASSIFIER_H_
#define SCALLOP_TK_CLASSIFIER_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <vector>

//OpenCV
#include <cv.h>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"

//------------------------------------------------------------------------------
//                              Class Definition
//------------------------------------------------------------------------------

namespace ScallopTK
{

// Simple class for storing info about a classifier output
class ClassifierIDLabel
{
public:

  // ID of the target object
  std::string id;

  // Special cases:
  //  - Is the classifier bin aimed at detecting background?
  bool isBackground;
  //  - Is the classifier bin aimed at sand dollars?
  bool isSandDollar;
  //  - Is the classifier bin aimed at ALL scallops?
  bool isScallop;
  //  - More specifically, is the classifier bin aimed at just white scallops?
  bool isWhite;
  //  - Is the classifier bin aimed at just brown scallops?
  bool isBrown;
  //  - Is the classifier bin aimed at just buried scallops
  bool isBuried;

  // Constructors
  ClassifierIDLabel() {}
  ~ClassifierIDLabel() {}
};

// Base class for arbitrary classifiers
class Classifier
{
public:

  Classifier() {}
  virtual ~Classifier() {}

  // Load the classifier system from a file
  virtual bool loadClassifiers(
    const SystemParameters& sysParams,
    const ClassifierParameters& clsParams ) = 0;

  // Classify candidates points according to internal classifier
  //
  // Image should contain the input image
  // Candidates the input candidates to score
  // Positive will contain any candidates with positive classifications
  virtual void classifyCandidates( cv::Mat image,
    CandidatePtrVector& candidates,
    CandidatePtrVector& positive ) = 0;

  // Does this classifier require feature extraction?
  virtual bool requiresFeatures() = 0;

  // Does this classifier have anything to do with scallop detection?
  virtual bool detectsScallops() = 0;

  // Extract training samples
  //
  // Image should contain the input image
  // Candidates the input candidates to match
  // GroundTruth the groundtruth candidates
  virtual void extractSamples( cv::Mat image,
    CandidatePtrVector& candidates,
    CandidatePtrVector& groundTruth ) = 0;

  // Number of individual output classes this classifier has
  virtual int outputClassCount() = 0;
  virtual int suppressionClassCount() = 0;

  // Get information about each bin that this classifier outputs
  virtual ClassifierIDLabel* getLabel( int label ) = 0;
  virtual ClassifierIDLabel* getSuppressionLabel( int label ) = 0;
};

//------------------------------------------------------------------------------
//                          Assorted Helper Functions
//------------------------------------------------------------------------------

// Load a new classifier
Classifier* loadClassifiers( const SystemParameters& sysParams,
  const ClassifierParameters& clsParams );

// Suppress inside (duplicate) points which probably correspond to the same object
void removeInsidePoints( CandidatePtrVector& input,
  CandidatePtrVector& output, int method );
void removeInsidePoints( CandidatePtrVector& input,
  CandidatePtrVector& output );

// Perform suppression and interpolation
DetectionPtrVector interpolateResults( CandidatePtrVector& input,
  Classifier* Classifiers, std::string Filename );

// Deallocate Detections
void deallocateDetections( DetectionPtrVector& vec );

// Append GT training results to some file
bool appendInfoToFile( DetectionVector& Detections, const std::string& list_fn );
  
// Append final Detections to some file
bool appendInfoToFile( DetectionVector& cds, const std::string& ListFilename,
  const std::string& this_fn );

}


#endif
