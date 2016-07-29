//------------------------------------------------------------------------------
// Title: Template3.h
// Author: Matthew Dawkins
// Description: Double Donut Detection
//------------------------------------------------------------------------------

#ifndef SCALLOP_TK_TEMPLATE4_H_
#define SCALLOP_TK_TEMPLATE4_H_

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
#include "ScallopTK/ScaleDetection/ImageProperties.h"
#include "ScallopTK/EdgeDetection/GaussianEdges.h"

//Benchmarking
#ifdef TEMPLATE_BENCHMARKING
	#include "ScallopTK/Utilities/Benchmarking.h"
#endif

//------------------------------------------------------------------------------
//                             Internal Constants
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void findTemplateCandidates( GradientChain& grad, std::vector<Candidate*>& cds, ImageProperties& imgProp, IplImage* mask = NULL );

#endif
