#ifndef SCALLOP_TK_COLORID_H_
#define SCALLOP_TK_COLORID_H_

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

void createOrientedsummaryImages( IplImage *base, vector<Candidate*>& cds );
void createcolorQuadrants( IplImage *base, vector<Candidate*>& cds );
void calculatecolorFeatures( IplImage* color_img, hfResults *color_class, Candidate *cd );

#endif
