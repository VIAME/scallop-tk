#ifndef SCALLOP_TK_CORE_DETECTOR_H_
#define SCALLOP_TK_CORE_DETECTOR_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

// OpenCV
#include <cv.h>

// Internal Definitions
#include "ScallopTK/Utilities/Definitions.h"

//------------------------------------------------------------------------------
//                                 Prototypes
//------------------------------------------------------------------------------

namespace ScallopTK
{

// Standalone detection function, returns total number of detections
//
// This function is called by the main ScallopDetector tool. It is
// capable of processing frames in a directory or a filelist. It
// also contains the main model training subroutine option.
int runCoreDetector( const SystemParameters& settings );

// Streaming class definition, for use by external programs
//
// This function should be called if an external library wants
// to run a pre-trained detector on arbitrary input frames.
// Note: training mode cannot be run in this configuration.
class CoreDetector
{
public:

  // Construct a detector given the location of a config file
  //
  // Throws runtime_error exception on failure to load models
  explicit CoreDetector( const std::string& configFile );

  // Construct a detector given a configuration class
  //
  // Throws runtime_error exception on failure to load models
  explicit CoreDetector( const SystemParameters& settings );

  // Destructor
  ~CoreDetector();

  // Process a new frame given an image
  //
  // Throws runtime_error exception on failure
  std::vector< Detection > processFrame( const cv::Mat& image );

  // Process a new frame given the location of an image
  //
  // Throws runtime_error exception on failure
  std::vector< Detection > processFrame( std::string filename );

private:

  // Class for storing all cross-frame required data
  class Priv;
  Priv* data;
};

}

#endif
