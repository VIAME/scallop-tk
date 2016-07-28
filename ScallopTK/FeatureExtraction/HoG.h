#ifndef HOG_H_
#define HOG_H_

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
#include "../Common/definitions.h"
#include "../Common/helper.h"

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

	// Generates descriptors for all candidates
	void Generate( vector<candidate*>& cds );

	// Generates descriptors for a single candidate
	bool GenerateSingle( candidate *cd );

private:

	// Integral image chain for feature generation
	IplImage **integrals;

	// Internal Stats for integral images
	float minRad;
	float maxRad;

	// Calculation Options
	float add_ratio;
	float bins;

	// Output index in candidate
	int output_index;
};

//DEPRECATED
//void calculateRHoG( candidate *cd, IplImage *base );
//void calculateCHoG( candidate *cd, IplImage *base );
//void calculateRHoG16( candidate *cd, IplImage *base );
//void HoGTest( IplImage *gs, vector<candidate*> cds );

#endif