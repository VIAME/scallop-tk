#ifndef SCALLOP_TK_GAUSSEDGE_H_
#define SCALLOP_TK_GAUSSEDGE_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

//OpenCV 2.1
#include "cv.h"
#include "cxcore.h"

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/ObjectProposals/HistogramFiltering.h"

//------------------------------------------------------------------------------
//                                Definitions
//------------------------------------------------------------------------------

struct GradientChain {

  // Size Properties shared for all images
  float scale;
  float minRad;
  float maxRad;

  // Lab color space edges
  IplImage *dxColorSig1;
  IplImage *dyColorSig1;
  IplImage *dxMergedSig1;
  IplImage *dyMergedSig1;
  IplImage *dMergedSig1;

  // Lab approx edges
  IplImage *dLabMag;
  IplImage *dLabOri;

  // Color Classifier Edges
  IplImage *dxCCGrad;
  IplImage *dyCCGrad;
  IplImage *netCCGrad;

  // Gray-scale edges
  IplImage *gsEdge;

  // Template matching inputs
  IplImage *dx;
  IplImage *dy;

  // Misc
  IplImage *cannyEdges;

  // Watershed inputs
  IplImage *WatershedInput;
};

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------

const float KERNEL_SIZE_PER_SIGMA = 6;

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

IplImage *gaussDerivVerticle( IplImage *input, double sigma );
IplImage *gaussDerivHorizontal( IplImage *input, double sigma );
GradientChain createGradientChain( IplImage *img_lab, IplImage *img_gs_32f, IplImage *img_gs_8u, IplImage *img_rgb_8u, hfResults *color, float minRad, float maxRad );
void deallocateGradientChain( GradientChain& chain );


#endif
