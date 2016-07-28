//------------------------------------------------------------------------------
// Title: helper.h
// Author: Matthew Dawkins
// Description: This file contains various helper function declarations
//------------------------------------------------------------------------------

#ifndef HELPER_H
#define HELPER_H

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
#include "../Common/definitions.h"

// Namespaces
using namespace std;

//------------------------------------------------------------------------------
//                                 Prototypes
//------------------------------------------------------------------------------

// Good luck!
void showImage( IplImage* img );
void showImageNW( IplImage* img );
void showImages( IplImage* img1, IplImage *img2 );
void deallocateCandidates( vector<candidate*> &kps );
candidate *copyCandidate( candidate* kp );
int getImageType( const string& filename );
void showCandidates( IplImage *img, vector<candidate*>& kps, float min = 0.0f, float max = INF );
void showScallops( IplImage *img, vector<candidate*>& kps );
void saveScallops( IplImage *img, vector<detection*>& kps, const string& fn );
void showCandidatesNW( IplImage *img, vector<candidate*>& kps, float min = 0.0f, float max = INF );
void saveMatrix( CvMat* mat, string filename );
string intToString(const int& i);
int stringToInt(const string& s);
inline void rgb2gray( IplImage *src, IplImage *dst );
void showIP( IplImage* img, IplImage *img2, candidate *ip );
void PrintMat(CvMat *A);
void showImageRange( IplImage* img );
void calcMinMax( IplImage *img );
void drawFilledEllipse( IplImage *input, float r, float c, float angle, float major, float minor );
void drawEllipseRing( IplImage *input, float r, float c, float angle, float major1, float minor1, float major2 );
void drawColorRing( IplImage *input, float r, float c, float angle, float major1, float minor1, float major2, float major3, int (&bins)[COLOR_BINS] );
void showIPNW( IplImage* img, IplImage *img2, candidate *ip );
void initalizeCandidateStats( vector<candidate*> cds, int imheight, int imwidth );
void updateMaskRing( IplImage *input, float r, float c, float angle, float major1, float minor1, float major2, tag obj );
void updateMask( IplImage *input, float r, float c, float angle, float major, float minor, tag obj );
float quickMedian( IplImage* img, int max_to_sample );
void showScallopsNW( IplImage *img, vector<candidate*>& kps );
void saveCandidates( IplImage *img, vector<candidate*>& kps, const string& fn );
void saveImage( IplImage *img, const string &fn );
void showIPNW( IplImage* img, candidate *ip );
void RemoveBorderCandidates( vector<candidate*>& cds, IplImage *img );

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

inline void scaleCD( candidate *cd, float sf ) {
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


#endif
