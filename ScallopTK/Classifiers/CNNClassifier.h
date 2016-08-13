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
  virtual void classifyCandidates( IplImage* image,
    CandidatePtrVector& candidates,
    CandidatePtrVector& positive );

private:

  caffe::Net< float >* initialClfr;

  bool suppressionEnabled;
  caffe::Net< float >* suppressionClfr;
  
};

#endif
