#ifndef EXPENSIVE_SEARCH_H_
#define EXPENSIVE_SEARCH_H_

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

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void expensiveEdgeSearch( GradientChain& Gradients, hfResults* color, 
	IplImage *img_lab_32f, IplImage *img_rgb_32f, vector<candidate*> cds );

#endif
