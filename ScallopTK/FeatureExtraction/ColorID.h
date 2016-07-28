#ifndef COLORID_H_
#define COLORID_H_

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
#include "ScallopTK/ObjectProposals/HistogramFiltering.h"

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void createOrientedSummaryImages( IplImage *base, vector<candidate*>& cds );
void createColorQuadrants( IplImage *base, vector<candidate*>& cds );
void calculateColorFeatures( IplImage* color_img, hfResults *color_class, candidate *cd );

#endif
