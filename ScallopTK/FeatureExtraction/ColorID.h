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

//Opencv
#include <cv.h>
#include <cxcore.h>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/ObjectProposals/HistogramFiltering.h"

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

namespace ScallopTK
{

void createOrientedsummaryImages( IplImage *base, CandidatePtrVector& cds );

void createcolorQuadrants( IplImage *base, CandidatePtrVector& cds );

void calculatecolorFeatures( IplImage* color_img, hfResults *color_class, Candidate *cd );

}

#endif
