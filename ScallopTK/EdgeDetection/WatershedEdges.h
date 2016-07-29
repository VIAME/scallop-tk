#ifndef SCALLOP_TK_WATERSHED_H_
#define SCALLOP_TK_WATERSHED_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

//OpenCV 2.2
#include "cv.h"
#include "cxcore.h"

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/EdgeDetection/GaussianEdges.h"

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void extractWatershedEdge( IplImage *img, candidate* kp );
void extractScallopContours( GradientChain& Gradients, vector<candidate*> cds );

#endif
