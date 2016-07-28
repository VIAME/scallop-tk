//------------------------------------------------------------------------------
// Title: 
// Author:
// Description: 
//------------------------------------------------------------------------------

#ifndef CONSOLIDATOR_H_
#define CONSOLIDATOR_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <fstream>
#include <queue>

//OpenCV 2.1
#include "cv.h"
#include "cxcore.h"

//Scallop Includes
#include "ScallopTK/TPL/KDTree/kdtree.h"
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/ObjectProposals/PriorStatistics.h"

//------------------------------------------------------------------------------
//                                Definitions
//------------------------------------------------------------------------------

class CompareCandidates {
public:
	bool operator() ( const candidate* cd1, const candidate* cd2 ) {
		if( cd1->method < cd2->method )
			return true;
		if( cd1->method == cd2->method && cd1->magnitude > cd2->magnitude )
			return true;
		return false;
	}
};

class CompareRank {
public:
	bool operator() ( const candidate* cd1, const candidate* cd2 ) {
		if( cd1->method_rank > cd2->method_rank )
			return true;
		return false;
	}
};

typedef priority_queue<candidate*, vector<candidate*>, CompareRank> CandidateQueue;
typedef vector<candidate*> CandidateVector;
typedef vector<detection*> DetectionVector;

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void prioritizeCandidates( CandidateVector& Blob, 
						   CandidateVector& Adaptive,
						   CandidateVector& Template,
						   CandidateVector& Canny,
						   CandidateVector& Unordered,
						   CandidateQueue& Ordered,
						   ThreadStatistics *GS );

#endif
