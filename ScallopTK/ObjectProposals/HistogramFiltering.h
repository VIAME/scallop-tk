#ifndef SCALLOP_TK_HISTOGRAM_FILTERING_H_
#define SCALLOP_TK_HISTOGRAM_FILTERING_H_

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

//Opencv
#include <cv.h>
#include <cxcore.h>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/ObjectProposals/DoG.h"

namespace ScallopTK
{

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------

//Checksum for reading in filter files from matlab
const int CHECKSUM = 999999;

//Header size devoted in each filter matlab filter file
const int HEADER_BYTES = 48;

//------------------------------------------------------------------------------
//  Struct to contain a particular color classification result for some image
//------------------------------------------------------------------------------

struct hfResults {
  float scale;
  float minRad;
  float maxRad;
  IplImage *BrownScallopClass;
  IplImage *WhiteScallopClass;
  IplImage *SandDollarsClass;
  IplImage *EnvironmentalClass;
  IplImage *NetScallops;
  IplImage *SaliencyMap;
  IplImage *EnvironmentMap;
};

//------------------------------------------------------------------------------
//                         Single Filter Class Prototype
//------------------------------------------------------------------------------

// Class to construct a histogram-based classifier
class hfFilter {
public:
  // Declares a new filter
  hfFilter() { filterLoaded = false; filter3d = NULL; secondary3d = NULL; };
  ~hfFilter();

  // Loads a filter from a file in the default filter directory
  // A secondary buffer should be used if you want to modify the filter, by
  // first inserting values into the secondary buffer, and then merging that
  // buffer into the primary
  bool loadFromFile( const string& filter_fn, bool allocSecondary = false );

  // Returns true if a valid filter is loaded
  bool isValid() { return filterLoaded; }

  // Classifies a single value
  float classifyPoint3d( float* pt );

  // Classifies an entire 3-chan image
  IplImage *classify3dImage( IplImage *img );

  // Sets the secondary buffer to 0
  void flushSecondary();

  // Inserts value into secondary
  int insertInSecondary( float *val );

  // Merges secondary into filter
  void mergeSecondary( float ratio, float secondaryDown );

private:

  // Buffer to hold color filter
  float *filter3d;

  // Buffer to hold secondary filter
  //  The secondary filter is a double buffer, it exists if we 
  //  want to merge the secondary into the primary filter [TODO Better descr]
  float *secondary3d;

  // Was the filter successfully loaded?
  bool filterLoaded;

  // The start of the range that this histogram covers
  float startCh1;
  float startCh2;
  float startCh3;

  // The end of the range that this histogram covers
  float endCh1;
  float endCh2;
  float endCh3;

  // The range of each channel that the histogram covers
  float bprCh1; // Bins per range (end-start)
  float bprCh2;
  float bprCh3;

  // The number of bins per each channel of the histogram
  int histBinsCh1;
  int histBinsCh2;
  int histBinsCh3;

  // The number of channels (deprecated, always should be 3)
  int numChan;

  // Histogram steps
  int ch1_scale;
  int ch2_scale;

  // Total # of floats in histogram
  int size;
};

//------------------------------------------------------------------------------
//                      Saliency Filter Class Prototype
//------------------------------------------------------------------------------

// Class to construct our filter and perform classifications with it
// Only RGB support [0,1] range, very similar to the above
class salFilter {
public:
  // Declares a new filter
  salFilter() { isValid = false; size = 0; filter3d = NULL; }
  ~salFilter();

  // Classifies an entire image
  IplImage *classify3dImage( IplImage *img );

  // Reset the histogram
  void flushFilter();

  // Smooth the histogram
  void smoothHist();

  // Builds the saliency map
  void allocMap( int binsPerDim );
  void buildMap( IplImage *img );

private:

  // Buffer to hold color filter
  float *filter3d;

  // Is the map valid
  bool isValid;

  // The number of bins per each channel of the histogram
  int histBinsCh1;
  int histBinsCh2;
  int histBinsCh3;

  // Histogram steps
  int ch1_scale;
  int ch2_scale;

  // Total # of floats in histogram
  int size;
};

//------------------------------------------------------------------------------
//                        Multi Filter Class Prototype
//------------------------------------------------------------------------------

// Class to encapsulate all histogram-based classifiers for all operations
class ColorClassifier {

public:

  // Class Constructor
  ColorClassifier() { filtersLoaded=false; }

  // Class Destructr
  ~ColorClassifier() {}

  // Loads a group of filters from the default directory given a seedname
  bool loadFilters( const string& dir, const string& seedname );

  // Returns true if valid filters have been loaded
  bool isValid() { return filtersLoaded; }

  // Performs all required histogram-based filtering of image
  hfResults *classifiyImage( IplImage *img );

  // Calls classifyImage after resizing/smoothing image
  hfResults *performColorClassification( IplImage* img, float minRad, float maxRad );

  // Updates all of the filters after interest points have been classified
  void Update( IplImage *img, IplImage *mask, int Detections[] );
  
private:

  // Last classifier results for last image (managed externally)
  IplImage *img;
  hfResults *res;

  // True if filters successfully loaded
  bool filtersLoaded;

  // Classifiers for each type
  hfFilter WhiteScallop;
  hfFilter BrownScallop;
  hfFilter Clam;
  hfFilter Environment;
  hfFilter SandDollars;
  salFilter SaliencyModel;
};

//------------------------------------------------------------------------------
//                            Function Prototypes
//------------------------------------------------------------------------------

// Deallocates a results struct
void hfDeallocResults( hfResults* res );

// Detects blobs in our color-classification results
void detectColoredBlobs( hfResults* color, CandidatePtrVector& cds );
void detectSalientBlobs( hfResults* color, CandidatePtrVector& cds );

// Quickly approximates the percentiles p1 and p2 in the single chan 32f image img
void quickPercentiles( IplImage* img, float p1, float p2, float &op1, float& op2 );

}

#endif
