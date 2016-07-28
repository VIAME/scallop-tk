//------------------------------------------------------------------------------
// Title: 
// Author: 
//------------------------------------------------------------------------------

#ifndef CANNYIP_H_
#define CANNYIP_H_

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

//OpenCV 2.1
#include "cv.h"
#include "cxcore.h"

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

void findCannyCandidates( GradientChain& grad, std::vector<candidate*>& cds );

#endif
