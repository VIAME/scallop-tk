#ifndef SCALLOP_TK_EXPENSIVE_SEARCH_H_
#define SCALLOP_TK_EXPENSIVE_SEARCH_H_

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

//OpenCV
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
	IplImage *ImgLab32f, IplImage *img_rgb_32f, vector<Candidate*> cds );

#endif
