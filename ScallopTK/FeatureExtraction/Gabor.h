#ifndef GABOR_H_
#define GABOR_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cmath>

//OpenCV 2.1
#include "cv.h"
#include "cxcore.h"

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

//void performGaborFiltering( candidate *cd );
void calculateGaborFeatures( IplImage *img_gs_32f, vector<candidate*>& cds );

#endif
