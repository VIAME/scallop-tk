//------------------------------------------------------------------------------
// Title: HelperFunctions.h
// Author: Matthew Dawkins
// Description: This file contains various helper function declarations
//------------------------------------------------------------------------------

#ifndef SCALLOP_TK_HELPER_FUNCTIONS_H_
#define SCALLOP_TK_HELPER_FUNCTIONS_H_

// C/C++ Includes
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

// OpenCV Includes
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

// Scallop Includes
#include "ScallopTK/Utilities/Definitions.h"

// Namespaces
using namespace std;


//------------------------------------------------------------------------------
//                                 Prototypes
//------------------------------------------------------------------------------

// Display Functions
void showImage( IplImage* img );
void showImageNW( IplImage* img );
void showImages( IplImage* img1, IplImage *img2 );
void showCandidates( IplImage *img, vector<Candidate*>& kps,
  float min = 0.0f, float max = INF );
void showCandidatesNW( IplImage *img, vector<Candidate*>& kps,
  float min = 0.0f, float max = INF );
void showScallops( IplImage *img, vector<Candidate*>& kps );
void saveScallops( IplImage *img, vector<Detection*>& kps,
  const string& fn );
void showScallopsNW( IplImage *img, vector<Candidate*>& kps );
void saveCandidates( IplImage *img, vector<Candidate*>& kps,
  const string& fn );
void saveImage( IplImage *img, const string &fn );
void showIPNW( IplImage* img, Candidate *ip );
void showMinMaxRad( IplImage* img, float minRad, float maxRad );
void showIP( IplImage* img, IplImage *img2, Candidate *ip );
void showIPNW( IplImage* img, IplImage *img2, Candidate *ip );

// Memory Functions
void deallocateCandidates( vector<Candidate*> &kps );
Candidate *copyCandidate( Candidate* kp );
int getImageType( const string& filename );

// Drawing functions
void drawFilledEllipse( IplImage *input, float r, float c,
  float angle, float major, float minor );
void drawEllipseRing( IplImage *input, float r, float c,
  float angle, float major1, float minor1, float major2 );
void drawColorRing( IplImage *input, float r, float c,
  float angle, float major1, float minor1, float major2,
  float major3, int (&bins)[COLOR_BINS] );

// Mask related
void updateMaskRing( IplImage *input, float r, float c,
  float angle, float major1, float minor1, float major2, tag obj );
void updateMask( IplImage *input, float r, float c,
  float angle, float major, float minor, tag obj );

// Misc
void saveMatrix( CvMat* mat, string filename );
string intToString( const int& i );
int stringToInt( const string& s );
inline void rgb2gray( IplImage *src, IplImage *dst );
void printMat( CvMat *A );
void showImageRange( IplImage* img );
void calcMinMax( IplImage *img );
void initalizeCandidateStats( vector<Candidate*> cds,
  int imheight, int imwidth );
float quickMedian( IplImage* img, int max_to_sample );
void removeBorderCandidates( vector<Candidate*>& cds, IplImage *img );
void cullNonImages( vector<string>& fn_list );

//------------------------------------------------------------------------------
//                               Inline Definitions
//------------------------------------------------------------------------------

// Get a pixel value from a 32-bit float image (single channel)
inline float getPixel32f( IplImage* img, int r, int c )
{
  return ( (float*)(img->imageData + img->widthStep*r) )[c];
}

// Get a pixel value from a 32-bit float image (single channel)
inline void setPixel32f( IplImage* img, int r, int c, float value )
{
  ( (float*)(img->imageData + img->widthStep*r) )[c] = value;
}

// Get a pixel value from a 32-bit float image (multi channel)
inline float getPixel32f( IplImage* img, int r, int c, int chan )
{
  return ( (float*)(img->imageData + img->widthStep*r) )[c*3 + chan];
}

inline void rgb2gray( IplImage *src, IplImage *dst ) {
  dst = cvCreateImage( cvGetSize( src ), IPL_DEPTH_8U, 1 );
  cvCvtColor( src, dst, CV_RGB2GRAY );
}

