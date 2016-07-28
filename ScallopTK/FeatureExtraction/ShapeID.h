#ifndef SHAPEID_H_
#define SHAPEID_H_

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
#include "../Common/definitions.h"
#include "../Common/helper.h"
#include "../Size Detection/ImageProperties.h"

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void calculateShapeFeatures( candidate *cd, IplImage *unused );
void calculateSizeFeatures( candidate *cd, ImageProperties& ip, float initResize );

#endif