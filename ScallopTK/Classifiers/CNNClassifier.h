#ifndef SCALLOP_TK_CNN_CLASSIFIER_H_
#define SCALLOP_TK_CNN_CLASSIFIER_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <vector>

//OpenCV
#include <cv.h>

//Caffe
#include <caffe/net.hpp>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Classifiers/Classifier.h"

//------------------------------------------------------------------------------
//                              Class Definitions
//------------------------------------------------------------------------------

class CNNClassifier : public Classifier
{
public:

  CNNClassifier();
  ~CNNClassifier();

  // Load the classifier system from a file
  virtual bool loadClassifiers(
    const SystemParameters& sysParams,
    const ClassifierParameters& clsParams );

  // Classify candidates points according to internal classifier
  //
  // Image should contain the input image
  // Candidates the input candidates to score
  // Positive will contain any candidates with positive classifications
  virtual void classifyCandidates( cv::Mat image,
    CandidatePtrVector& candidates,
    CandidatePtrVector& positive );

  // Does this classifier require feature extraction?
  bool requiresFeatures() { return false; }

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

private:

  struct IDLabel
  {
    // ID of the target object
    std::string id;

    // Special cases:
    //  - Is the classifier bin aimed at detecting background?
    bool isBackground;
    //  - Is the classifier bin aimed at dollars?
    bool isSandDollar;
    //  - Is the classifier bin aimed at ALL scallops?
    bool isScallop;
    //  - More specifically, is the classifier bin aimed at just white scallops?
    bool isWhite;
    //  - Is the classifier bin aimed at just brown scallops?
    bool isBrown;
    //  - Is the classifier bin aimed at just buried scallops
    bool isBuried;
  };

  typedef std::vector< IDLabel > IDVector;
  typedef caffe::Net< float > CNN;
  typedef caffe::Caffe::Brew DeviceMode;

  // Main (initial) classifier applied to all candidates
  CNN* initialClfr;
  IDVector initialClfrLabels;

  // Optional suppression classifiers
  CNN* suppressionClfr;
  IDVector suppressionClfrLabels;

  // Is this system aimed at scallops or something entirely different?
  bool isScallopDirected;

  // Detection threshold
  double threshold;
  double trainingPercentKeep;

  // Are we in training mode
  bool isTrainingMode;
  std::string outputFolder;

  // GPU device settings
  DeviceMode deviceMode;
  int deviceID;
  double deviceMem;

  // Helper functions
  void deallocCNNs();
  cv::Mat getCandidateChip( cv::Mat image, Candidate* cd, int width, int height );
};

#endif
