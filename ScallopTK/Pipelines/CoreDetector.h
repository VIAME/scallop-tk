#ifndef SCALLOP_DETECTOR_H_
#define SCALLOP_DETECTOR_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//OpenCV v2.2
#include "cv.h"
#include "highgui.h"

//File system
#ifdef WIN32
  #include "Common/filesystem_win32.h"
#else
  #include "Common/filesystem_unix.h"
#endif

//Scallop Includes
#include "Config/ConfigParsing.h"
#include "Common/display.h"
#include "Common/benchmarking.h"
#include "Common/threads.h"
#include "Size Detection/ImageProperties.h"
#include "Interest Point Detection/Consolidator.h"
#include "Interest Point Detection/HistogramFiltering.h"
#include "Interest Point Detection/PriorStatistics.h"
#include "Interest Point Detection/AdaptiveThresholding.h"
#include "Interest Point Detection/TemplateApproximator.h"
#include "Interest Point Detection/CannyPoints.h"
#include "Edge Detection/WatershedEdges.h"
#include "Edge Detection/StableSearch.h"
#include "Edge Detection/ExpensiveSearch.h"
#include "Features/ColorID.h"
#include "Features/HoG.h"
#include "Features/ShapeID.h"
#include "Features/Gabor.h"
#include "Classifiers/Training.h"
#include "Classifiers/TopClassifier.h"

//------------------------------------------------------------------------------
//                               Configurations
//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------
//                                 Prototypes
//------------------------------------------------------------------------------

int DETECT_SCALLOPS( SystemSettings& settings );

#endif
