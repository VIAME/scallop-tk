//------------------------------------------------------------------------------
// Title: Definitions.h
//
// Description: This file contains declarations, constants, and shared structs
// used across many other operations within this toolkit
//------------------------------------------------------------------------------

#ifndef SCALLOP_TK_DEFINITIONS_H_
#define SCALLOP_TK_DEFINITIONS_H_

// C++ Includes
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <limits>

// OpenCV Includes
#include <cv.h>

namespace ScallopTK
{

//------------------------------------------------------------------------------
//                         Common Defines and Macros
//------------------------------------------------------------------------------

// Convert an integer to a string
#define INT_2_STR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

//------------------------------------------------------------------------------
//                                Constants
//------------------------------------------------------------------------------

// Mathematical Constants
const float PI  = 3.14159265f;
const float INF = 1E+37f;

// Default search directories for config files for binaries
const std::string CONFIG_SEARCH_DIR1 = "/Models/";
const std::string CONFIG_SEARCH_DIR2 = "/scallop_tk_models/";

const std::string DEFAULT_COLORBANK_DIR = "ColorFilterBanks/";
const std::string DEFAULT_CLASSIFIER_DIR = "Classifiers/";
const std::string DEFAULT_CONFIG_FILE = "SYSTEM_SETTINGS";
const std::string DEFAULT_COLORBANK_EXT = "_32f_rgb_v1.cfilt";

// Max search depth for reading metadata contained within JPEG files
const int MAX_META_SEARCH_DEPTH = 10000;

// Input image type definitions
const int UNKNOWN  = 0x00;   //.???
const int JPEG     = 0x01;   //.jpg || .JPG
const int RAW_TIF  = 0x02;   //.tif || .TIF
const int RAW_TIFF = 0x03;   //.tiff
const int BMP      = 0x04;   //.bmp
const int PNG      = 0x05;   //.png

// Scallop Display Window Name
const std::string DISPLAY_WINDOW_NAME = "ScallopDisplayWindow";

// Desired maximum pixel count covering the min object search radius,
// this is an optimization which speeds up all operations by performing
// a downscale of the input image, if possible.
const float MAX_PIXELS_FOR_MIN_RAD = 10.0;

// A downsizing image resize factor must be lower than x% to validate
// and actually perform the resize for computational or accuracy benefits
// (otherwise it doesn't really pay). Note, this can also be greater than
// 1 for some problems.
const float RESIZE_FACTOR_REQUIRED = 0.975f;

// Default image scale factors for assorted operations, this is applied
// on top of any initial image filtering resize optimizations. Units are
// relative measure as to the number of pixels minimum object search
// radius should take up.
const float MPFMR_COLOR_CLASS = 1.0 * MAX_PIXELS_FOR_MIN_RAD;
const float MPFMR_COLOR_DOG   = 2.0 * MAX_PIXELS_FOR_MIN_RAD;
const float MPFMR_TEMPLATE    = 0.5 * MAX_PIXELS_FOR_MIN_RAD;
const float MPFMR_ADAPTIVE    = 1.0 * MAX_PIXELS_FOR_MIN_RAD;
const float MPFMR_WATERSHED   = 1.0 * MAX_PIXELS_FOR_MIN_RAD;
const float MPFMR_CLUST       = 1.0 * MAX_PIXELS_FOR_MIN_RAD;
const float MPFMR_TEXTONS     = 1.0 * MAX_PIXELS_FOR_MIN_RAD;
const float MPFMR_HOG         = 1.0 * MAX_PIXELS_FOR_MIN_RAD;

// Properties for gradient calculations
const float LAB_GRAD_SIGMA     = 1.35f;
const float ENV_GRAD_SIGMA     = 1.85f;

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

// Special type definitions for input classification files
const std::string BACKGROUND     = "BACKGROUND";
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

// Default internally specified tags for different species
// types. The classifier system file can manually specify
// other tags to put in output files, etc.
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

// Detection methods for prioritization
const unsigned int TEMPLATE  = 0;
const unsigned int DOG       = 1;
const unsigned int ADAPTIVE  = 2;
const unsigned int CANNY     = 3;
const unsigned int MULTIPLE1 = 4;
const unsigned int MULTIPLE2 = 5;
const unsigned int MULTIPLE3 = 6;
const unsigned int TOTAL_DM  = 7;

// Feature size-related constants
const unsigned int COLOR_BINS     = 32;
const unsigned int COLOR_FEATURES = 122;
const unsigned int GABOR_FEATURES = 36;
const unsigned int SIZE_FEATURES  = 9;
const unsigned int EDGE_FEATURES  = 137;
const unsigned int HOG_FEATURES   = 1764;
const unsigned int NUM_HOG        = 2;

// Amount to expand bounding box around candidate by when
// computing image chips to feed into a CNN classifier.
const float CNN_EXPANSION_RATIO = 1.45f;

// Sand dollar suppression system factor
const double SDSS_DIFFERENCE_FACTOR = 2.0;

// Maximum number of classifiers in a single classification system
const unsigned MAX_CLASSIFIERS = 30;

//------------------------------------------------------------------------------
//          System Parameters - Set at Run-Time From Config Files
//------------------------------------------------------------------------------

struct SystemParameters
{
  // Input directory containing input images, if specified
  std::string InputDirectory;

