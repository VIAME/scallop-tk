#ifndef SCALLOP_TK_TOP_CLASSIFIERS_H_
#define SCALLOP_TK_TOP_CLASSIFIERS_H_

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
#include "ScallopTK/Utilities/Threads.h"

//------------------------------------------------------------------------------
//                            Constants / Typedefs
//------------------------------------------------------------------------------

typedef CBoostedCommittee SingleAdaClassifier;

struct Classifier
{
	// ID of classifier object
	std::string ID;
	
	// The type of the classifier ( 0 - main, 1-3 suppression style )
	int Type;
	
	// The adaboost classifier itself
	SingleAdaClassifier Clsfr;
	
	// Special cases:
	//  - Is the classifier aimed at dollars?
	bool IsDollar;
	//  - Is the classifier aimed at ALL scallops?
	bool IsScallop;
	//  - More specifically, is the classifier aimed at just white scallops?
	bool IsWhite;
	//  - Is the classifier aimed at just brown scallops?
	bool IsBrown;
	//  - Is the classifier aimed at just buried scallops
	bool IsBuried;
};

struct ClassifierSystem
{
	// Tier 1 classifeirs
	std::vector< Classifier > MainClassifiers;
	
	// Tier 2 classifiers
	std::vector< Classifier > SuppressionClassifiers;
	
	// Is this system aimed at scallops or something entirely different?
	bool IsScallopDirected;

	// Is the SDSS subsystem active
	bool SDSS;
	
	// Detection threshold
	double Threshold;
};

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

// Generate all raw classifier values for an interest point as required
int classifyCandidate( Candidate *cd, ClassifierSystem *Classifiers );

// Suppress inside (duplicate) points which probably correspond to the same object
void removeInsidePoints( vector<Candidate*>& input, vector<Candidate*>& output, int method );
void removeInsidePoints( vector<Candidate*>& input, vector<Candidate*>& output );

// Load a new classifier system based on the loaded classifier config file
ClassifierSystem* loadClassifiers( const SystemSettings& sparams, ClassifierConfigParameters& cparams );

// Perform suppression and interpolation
vector<Detection*> interpolateResults( vector<Candidate*>& input, ClassifierSystem* Classifiers, std::string Filename );

// Deallocate Detections
void deallocateDetections( vector<Detection*>& vec );

// Append MIP training results to some file
bool appendInfoToFile( vector<Detection>& Detections, const string& list_fn );
	
// Append final Detections to some file
bool appendInfoToFile( vector<Detection*>& cds, const string& ListFilename, const string& this_fn, float resize_factor );

#endif
