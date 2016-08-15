//------------------------------------------------------------------------------
// Title: Display.h
// Author: Matthew Dawkins
//------------------------------------------------------------------------------

#ifndef SCALLOP_TK_DISPLAY_SYS_H
#define SCALLOP_TK_DISPLAY_SYS_H

// C/C++ Includes
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// OpenCV Includes
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

// Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"

// Namespaces
using namespace std;

//------------------------------------------------------------------------------
//                                  Variables
//------------------------------------------------------------------------------

const float DISPLAY_MAX_WIDTH = 600.f;
const float DISPLAY_MAX_HEIGHT = 600.f;

//------------------------------------------------------------------------------
//                              Function Definitions
//------------------------------------------------------------------------------

void initOutputDisplay();
void killOuputDisplay();
void displayImage( IplImage* img );
void displayInterestPointImage( IplImage* img, CandidatePtrVector& cds );
void displayResultsImage( IplImage* img, CandidatePtrVector& Scallops );
void displayResultsImage( IplImage* img, DetectionPtrVector& cds, string Filename );

#endif
