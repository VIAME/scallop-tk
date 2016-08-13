#ifndef SCALLOP_TK_CANNY_POINTS_H_
#define SCALLOP_TK_CANNY_POINTS_H_

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
#include <stack>

//Opencv
#include <cv.h>
#include <cxcore.h>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/ScaleDetection/ImageProperties.h"
#include "ScallopTK/EdgeDetection/GaussianEdges.h"

//------------------------------------------------------------------------------
//                             Internal Constants
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void findCannyCandidates( GradientChain& grad, CandidatePtrVector& cds );

#endif
