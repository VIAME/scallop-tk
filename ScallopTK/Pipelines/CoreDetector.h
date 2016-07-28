#ifndef SCALLOP_DETECTOR_H_
#define SCALLOP_DETECTOR_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

// Standard C/C++
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// OpenCV v2.2
#include "cv.h"
#include "highgui.h"

// File system
#ifdef WIN32
  #include "ScallopTK/Utilities/FilesystemWin32.h"
#else
  #include "ScallopTK/Utilities/FilesystemUnix.h"
#endif

// Internal Definitions
#include "ScallopTK/Utilities/Threads.h"

//------------------------------------------------------------------------------
//                               Configurations
//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------
//                                 Prototypes
//------------------------------------------------------------------------------

int DETECT_SCALLOPS( SystemSettings& settings );

#endif
