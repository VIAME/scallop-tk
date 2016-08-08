
#include "StemID.h"

//------------------------------------------------------------------------------
//                                  Constants
//------------------------------------------------------------------------------

#define SCALLOP_TK_MAX_CORNERS 3

struct Corner {
  float r_adj;
  float c_adj;
  float mag;
};

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

void cornerSearch( Candidate *cd, vector<Corner>& cv );
void calculateTriangleFeatures( Corner& c1, Corner& c2, int index, float *arr );

//------------------------------------------------------------------------------
//                                 Definitions
//------------------------------------------------------------------------------

/*

void stemDetection( Candidate *cd, IplImage* unused ) {
  if( !cd->stats->active ) 
    return;
  vector<Corner> cv;
  //showIP( unused, cd->stats->imgAdj64, cd );
  cornerSearch( cd, cv );  

  if( cv.size() > 2 )
    calculateTriangleFeatures( cv[0], cv[1], 0, cd->stats->stem_info );
  if( cv.size() > 3 ) {
    calculateTriangleFeatures( cv[0], cv[2], 7, cd->stats->stem_info );
    calculateTriangleFeatures( cv[1], cv[2], 14, cd->stats->stem_info );
  }  
}

float isMaximum( IplImage* img, int r, int c )
{
  float val = getPixel32f( img, r, c );
  int j, k;
  
  for( j = -1; j <= 1; j++ )
    for( k = -1; k <= 1; k++ )
      if( val < getPixel32f( img, r + j, c + k ) )
        return 0;


  return val;
}

bool compareCorners( const Corner& c1, const Corner& c2 ) {
  return c2.mag < c1.mag;
}

void cornerSearch( Candidate *cd, vector<Corner>& cv ) {

  // Perform harris filter
  IplImage *harris = cvCreateImage( cvSize(64,64), IPL_DEPTH_32F, 1 );
  cvCornerHarris( cd->stats->imgAdj64, harris, 10 );
  cvSmooth( harris, harris, CV_BLUR, 9, 9 );

  // Identify maxima
  Corner local;
  for( int r=1; r<63; r++ ) {
    for( int c=1; c<63; c++ ) {
      local.mag = isMaximum( harris, r, c );
      if( local.mag != 0 ) {
        local.c_adj = c - 31.5;
        local.r_adj = r - 31.5;
        cv.push_back( local );
      }
    }
  }
  //showImageRange( harris );
  cvReleaseImage(&harris);

  // Sort vector
  sort( cv.begin(), cv.end(), compareCorners );
}

void calculateTriangleFeatures( Corner& c1, Corner& c2, int index, float *arr ) {
  float dist[3];
  float mag_ratio;
  float angle;

  dist[0] = sqrt(c1.r_adj*c1.r_adj + c1.c_adj*c1.c_adj);
  dist[1] = sqrt(c2.r_adj*c2.r_adj + c2.c_adj*c2.c_adj);
  float d1 = c2.r_adj - c1.r_adj;
  float d2 = c2.c_adj - c1.c_adj;
  dist[2] = sqrt(d1*d1+d2*d2);
  if( c2.mag < 0 )
    c2.mag = 0.0001;
  mag_ratio = c1.mag / c2.mag;
  if( mag_ratio > 1 && mag_ratio != 0 )
    mag_ratio = 1 / mag_ratio;
  double dot = (c1.r_adj / dist[0])*c2.r_adj/dist[1];
  dot = dot + (c1.c_adj / dist[0])*c2.c_adj/dist[1];
  angle = acos( dot );

  // insert into feature array
  arr[0+index] = dist[0];
  arr[1+index] = dist[1];
  arr[2+index] = dist[2];
  arr[3+index] = mag_ratio;
  arr[4+index] = angle;
  arr[5+index] = c1.mag;
  arr[6+index] = c2.mag;  
}

*/
