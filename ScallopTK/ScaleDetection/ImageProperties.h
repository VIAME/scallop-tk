
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

//OpenCV Includes
#include "cv.h"
#include "cxcore.h"

//Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"
#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/TPL/Homography/ScottCamera.h"

//------------------------------------------------------------------------------
//                             Class Declaration
//------------------------------------------------------------------------------

// Estimates various image properties (GSD, Scale) from metadata.
class ImageProperties {

public:

	// Constructors/Deconstructors -
	ImageProperties() { is_valid = false; }

	// Constructor to attempt and load metadata from file
	void CalculateImageProperties( const std::string& fn, const int& cols, const int& rows, const float& focal_length );

	// Constructor to query user for metadata
	void CalculateImageProperties( const int& cols, const int& rows );

	// Constructor to get metadata from external source
	void CalculateImageProperties( const int& cols,
					               const int& rows,
					               const float& metaaltitude,
					               const float& metapitch,
					               const float& metaroll,
                         const float& focal,
					               float metaheading = 0.0 );

	// Destructor
	~ImageProperties() {}

	// Accessors
	bool hasMetadata() { return is_valid; }
	float getMaxScallopSize() { return maxScallopRadius; }
	float getMinScallopSize() { return minScallopRadius; }
	float getImgHeightMeters() { return avgHeight; }
	float getImgWidthMeters() { return avgWidth; }
	float getAvgPixelSizeMeters() { return avgPixelSize; }

	// Show the min and max scallop size superimposed on the image 'img'
	void showScallopMinMax( IplImage* img, float resizeFactor );
	
private:

	//Internal helper functions
	bool loadMetadata( const float& focal );
	void calculateProperties();
	int getImageType();

	//Image Filename
	std::string filename;
	bool is_valid;
	int imageType;

	//Stored Resolution
	int imgRows;
	int imgCols;

	//Metadata contents
	float heading;
	float pitch;
	float roll;
	float altitude;
	float focal_length;
	float depth;

	//Processed metadata variables
	float estArea;
	float avgHeight;
	float avgWidth;
	float pixelHeight;
	float pixelWidth;
	float avgPixelSize;

	//What size should scallops appear?
	float minScallopRadius;
	float maxScallopRadius; 
};

#endif
