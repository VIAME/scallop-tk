
#ifndef SCALLOP_TK_IMAGE_PROPERTIES_H_
#define SCALLOP_TK_IMAGE_PROPERTIES_H_

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Standard C/C++
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"

//------------------------------------------------------------------------------
//                             Class Declaration
//------------------------------------------------------------------------------

// Estimates various image properties (GSD, Scale) from metadata.
class ImageProperties {

public:

  // Constructors/Deconstructors -
  ImageProperties() { isValid = false; }

  // Constructor to attempt and load metadata from file
  void calculateImageProperties( const std::string& fn, const int& cols,
    const int& rows, const float& focalLength );

  // Constructor to get metadata from external source
  void calculateImageProperties( const int& cols, const int& rows,
    const float& metaaltitude, const float& metapitch, const float& metaroll,
    const float& focal, float metaheading = 0.0 );

  // Generate fake metadata, assuming 1 meter per pixel scale
  void calculateImageProperties( const int& cols, const int& rows );

  // Destructor
  ~ImageProperties() {}

  // Accessors
  bool hasMetadata() { return isValid; }
  float getImgHeightMeters() { return avgHeight; }
  float getImgWidthMeters() { return avgWidth; }
  float getAvgPixelSizeMeters() { return avgPixelSize; }

private:

  // Internal helper functions
  bool loadMetadata( const float& focal );
  void calculateProperties();
  int getImageType();

  // Image Filename
  std::string filename;
  bool isValid;
  int imageType;

  // Stored Resolution
  int imgRows;
  int imgCols;

  // Metadata contents
  float heading;
  float pitch;
  float roll;
  float altitude;
  float focalLength;
  float depth;

  // Processed metadata variables
  float estArea;
  float avgHeight;
  float avgWidth;
  float pixelHeight;
  float pixelWidth;
  float avgPixelSize;
};

#endif
