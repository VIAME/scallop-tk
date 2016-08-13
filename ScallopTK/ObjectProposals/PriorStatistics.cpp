//------------------------------------------------------------------------------
// Title: Template2.cpp
// Author: Matthew Dawkins
// Description: 
//------------------------------------------------------------------------------

#include "PriorStatistics.h"

//------------------------------------------------------------------------------
//                            Function Definitions
//------------------------------------------------------------------------------

ThreadStatistics::ThreadStatistics() {

  // Calculate resize min - todo: binary merge next time n.t.i.m.
  global_min_rad = max( MPFMR_COLOR_DOG, MPFMR_TEMPLATE );
  if( global_min_rad < MPFMR_COLOR_CLASS ) 
    global_min_rad = MPFMR_COLOR_CLASS;
  if( global_min_rad < MPFMR_ADAPTIVE )
    global_min_rad = MPFMR_ADAPTIVE;
  if( global_min_rad < MPFMR_WATERSHED )
    global_min_rad = MPFMR_WATERSHED;
  if( global_min_rad < MPFMR_CLUST )
    global_min_rad = MPFMR_CLUST;
  if( global_min_rad < MPFMR_TEXTONS )
    global_min_rad = MPFMR_TEXTONS;
  if( global_min_rad < MPFMR_HOG )
    global_min_rad = MPFMR_HOG;

  // 0 inits
  ScallopDensity = 0.0f;
  ClamDensity = 0.0f;
  SandDollarDensity = 0.0f;
  UrchinDensity = 0.0f;
  ScallopDensityPrev = 0.0f;
  ClamDensityPrev = 0.0f;
  SandDollarDensityPrev = 0.0f;
  UrchinDensityPrev = 0.0f;
  processed = 0;

  // Detection mat
  for( unsigned int i = 0; i < TOTAL_DESIG; i++ ) {
    thread_Detections[ i ] = 0;
  }
}

void ThreadStatistics::Update( int Detections[], float im_area ) {
  
  // Calculate Density Stats
  float tot_scal = Detections[SCALLOP_BROWN]+Detections[SCALLOP_WHITE]+Detections[SCALLOP_BURIED];
  ScallopDensityPrev = tot_scal / im_area;
  ClamDensityPrev = (float)Detections[CLAM] / im_area;
  SandDollarDensityPrev = (float)Detections[DOLLAR] / im_area;
  UrchinDensityPrev = (float)Detections[URCHIN] / im_area;

  // Update Avg Skewed Densities
  if( processed < 100 ) {
    ScallopDensity = (ScallopDensity * processed + ScallopDensityPrev) / (processed+1);
    ClamDensity = (ClamDensity * processed + ClamDensityPrev) / (processed+1);
    SandDollarDensity = (SandDollarDensity * processed + SandDollarDensityPrev) / (processed+1);
    UrchinDensity = (UrchinDensity * processed + UrchinDensityPrev) / (processed+1);
  } else {
    ScallopDensity = (ScallopDensity * 0.98 + ScallopDensityPrev *0.02);
    ClamDensity = (ClamDensity * 0.98 + ClamDensityPrev*0.02);
    SandDollarDensity = (SandDollarDensity * 0.98 + SandDollarDensityPrev*0.02);
    UrchinDensity = (UrchinDensity * 0.98 + UrchinDensityPrev*0.02);
  }

  // Increase image processed counters
  processed++;

  // Add Detections
  for( unsigned int i=0; i < TOTAL_DESIG; i++ ) {
    thread_Detections[ i ] += Detections[ i ];
  }
}

