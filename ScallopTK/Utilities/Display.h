//------------------------------------------------------------------------------
// Title: Display.h
// Author: Matthew Dawkins
//------------------------------------------------------------------------------

#ifndef DISPLAY_SYS_H
#define DISPLAY_SYS_H

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

const int DISPLAY_WIDTH = 435;
const int DISPLAY_HEIGHT = 345;

//------------------------------------------------------------------------------
//                              Function Definitions
//------------------------------------------------------------------------------

void initOutputDisplay();
void killOuputDisplay();
void displayImage( IplImage* img, string wname = "SCALLOPS" );
void displayInterestPointImage( IplImage* img, vector<candidate*>& cds );
void displayResultsImage( IplImage* img, vector<candidate*>& Scallops );
void displayResultsImage( IplImage* img, vector<detection*>& cds, string Filename );

#endif
