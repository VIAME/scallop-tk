#ifndef STABLE_SEARCH_H_
#define STABLE_SEARCH_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <stack>

//OpenCV 2.2
#include "cv.h"
#include "cxcore.h"

//Scallop Includes
#include "../Common/definitions.h"
#include "../Common/helper.h"
#include "../Edge Detection/GaussianEdges.h"
#include "../Interest Point Detection/HistogramFiltering.h"

//Benchmarking
#ifdef SS_BENCHMARKING
	#include "../Common/benchmarking.h"
#endif

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void edgeSearch( GradientChain& Gradients, hfResults* color, IplImage *img_lab_32f, vector<candidate*> cds, IplImage *rgb );

#endif