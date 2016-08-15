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
bool appendInfoToFile( DetectionPtrVector& cds, const std::string& ListFilename,
  const std::string& this_fn, float resize_factor );


#endif
