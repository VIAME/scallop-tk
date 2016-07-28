#ifndef CLASS_TRAINING_H_
#define CLASS_TRAINING_H_

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
#include "../Common/definitions.h"
#include "../Common/helper.h"
#include "../Classifiers/AdaBoost/BoostedCommittee.h"
#include "../Interest Point Detection/Consolidator.h"

//------------------------------------------------------------------------------
//                                  Globals
//------------------------------------------------------------------------------

static ofstream instruction_file;
static ofstream data_file;
static std::string ip_file_out;
static bool training_exit_flag = false;

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

// Initialize internal properties needed for GUI trainner
bool initializeTrainingMode( const std::string& folder, const std::string& file );

// Print candidate features to file for GUI mode
void printCandidateInfo( int desig, candidate *cd );

// End GUI training mode
void exitTrainingMode();

// Get designations from user in GUI mode
bool getDesignationsFromUser(vector<candidate*>& UnorderedCandidates, IplImage *display_img, IplImage *mask,
								int *detections, float minRad, float maxRad, string img_name );
bool getDesignationsFromUser(CandidateQueue& OrderedCandidates, IplImage *display_img, IplImage *mask,
								int *detections, float minRad, float maxRad, string img_name );

// Print out features to given file in MIP mode
void dumpCandidateFeatures( string file_name, vector<candidate*>& cd );

#endif
