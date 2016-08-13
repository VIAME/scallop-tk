#ifndef SCALLOP_TK_STABLE_SEARCH_H_
#define SCALLOP_TK_STABLE_SEARCH_H_

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

//Opencv
#include <cv.h>
#include <cxcore.h>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/EdgeDetection/GaussianEdges.h"
#include "ScallopTK/ObjectProposals/HistogramFiltering.h"

//Benchmarking
#ifdef SS_ENABLE_BENCHMARKINGING
  #include "ScallopTK/Utilities/Benchmarking.h"
#endif

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void edgeSearch( GradientChain& Gradients,
                 hfResults* color,
                 IplImage *ImgLab32f,
                 vector<Candidate*> cds,
                 IplImage *rgb );

#endif
