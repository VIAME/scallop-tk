#ifndef SCALLOP_TK_HOG_H_
#define SCALLOP_TK_HOG_H_

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
#include <cxcore.h>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------

const int HOG_PIXEL_WIDTH_REQUIRED = 10;
const int HOG_NORMALIZATION_METHOD = CV_L2;

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

class HoGFeatureGenerator {

public:

  // Calculates required integral images
  HoGFeatureGenerator( IplImage *img_gs, float minR, float maxR, int index );

  // Performs necessary deallocations
  ~HoGFeatureGenerator();

  // Sets any desired options
  void SetOptions( float add_ratio, float bins_per_dim );

  // Generates descriptors for all Candidates
  void Generate( vector<Candidate*>& cds );

  // Generates descriptors for a single Candidate
  bool GenerateSingle( Candidate *cd );

private:

  // Integral image chain for feature generation
  IplImage **integrals;

  // Internal Stats for integral images
  float minRad;
  float maxRad;

  // Calculation Options
  float add_ratio;
  float bins;

  // Output index in Candidate
  int output_index;
};

//DEPRECATED
//void calculateRHoG( Candidate *cd, IplImage *base );
//void calculateCHoG( Candidate *cd, IplImage *base );
//void calculateRHoG16( Candidate *cd, IplImage *base );
//void HoGTest( IplImage *gs, vector<Candidate*> cds );

#endif
