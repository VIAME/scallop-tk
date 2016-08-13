
#include "CNNClassifier.h"

CNNClassifier::CNNClassifier()
{
  initialClfr = NULL;
  suppressionEnabled = false;
  suppressionClfr = NULL;
}

CNNClassifier::~CNNClassifier()
{
  if( initialClfr )
  {
    delete initialClfr;
  }

  if( suppressionClfr )
  {
    delete suppressionClfr;
  }
}

bool CNNClassifier::loadClassifiers(
  const SystemParameters& sysParams,
  const ClassifierParameters& clsParams )
{
  
}

void CNNClassifier::classifyCandidates(
  IplImage* image,
  CandidatePtrVector& candidates,
  CandidatePtrVector& positive )
{

}
