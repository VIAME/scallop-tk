//------------------------------------------------------------------------------
// Title: Definitions.h
// Description: This file contains declarations, constants, and shared structs
// used across many other operations within this toolkit
//------------------------------------------------------------------------------

#ifndef SCALLOP_TK_DEFINITIONS_H_
#define SCALLOP_TK_DEFINITIONS_H_

// C++ Includes
#include <string>
#include <vector>
#include <map>

// OpenCV Includes
#include <cv.h>

//------------------------------------------------------------------------------
//                           Compile/Build Parameters
//------------------------------------------------------------------------------

// Enables automatic reading of metadata from image file
//  - For any HabCam data this should be on -
#define AUTO_READ_METADATA 1

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------

// Mathematical Constants
const float PI  = 3.14159265f;
const float INF = 1E+37f;

// Max search depth for reading metadata contained within JPEG files
const int MAX_META_SEARCH_DEPTH = 10000;

// Input image type definitions
const int UNKNOWN   = 0x00;   //.???
const int JPEG      = 0x01;   //.jpg || .JPG
const int RAW_TIF   = 0x02;   //.tif || .TIF
const int RAW_TIFF  = 0x03;   //.tiff
const int BMP       = 0x04;   //.bmp
const int PNG       = 0x05;   //.png

// Default image scale factors for assorted operations, this is applied
// on top of any initial image filtering resize optimizations. Units are
// relative measure w.r.t. base image size.
const float OSF_COLOR_CLASS = 1.0;
const float OSF_COLOR_DOG   = 2.0;
const float OSF_TEMPLATE    = 1.0;
const float OSF_ADAPTIVE    = 1.0;
const float OSF_WATERSHED   = 1.0;
const float OSF_CLUST       = 1.0;
const float OSF_TEXTONS     = 1.0;
const float OSF_HOG         = 1.0;

// A downsizing image resize factor must be lower than 95% to validate
// and actually perform the resize for computational benefit (otherwise
// it doesn't pay)
const float RESIZE_FACTOR_REQUIRED = 0.95f;

// Histogram Update Propterties for Fast Read/Merge
// These are approximates which don't mean that much.
// -Approximate # of pixels to scan
const float MAX_ESTIM_TO_SCAN   = 60000;
// -Estimated ratio in # of environment/object pixels
const float OBJ_ENVI_RATIO      = 0.1f;
// -Default # of pixels to skip after hitting an env pixel
const int DEFAULT_ENVI_SKIP     = 5;
// -Default # of pixels to skip after hitting an obj pixel
const int DEFAULT_OBJ_SKIP      = 2;
// -Default merge ratio for syncing old w/ new histogram
const float DEFAULT_MERGE_RATIO = 0.08f;

// Properties for gradient calculations
const float LAB_GRAD_SIGMA     = 1.35f;
const float ENV_GRAD_SIGMA     = 1.85f;

// Special type definitions for input classification files
const std::string BROWN_SCALLOP  = "BROWN";
const std::string WHITE_SCALLOP  = "WHITE";
const std::string BURIED_SCALLOP = "BURIED";
const std::string ALL_SCALLOP    = "ALL";
const std::string SAND_DOLLAR    = "DOLLARS";

// Suppression type definitions for input classification files
const std::string WORLD_VS_OBJ_STR   = "WVO";
const std::string DESIRED_VS_OBJ_STR = "OVO";
const std::string MIXED_CLASS_STR    = "MIX";

// Classifier types
const unsigned int MAIN_CLASS     = 0x00;
const unsigned int WORLD_VS_OBJ   = 0x01;
const unsigned int DESIRED_VS_OBJ = 0x02;
const unsigned int MIXED_CLASS    = 0x03;

// Internal tag designations for objects in mask
//  - Why isn't this an enum? -
typedef unsigned int tag;

const tag UNCLASSIFIED       = 0x00;
const tag ENVIRONMENT        = 0x01;
const tag SCALLOP_WHITE      = 0x02;
const tag SCALLOP_BROWN      = 0x03;
const tag SCALLOP_BURIED     = 0x04;
const tag DOLLAR             = 0x05;
const tag CLAM               = 0x06;
const tag URCHIN             = 0x07;
const tag SAC                = 0x08;
const tag ROCK               = 0x09;
const tag OTHER              = 0x10;
const tag TOTAL_DESIG        = 0x11;

// Detection methods for prioritzation
const unsigned int TEMPLATE  = 0;
const unsigned int DOG       = 1;
const unsigned int ADAPTIVE  = 2;
const unsigned int CANNY     = 3;
const unsigned int MULTIPLE1 = 4;
const unsigned int MULTIPLE2 = 5;
const unsigned int MULTIPLE3 = 6;
const unsigned int TOTAL_DM  = 7;

// Feature constants
const unsigned int COLOR_BINS     = 32;
const unsigned int COLOR_FEATURES = 122;
const unsigned int GABOR_FEATURES = 36;
const unsigned int SIZE_FEATURES  = 9;
const unsigned int EDGE_FEATURES  = 137;
const unsigned int NUM_HOG        = 2;

// Sand Dollar Suppression Sys value
const double SDSS_DIFFERENCE_FACTOR = 2.0;

// Maximum number of classifiers in a single classification system
const int MAX_CLASSIFIERS = 30;

//------------------------------------------------------------------------------
//                     System Parameters - Set at Run-Time
//------------------------------------------------------------------------------

struct SystemParameters
{
  // Input directory, if specified
  std::string InputDirectory;
  
  // Input filename, if we're processing a list or using metadata from a list
  std::string InputFilename;
  
