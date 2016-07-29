#ifndef SCALLOP_TK_EDGELINK_H_
#define SCALLOP_TK_EDGELINK_H_

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
#include "ScallopTK/ObjectProposals/DoG.h"

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
