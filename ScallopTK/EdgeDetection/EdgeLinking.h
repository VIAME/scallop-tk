#ifndef EDGELINK_H_
#define EDGELINK_H_

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
#include "../Interest Point Detection/DoG.h"

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

int findStableMatches( CvSeq *seq, float minRad, float maxRad, vector<candidate*>& kps, IplImage* bin );

//------------------------------------------------------------------------------
//                              Filter Declaration
//------------------------------------------------------------------------------


#endif