  // Input filename, if we're processing a list or using metadata from a list
  std::string InputFilename;

  // Output directory to dump annotated images into
  std::string OutputDirectory;

  // Output filename for detection list or training data in the above folder
  std::string OutputFilename;

  // Are input images coming from a directory or a filelist
  bool IsInputDirectory;

  // Root directory containing all config files and models
  std::string RootConfigDIR;

  // Root directory for color filters
  std::string RootColorDIR;

  // Root directory for classifiers
  std::string RootClassifierDIR;

  // Are we using image metadata at all during processing?
  bool UseMetadata;

  // Is metadata stored in the image or the file list?
  bool IsMetadataInImage;

  // Only process the left half of the input image
  bool ProcessLeftHalfOnly;

  // The main classifier ID to use
  std::string ClassifierToUse;

  // Minimum search radius in meters (Used if metadata is available)
  float MinSearchRadiusMeters;

  // Maximum search radius in meters (used if metadata is available)
  float MaxSearchRadiusMeters;

  // Minimum search radius in meters (Used if metadata is not available)
  float MinSearchRadiusPixels;

  // Maximum search radius in meters (used if metadata is not available)
  float MaxSearchRadiusPixels;

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
  bool EnableSDSS;

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

  // Type of annotation
  enum{ POINT, LINE, BOX } Type;

  // X1, Y1
  double X1, Y1;

  // X2, Y2
  double X2, Y2;
};

typedef std::vector< GTEntry > GTEntryList;

//------------------------------------------------------------------------------
//                        Simple Contour Definition
//------------------------------------------------------------------------------

struct Point2D
{
  Point2D( const int& _r, const int& _c ) : r(_r), c(_c) {}
  int r, c;
};

struct Contour
{
  int label;                  // Contour Identifier
  float mag;                  // Contour Magnitude (Confidence)
  bool coversOct[8];          // Contour Octant Coverage around IP center
  std::vector<Point2D> pts;   // Vector of points comprising Contour
};

//------------------------------------------------------------------------------
//                         Interest Point Definition
//------------------------------------------------------------------------------

// Candidate Point (Object Proposal) and associated information
//
// This object stores location, stats for classification, features extracted
// around the candidate location, and preliminary classification results for
// the candidate.
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
  unsigned int methodRank;

  // Is the Candidate a corner entry
  bool isCorner; //is the Candidate on an image boundary
  bool isSideBorder[8]; // which octants are outside the image

  // Features for classification
  bool isActive;
  double colorFeatures[COLOR_FEATURES];
  double gaborFeatures[GABOR_FEATURES];
  double sizeFeatures[SIZE_FEATURES];
  CvMat *hogResults[NUM_HOG];
  double majorAxisMeters;

  // Used for color detectors
  IplImage *summaryImage;
  IplImage *colorQuadrants;
  int colorQR, colorQC;
  int colorBinCount[COLOR_BINS];

  // Edge Based Features
  bool hasEdgeFeatures;
  double edgeFeatures[EDGE_FEATURES];

  // Expensive edge search results
  float innerColorAvg[3];
  float outerColorAvg[3];
  Contour *bestContour;
  Contour *fullContour;

  // User entered designation, Candidate filename if in training mode
  int designation;
  std::string filename;

  // Stats for final classification
  unsigned int classification;
  double classMagnitudes[MAX_CLASSIFIERS];

  // Default constructor
  Candidate()
  : summaryImage( NULL ),
    colorQuadrants( NULL ),
    bestContour( NULL ),
    fullContour( NULL )
  {
    for( unsigned i = 0; i < NUM_HOG; i++ )
    {
      hogResults[i] = NULL;
    }

    for( unsigned i = 0; i < MAX_CLASSIFIERS; i++ )
    {
      classMagnitudes[i] = -std::numeric_limits<double>::max();
    }
  }
};

// An actual Detection according to our algorithm, used for output and
// post processing. Contains less information than a candidate.
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
  Contour cntr;

  // Possible Object IDs and classification Detection values
  std::vector< std::string > classIDs;
  std::vector< double > classProbabilities;

  // Does the best match fall into any of these categories?
  bool isBrownScallop;
  bool isWhiteScallop;
  bool isBuriedScallop;
  bool isSandDollar;
};

typedef std::vector<Candidate*> CandidatePtrVector;
typedef std::vector<Detection*> DetectionPtrVector;

typedef std::vector<Candidate> CandidateVector;
typedef std::vector<Detection> DetectionVector;

}

#endif
