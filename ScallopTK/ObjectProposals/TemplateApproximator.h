//------------------------------------------------------------------------------
// Title: Template3.h
// Author: Matthew Dawkins
// Description: Double Donut Detection
//------------------------------------------------------------------------------

#ifndef TEMPLATE4_H_
#define TEMPLATE4_H_

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
#include "../Common/definitions.h"
#include "../Common/helper.h"
#include "../Size Detection/ImageProperties.h"
#include "../Edge Detection/GaussianEdges.h"

//Benchmarking
#ifdef TEMPLATE_BENCHMARKING
	#include "../Common/benchmarking.h"
#endif

//------------------------------------------------------------------------------
//                             Internal Constants
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void findTemplateCandidates( GradientChain& grad, std::vector<candidate*>& cds, ImageProperties& imgProp, IplImage* mask = NULL );

#endif