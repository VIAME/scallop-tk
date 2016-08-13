#ifndef SCALLOP_TK_CNN_CLASSIFIER_H_
#define SCALLOP_TK_CNN_CLASSIFIER_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <vector>

//OpenCV
#include <cv.h>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Classifiers/Classifier.h"

//------------------------------------------------------------------------------
//                              Class Definitions
//------------------------------------------------------------------------------

class CNNClassifier : public Classifier
{
public:

  explicit CNNClassifier() {}
  ~CNNClassifier() {}

  // Classify candidates points according to internal classifier
  void classifyCandidates( IplImage* image,
    CandidatePtrVector& candidates,
    CandidatePtrVector& positive );

private:

  
};

#endif
