//------------------------------------------------------------------------------
// Title: 
// Author:
// Description: 
//------------------------------------------------------------------------------

#ifndef SCALLOP_TK_CONSOLIDATOR_H_
#define SCALLOP_TK_CONSOLIDATOR_H_

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
  bool operator() ( const Candidate* cd1, const Candidate* cd2 ) {
    if( cd1->method < cd2->method )
      return true;
    if( cd1->method == cd2->method && cd1->magnitude > cd2->magnitude )
      return true;
    return false;
  }
};

class CompareRank {
public:
  bool operator() ( const Candidate* cd1, const Candidate* cd2 ) {
    if( cd1->methodRank > cd2->methodRank )
      return true;
    return false;
  }
};

typedef priority_queue<Candidate*, vector<Candidate*>, CompareRank> CandidateQueue;
typedef vector<Candidate*> CandidateVector;
typedef vector<Detection*> DetectionVector;

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
