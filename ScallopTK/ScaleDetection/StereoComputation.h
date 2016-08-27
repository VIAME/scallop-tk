
#ifndef SCALLOP_TK_STEREO_COMPUTATION_H_
#define SCALLOP_TK_STEREO_COMPUTATION_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"

//------------------------------------------------------------------------------
//                            Function Declarations
//------------------------------------------------------------------------------

namespace ScallopTK
{

void computeDepthMap( cv::Mat& leftImg, cv::Mat& rightImg, cv::Mat& depthMap );

}

#endif