  // Output directory to dump annotated images into
  std::string OutputDirectory;
  
  // Output filename for detection list or training data in the above folder
  std::string OutputFilename;

  // Are input images coming from a directory or a filelist
  bool IsInputDirectory;

  // Root directory for color filters
  std::string RootColorDIR;
  
  // Root directory for classifiers
  std::string RootClassifierDIR;

  // Are we using image metadata at all during processing?
  bool UseMetadata;

  // Is metadata stored in the image or the file list?
  bool IsMetadataInImage;

  // Are we in training mode or process (testing) mode?
  bool IsTrainingMode;

  // If we are in training mode, are we getting annotations from a file,
  // or should the internal GUI be enabled for annotations?
  bool UseFileForTraining;

  // If training from files, the percentage of false points outside of
  // annotations to use as false examples.
  float TrainingPercentKeep;

  // Should we consider classifying object proposals near image boundaries?
  bool LookAtBorderPoints;

  // The classifier ID to use
  std::string ClassifierToUse;
  
  // Enable output GUI display?
  bool EnableOutputDisplay;
  
  // Output Detection list?
  bool OutputList;
  
  // If an IP falls into more than one category, output it multiple times?
  bool OutputDuplicateClass;
  
  // Output images with proposals in output directory?
  bool OutputProposalImages;
  
  // Output images with detections in output directory?
  bool OutputDetectionImages;

  // Camera focal length, if known
  float FocalLength;

  // Number of worker threads to allocate for processing images
  int NumThreads;
};


//------------------------------------------------------------------------------
//          Classifier Parameters - As read from some config file
//------------------------------------------------------------------------------

struct ClassifierParameters
{
  // The subdirectory for all classifier files within the root folder
  std::string ClassifierSubdir;

  // Is this classifier subsystem using AdaBoost or CNN classifiers?
  bool UseCNNClassifier;

  // Information for the primary classifiers
  std::vector< std::string > L1Keys;
  std::vector< std::string > L1Files;
  std::vector< std::string > L1SpecTypes;

  // Information for the secondary classifiers
  std::vector< std::string > L2Keys;
  std::vector< std::string > L2Files;
  std::vector< std::string > L2SpecTypes;
  std::vector< std::string > L2SuppTypes;

  // Is sand dollar suppression turned on?
  bool EnabledSDSS;

  // Classifier threshold
  double Threshold;
};


//------------------------------------------------------------------------------
//              Required Parameters Read From Some GT file
//------------------------------------------------------------------------------

struct GTEntry
{
  // Image name where object is
  std::string Name;
  
  // ID code of object
  int ID;
  
  // X1, Y1
  double X1, Y1;
  
  // X2, Y2
  double X2, Y2;
};

typedef std::vector< GTEntry > GTEntryList;

//------------------------------------------------------------------------------
//                         Simple Contour Definition
//------------------------------------------------------------------------------

struct ScanPoint {
  ScanPoint( const int& _r, const int& _c ) : r(_r), c(_c) {}
  int r, c;
};

struct Contour {
  float mag; // Contour Magnitude
  int label; // Contour Label in Binary Image
  bool covers_oct[8]; // Contour octant coverage around an IP center
  std::vector<ScanPoint> pts; // Vector of points comprising Contour
};

//------------------------------------------------------------------------------
//                          Interest Point Definition
//------------------------------------------------------------------------------

// Candidate Point
// Stores location, stats for classification, and classification results         
struct Candidate
{

  // Initial ellipse Location
  // (Detected with low-resolution IP detectors)
  double r;
  double c;
  double major;
  double minor;
  double angle;

  // Revised ellipse location
  // (Calculated from edge feature cost function)
  double nr;
  double nc;
  double nmajor;
  double nminor;
  double nangle;

  // IP Detection Method and Magnitude of said Method
  unsigned int method;
  double magnitude;
  unsigned int method_rank;

  // Is the Candidate a corner entry
  bool is_corner; //is the Candidate on an image boundary
  bool is_side_border[8]; // which octants are outside the image

  // Features for classification  
  bool is_active;
  double ColorFeatures[COLOR_FEATURES];
  double GaborFeatures[GABOR_FEATURES];
  double SizeFeatures[SIZE_FEATURES];
  CvMat *HoGResult[NUM_HOG];
  double major_meters;

  // Used for color detectors
  IplImage *SummaryImage;
  IplImage *ColorQuadrants;
  int ColorQR, ColorQC;
  int ColorBinCount[COLOR_BINS];

  // Edge Based Features
  bool has_edge_features;
  double EdgeFeatures[EDGE_FEATURES];

  // Expensive edge search results
  float innerColorAvg[3];
  float outerColorAvg[3];
  Contour *best_cntr;
  Contour *full_cntr;

  // User entered designation, Candidate filename if in training mode
  int designation;
  std::string filename;

  // Stats for final classification
  unsigned int classification;
  double class_magnitudes[MAX_CLASSIFIERS];
};

// An actual Detection according to our algorithm, used just for post processing
struct Detection
{
  // Image name
  std::string img;
  
  // Object Location
  double r;
  double c;
  double major;
  double minor;
  double angle;

  // Object Contour (if it exists)
  Contour *cntr;

  // Possible Object IDs and classification Detection values
  std::vector< std::string > IDs;
  std::vector< double > ClassificationValues;
  
  // Does the best match fall into any of these categories?
  bool IsBrownScallop;
  bool IsWhiteScallop;
  bool IsBuriedScallop;
  bool IsDollar;
};

#endif
