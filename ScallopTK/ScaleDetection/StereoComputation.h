
#ifndef SCALLOP_TK_STEREO_COMPUTATION_H_
#define SCALLOP_TK_STEREO_COMPUTATION_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"

//------------------------------------------------------------------------------
//                            Function Declarations
//------------------------------------------------------------------------------

void computeDepthMap( IplImage* leftImg, IplImage* rightImg, IplImage* depthMap );

#endif
