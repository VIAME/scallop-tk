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
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/EdgeDetection/GaussianEdges.h"
#include "ScallopTK/ObjectProposals/HistogramFiltering.h"

//Benchmarking
#ifdef SS_BENCHMARKING
	#include "ScallopTK/Utilities/Benchmarking.h"
#endif

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void edgeSearch( GradientChain& Gradients, hfResults* color, IplImage *img_lab_32f, vector<candidate*> cds, IplImage *rgb );

#endif