inline void scaleCD( Candidate *cd, float sf ) {
  cd->r = cd->r * sf;
  cd->c = cd->c * sf;
  cd->major = cd->major * sf;
  cd->minor = cd->minor * sf;
}

inline int dround( const double& input ) {
  float exp = input - floor( input );
  if( exp < 0.5 )
    return floor( input );
  return ceil( input );
}

inline int determine8quads( const int& x, const int&y ) {
  if( x <= 0 ) {
    if( y <= 0 ) {
      if( x <= y ) {
        return 0;
      } else {
        return 1;
      }
    } else {
      if( -x > y ) {
        return 2;
      } else {
        return 3;
      }
    }
  } else {
    if( y <= 0 ) {
      if( x > -y ) {
        return 4;
      } else {
        return 5;
      }
    } else {
      if( x > y ) {
        return 6;
      } else {
        return 7;
      }
    }
  }
}

inline Candidate* ConvertGTToCandidate( GTEntry& Pt, float DownsizeFactor )
{
  // Convert a GT point to a Candidate with some random flux
  Candidate* output = new Candidate;

  // Calculate random adjustment factors
  double RAND_R = ( ((double)rand()/(double)RAND_MAX) - 0.5 ) / 5;
  double RAND_C = ( ((double)rand()/(double)RAND_MAX) - 0.5 ) / 5;
  double RAND_ANGLE = 3.14159 * ((double)rand()/(double)RAND_MAX);
  double RAND_MAJOR = 1.0 + ( ((double)rand()/(double)RAND_MAX) - 0.5 ) / 5;
  double RAND_MINOR = 1.0 + ( ((double)rand()/(double)RAND_MAX) - 0.5 ) / 5;
  if( ((double)rand()/(double)RAND_MAX) < 0.3 )
    RAND_ANGLE = 0.0;

  // Calculate GT location in native
  double R = ( Pt.Y1 + Pt.Y2 ) / 2.0;
  double C = ( Pt.X1 + Pt.X2 ) / 2.0;
  double x_sqr = Pt.X2 - Pt.X1;
  x_sqr = x_sqr * x_sqr;
  double y_sqr = Pt.Y2 - Pt.Y1;
  y_sqr = y_sqr * y_sqr;
  double DIST = sqrt( x_sqr + y_sqr );

  // Adjust location for rescale factor
  R *= DownsizeFactor;
  C *= DownsizeFactor;
  DIST *= DownsizeFactor;

  // Set Candidate vals
  output->r = R + DIST * RAND_R;
  output->c = C + DIST * RAND_C;
  output->major = RAND_MAJOR * DIST / 2.0;
  output->minor = RAND_MINOR * DIST / 2.0;
  output->angle = RAND_ANGLE;

  // Set classification value
  output->classification = Pt.ID;

  return output;
}

// Move this function to somplace better
inline void RemoveOverlapAndMerge( vector<Candidate*>& Base,
  vector<Candidate*>& Truth, double percentage_keep = 0.10 )
{
  // Top down greedy search - very slow but who cares its for training only
  for( int j = Base.size() - 1; j >= 0; j-- )
  {
    // Determine if we should kill this Candidate (too close to GT)
    bool RemoveCandidate = false;

    for( int i = 0; i < Truth.size(); i++ )
    {
      double MAJOR = ( Base[j]->major < Truth[i]->major ? Truth[i]->major : Base[j]->major );
      double COMP1 = Base[j]->r - Truth[i]->r;
      double COMP2 = Base[j]->c - Truth[i]->c;
      COMP1 *= COMP1;
      COMP2 *= COMP2;
      double DIST = sqrt( COMP1 + COMP2 );
      if( DIST < 1.8 * MAJOR )
      {
        RemoveCandidate = true;
        break;
      }
    }

    if( RemoveCandidate || ((double)rand()/(double)RAND_MAX) > percentage_keep )
    {
      delete Base[j];
      Base.erase(Base.begin()+j);
    }
    else
    {
      Base[j]->classification = 0;
    }
  }

  // Merge results
  for( int j = 0; j < Truth.size(); j++ )
  {
    Base.push_back( Truth[j] );
  }
}

#endif
