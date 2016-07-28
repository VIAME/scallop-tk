//------------------------------------------------------------------------------
// Title: 
// Author: Matthew Dawkins
// Description: 
//------------------------------------------------------------------------------

#ifndef PRIORSTATS_H_
#define PRIORSTATS_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <fstream>

//OpenCV 2.1
#include "cv.h"
#include "cxcore.h"

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

class ThreadStatistics {

public:

	// Functions
	ThreadStatistics();
	~ThreadStatistics() {}
	void Update( int detections[], float im_area );
	float returnMaxMinRadRequired() { return global_min_rad; }

	// Detected Densities - last 100
	float ScallopDensity;
	float ClamDensity;
	float SandDollarDensity;
	float UrchinDensity;

	// Detected Densities - last 1
	float ScallopDensityPrev;
	float ClamDensityPrev;
	float SandDollarDensityPrev;
	float UrchinDensityPrev;

	// Entries detected across all 
	int thread_detections[TOTAL_DESIG];

	// Images processed
	int processed;

private:

	// Smallest image size required for all ops (pixels)
	float global_min_rad;
};

#endif
