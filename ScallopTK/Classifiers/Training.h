#ifndef SCALLOP_TK_CLASS_TRAINING_H_
#define SCALLOP_TK_CLASS_TRAINING_H_

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
#include "ScallopTK/TPL/AdaBoost/BoostedCommittee.h"
#include "ScallopTK/ObjectProposals/Consolidator.h"

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

// Print Candidate features to file for GUI mode
void printCandidateInfo( int desig, Candidate *cd );

// End GUI training mode
void exitTrainingMode();

// Get designations from user in GUI mode
bool getDesignationsFromUser(vector<Candidate*>& UnorderedCandidates, IplImage *display_img, IplImage *mask,
								int *Detections, float minRad, float maxRad, string img_name );
bool getDesignationsFromUser(CandidateQueue& OrderedCandidates, IplImage *display_img, IplImage *mask,
								int *Detections, float minRad, float maxRad, string img_name );

// Print out features to given file in MIP mode
void dumpCandidateFeatures( string file_name, vector<Candidate*>& cd );

#endif
