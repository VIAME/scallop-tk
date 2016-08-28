#ifndef SCALLOP_TK_SHAPEID_H_
#define SCALLOP_TK_SHAPEID_H_

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

//Opencv
#include <cv.h>
#include <cxcore.h>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/ScaleDetection/ImageProperties.h"

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

namespace ScallopTK
{

void calculateShapeFeatures( Candidate *cd, IplImage *unused );
void calculateSizeFeatures( Candidate *cd, ImageProperties& ip, float initResize );

}

#endif
