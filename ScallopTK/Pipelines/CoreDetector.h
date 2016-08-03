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

// Standalone detection function
int runDetector( const SystemSettings& settings );

// Streaming class definition, for use by external libraries
class CoreDetector
{
public:

  // Construct a detector given the location of a config file
  explicit CoreDetector( std::string configFile );

  // Construct a detector given a configuration class
  explicit CoreDetector( const SystemSettings& settings );

  // Destructor
  ~CoreDetector();

  // Process a new frame given an image
  std::vector< Detection > processFrame( const cv::Mat& image );

  // Process a new frame given the location of an image
  std::vector< Detection > processFrame( std::string filename );

private:

  // Class for storing all cross-frame required data
  class priv;
  priv* data;
};

}

#endif
