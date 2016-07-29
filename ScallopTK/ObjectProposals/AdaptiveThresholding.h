#ifndef SCALLOP_TK_ADAPTIVETHRESH_H_
#define SCALLOP_TK_ADAPTIVETHRESH_H_

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

//OpenCV 2.1
#include "cv.h"
#include "cxcore.h"

//Scallop Includes
#include "ScallopTK/ObjectProposals/HistogramFiltering.h"
#include "ScallopTK/EdgeDetection/EdgeLinking.h"
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------

// DEPRECATED
// Container for filter result statistics
/*const int HF_STAT_SIZE = 33;
typedef float hfFilterStats[HF_STAT_SIZE];
const float hfStatPercentiles[HF_STAT_SIZE] = 
{ 0.000f, 0.800f, 0.900f, 0.700f, 
  0.950f, 0.850f, 0.750f, 0.600f,
  0.975f, 0.925f, 0.875f, 0.825f, 0.775f, 0.725f, 0.650f, 0.400f, 
  0.987f, 0.962f, 0.937f, 0.912f, 0.887f, 0.862f, 0.837f, 0.812f,
  0.788f, 0.762f, 0.737f, 0.712f, 0.675f, 0.625f, 0.500f, 0.150f, 1.00f };*/

// Size of scanning percentiles
const int AT_LOWER_SIZE = 4;
const int AT_UPPER_SIZE = 4;

// Percentiles to perform thresholding at
const float AT_LOWER_PER[AT_LOWER_SIZE] = { 0.00, 0.33, 0.18, 0.66 };
const float AT_UPPER_PER[AT_UPPER_SIZE] = { 0.00, 0.80, 0.60, 0.90 };

// Specifies which values in the input (classified) image correspond to which %tiles
struct atStats {
	float lower_intvls[AT_LOWER_SIZE];
	float upper_intvls[AT_UPPER_SIZE];
};

// Return variables for bin search
const int AT_LOWER = 0x01;
const int AT_RAISE = 0x02;

// Sampling points for approximating percentiles
const int AT_SAMPLES = 1200;

//------------------------------------------------------------------------------
//                                Prototypes
//------------------------------------------------------------------------------

void performAdaptiveFiltering( hfResults* color, vector<candidate*>& cds, float minRad, bool doubleIntrp = false );

#endif
