
#include "ImageProperties.h"

#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/TPL/Homography/ScottCamera.h"

using namespace std;

//------------------------------------------------------------------------------
//                            Function Definitions
//------------------------------------------------------------------------------

// Loads metadata from file fn
void ImageProperties::calculateImageProperties( const std::string& fn,
  const int& cols, const int& rows, const float& focalLength ) {
  filename = fn;
  imgRows = rows;
  imgCols = cols;
  imageType = getImageType();
  isValid = loadMetadata( focalLength);
  if( isValid )
    calculateProperties();
}

void ImageProperties::calculateImageProperties( const int& cols,
  const int& rows, const float& metaaltitude, const float& metapitch,
  const float& metaroll, const float& focal, float metaheading )
{
  imgRows = rows;
  imgCols = cols;
  heading = metaheading;
  pitch = metapitch;
  roll = metaroll;
  altitude = metaaltitude;
  focalLength = focal;
  isValid = true;

  //Adjust for special circumstances (unreported data)
  if( heading == -999.99f )
    heading = 0.0f;
  if( pitch == -999.99f )
    pitch = 0.0f;
  if( roll == -999.99f )
    roll = 0.0f;

  calculateProperties();
}

// Queries user for metadata
void ImageProperties::calculateImageProperties( const int& cols, const int& rows ) {

  // Set basic properties
  imgRows = rows;
  imgCols = cols;
  isValid = false;

  // Approx below properties (probably unused in manual mode)
  avgPixelSize = 1.0;
  avgHeight = avgPixelSize * imgRows;
  avgWidth = avgPixelSize * imgCols;
  estArea = avgHeight * avgWidth;
  pixelHeight = avgPixelSize;
  pixelWidth = avgPixelSize;
}

//Returns the type of the image (only identified via extension, not contents)
int ImageProperties::getImageType() { 
  string ext = filename.substr(filename.find_last_of(".") + 1);
  if( ext == "jpg" || ext == "JPG" )
    return JPEG;
  else if( ext == "tif" || ext == "TIF" )
    return RAW_TIF;
  else if( ext == "tiff" || ext == "TIFF" )
    return RAW_TIFF;
  else if( ext == "bmp" || ext == "BMP" )
    return BMP;
  else if( ext == "png" || ext == "PNG" )
    return PNG;
  return UNKNOWN;
}

// Process metadata
void ImageProperties::calculateProperties() {

  //Use modified scott_camera function to calculate real dimensions
  calculateDimensions( heading, pitch, roll, altitude,
    (float)imgCols, (float)imgRows, focalLength, estArea,
    avgHeight, avgWidth );

  //Calculate any other statistics
  pixelHeight = avgHeight / (float)imgRows;
  pixelWidth = avgWidth / (float)imgCols;
  avgPixelSize = ( pixelHeight + pixelWidth ) / 2;
}

// Load metadata from the specified file
bool ImageProperties::loadMetadata( const float& focal ) {

  // Open filestream
  ifstream inFile( filename.c_str() );

  // Check to make sure file opened
  if( !inFile.is_open() ) {
    cerr << "ERROR: Reading metadata fail.\n";
    return false;
  }

  // Set focal length (is not hardcoded in file)
  focalLength = focal;

  if( imageType == JPEG ) {

    // Read until we hit 'IMTAKE' 
    string word;
    int type = 0;
    int counter = 0;
    while(!inFile.eof()) {
      inFile >> word;
      counter++;
      if( word == "IMTAKE" ) {
        type = 1;
        break;
      } else if( word == "alt," ) {
        type = 2;
        break;
      } else if( counter >= MAX_META_SEARCH_DEPTH ) {
        return false;
      }
    }

    // Read in required variables
    if( type == 1 ) {
      inFile >> word; //Date (unused)
      inFile >> word; //Time (unused)
      inFile >> word; //Filename (unused)
      inFile >> altitude; //Altitude
      inFile >> depth; //Depth
      inFile >> heading; //Head
      inFile >> pitch; //Pitch
      inFile >> roll; //Roll
    } else if ( type == 2 ) {
      inFile >> word;
      word = word.substr( 0, word.length()-1 );
      altitude = (float) atof( word.c_str() ); //Altitude
      inFile >> word >> word;
      word = word.substr( 0, word.length()-1 );
      depth = (float) atof( word.c_str() );
      inFile >> word >> word;
      word = word.substr( 0, word.length()-1 );
      heading = (float) atof( word.c_str() );
      inFile >> word >> word;
      word = word.substr( 0, word.length()-1 );
      pitch = (float) atof( word.c_str() );
      inFile >> word >> word;
      word = word.substr( 0, word.length()-1 );
      roll = (float) atof( word.c_str() );
    } else {
      return false;
    }

  } else if( imageType == RAW_TIF || imageType == RAW_TIFF ) {

    //Initial seek location from end of file
    const int MAX_SEEK = 4408;
    inFile.seekg(-MAX_SEEK, ios::end);

    //Scan for start of header data
    string word, first;
    bool md_found = false;
    while(!inFile.eof() && !md_found) {
      getline(inFile,word,' ');
      stringstream ss(word);
      getline(ss,first,'&');
      if( first == "alt" ) 
        ss >> altitude;
      else if( first == "depth" )
        ss >> depth;
      else if( first == "head" )
        ss >> heading;
      else if( first == "pitch" )
        ss >> pitch;
      else if( first == "roll" ) {
        ss >> roll;
        md_found = true;
        break;
      }
    }
    if( !md_found ) {
      cerr << "ERROR: Metadata read fail\n";
      return false;
    }
  } else {
    cerr << "ERROR: Metadata read fail\n";
    inFile.close();
    return false;
  }

  // Close filestream
  inFile.close();

  //Adjust for special circumstances (unreported data)
  if( heading == -999.99f )
    heading = 0.0f;
  if( pitch == -999.99f )
    pitch = 0.0f;
  if( roll == -999.99f )
    roll = 0.0f;

  return true;
}
