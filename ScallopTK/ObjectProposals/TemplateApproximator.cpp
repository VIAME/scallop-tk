//------------------------------------------------------------------------------
// Title: Template2.cpp
// Author: Matthew Dawkins
// Description: 
//------------------------------------------------------------------------------

#include "TemplateApproximator.h"

//------------------------------------------------------------------------------
//                              Misc Definitions
//------------------------------------------------------------------------------

// Constants
const int INTERVALS_SCALE1 = 4;
const int INTERVALS_SCALE2 = 7;
const int INTERVALS_SCALE3 = 5;
const int START1 = 1;
const int END1 = START1 + INTERVALS_SCALE1;
const int START2 = END1 + 2;
const int END2 = START2 + INTERVALS_SCALE2;
const int START3 = END2 + 2;
const int END3 = START3 + INTERVALS_SCALE3;
const int TOTAL_SCALES = END3 + 1;
const float BASE_SIGMA = 1.2f;
const int MAX_T4_IP = INT_MAX;

// Structs
struct t4cand {
  float r, c, scl, mag;
};

typedef vector<t4cand> Candidates;

bool compareT4( const t4cand& cd1, const t4cand& cd2 ) {
  if( cd1.mag > cd2.mag )
    return true;
  return false;
}

// Scale Space Information
struct SSInfo {
  float RELATIVE_SCALE[TOTAL_SCALES];
  float SCALE_RADII[TOTAL_SCALES];
  float SCALE_OFFSET[TOTAL_SCALES];
  float TOP_OFFSET[TOTAL_SCALES];
};

//------------------------------------------------------------------------------
//                            Function Prototypes
//------------------------------------------------------------------------------

IplImage *gaussDerivVerticle( IplImage *input, double sigma );
IplImage *gaussDerivHorizontal( IplImage *input, double sigma );
IplImage *createT4Scale( IplImage *dx, IplImage *dy, float radius, float offset );
IplImage *createT4ScaleC( IplImage *dx, IplImage *dy, float radius, int offset );
void detectT4Extremum( IplImage** ss, Candidates& cds, SSInfo& ssinfo );
void interpolateIP( IplImage **ss, Candidates& cds, vector<Candidate*>& kps, float resize_factor,
      float minRad, float maxRad, int height, int width, ImageProperties& imgProp, SSInfo& ssinfo );

//------------------------------------------------------------------------------
//                               Benchmarking
//------------------------------------------------------------------------------

#ifdef TEMPLATE_ENABLE_BENCHMARKINGING
  const string temp_bm_fn = "TemplateBMResults.dat";
  vector<double> tp_exe_times;
  ofstream tp_bm_output;
#endif

//------------------------------------------------------------------------------
//                            Function Definitions
//------------------------------------------------------------------------------

void showChain( IplImage **ss ) {  
  for( int i=0; i<END3; i++ ) {
    cvScale( ss[i], ss[i], .003f );
    showImage( ss[i] );
  }
}

float quickMedianTemp( IplImage* img ) {
  float *ptr = (float*) img->imageData;
  int sze = img->width * img->height;
  float *endptr = ptr + sze;
  int skippage = 20;
  if( sze / 1000 > 20 )
    skippage = sze / 1000;
  vector<float> val_list(sze/skippage+1,0.0f);
  int counter = 0;
  while( ptr < endptr ) {
    val_list[counter] = *ptr;
    counter++;
    ptr = ptr + skippage;
  }
  sort( val_list.begin(), val_list.end() );
  return val_list[0.5f*sze/skippage];
}

// Create tapprox hough ss
IplImage **createScaleSpace( IplImage* dx, IplImage *dy, float minRad, float maxRad, SSInfo& ssinfo, IplImage *mask ) {

  // Initialize array
  int total_scales = INTERVALS_SCALE1 + INTERVALS_SCALE2 + INTERVALS_SCALE3 + 6;
  IplImage **ss = (IplImage**) malloc( total_scales * sizeof( IplImage* ) );

  // Adjust radius for standard->temp difference
  float ratio = minTemplateRadius / minRadius;
  float firstScanRad = minRad * ratio;
  float lastScanRad = maxRad;

  // Calculate required statistics
  /*float step = (lastScanRad - firstScanRad) / 3;
  float s1 = firstScanRad;
  float s2 = firstScanRad + step;
  float s3 = firstScanRad + 2*step;
  float s4 = lastScanRad;
  float intvl1 = (s2-s1) / INTERVALS_SCALE1;
  float intvl2 = (s3-s2) / INTERVALS_SCALE2;
  float intvl3 = (s4-s3) / INTERVALS_SCALE3;*/
  float s1 = firstScanRad;
  float s2 = firstScanRad*2;
  float s3 = firstScanRad*4;
  float s4 = lastScanRad;
  float intvl1 = (s2-s1) / INTERVALS_SCALE1;
  float intvl2 = (s3-s2) / INTERVALS_SCALE2;
  float intvl3 = (s4-s3) / INTERVALS_SCALE3;
  //assert( s3 < s4 );

  // Add border to base image
  CvSize newSize;
  newSize.height = dx->height + 2*maxRad;
  newSize.width = dx->width + 2*maxRad;
  IplImage* dxBase = cvCreateImage( newSize, dx->depth, dx->nChannels );
  IplImage* dyBase = cvCreateImage( newSize, dy->depth, dy->nChannels );
  CvPoint offset;
  offset.x = maxRad;
  offset.y = maxRad;
  CvScalar scala;
  scala.val[0] = quickMedianTemp( dx );
  cvCopyMakeBorder( dx, dxBase, offset, IPL_BORDER_CONSTANT, scala );
  cvCopyMakeBorder( dy, dyBase, offset, IPL_BORDER_CONSTANT, scala );
  cvSmooth( dxBase, dxBase, 2, 3, 3 );
  cvSmooth( dyBase, dyBase, 2, 3, 3 );

  // Create SS#1
  float currad = s1 - intvl1;
  for( int i=START1-1; i<END1+1; i++ ) {
    ssinfo.RELATIVE_SCALE[i] = 1.0f;
    ssinfo.SCALE_OFFSET[i] = maxRad;
    ssinfo.SCALE_RADII[i] = currad;
    ss[i] = createT4ScaleC( dxBase, dyBase, currad, maxRad );
    currad = currad + intvl1;
  }

  // Resize base to remaining 2 scales
  float resize_factor2 = 0.5f;
  IplImage *dxLvl2 = dxBase;
  IplImage *dyLvl2 = dyBase;
  if( resize_factor2 < 1.0f ) {
    int nheight = resize_factor2 * dxBase->height;
    int nwidth = resize_factor2 * dxBase->width;
    dxLvl2 = cvCreateImage( cvSize(nwidth, nheight), IPL_DEPTH_32F, 1 );
    dyLvl2 = cvCreateImage( cvSize(nwidth, nheight), IPL_DEPTH_32F, 1 );
    int sf = (int)(1.0 / resize_factor2);
    if( sf > 1 ) {
      cvSmooth(dxBase,dxBase,CV_BLUR,1+sf,1+sf);
      cvSmooth(dyBase,dyBase,CV_BLUR,1+sf,1+sf);
    }
    cvResize( dxBase, dxLvl2 );
    cvResize( dyBase, dyLvl2 );
  } else {
    resize_factor2 = 1.0f;
  }

  // Create SS#2
  currad = s2 - intvl2;
  for( int i=START2-1; i<END2+1; i++ ) {
    ssinfo.RELATIVE_SCALE[i] = resize_factor2;
    ssinfo.TOP_OFFSET[i] = maxRad;
    ssinfo.SCALE_OFFSET[i] = maxRad * resize_factor2;
    ssinfo.SCALE_RADII[i] = currad;
    ss[i] = createT4ScaleC( dxLvl2, dyLvl2, currad*resize_factor2, ssinfo.SCALE_OFFSET[i] );
    currad = currad + intvl2;
  }

  // Resize base to remaining 2 scales
  float resize_factor3 = 0.5f;
  IplImage *dxLvl3 = dxLvl2;
  IplImage *dyLvl3 = dyLvl2;
  if( resize_factor3 < 1.0f ) {
    int nheight = resize_factor3 * dxLvl2->height;
    int nwidth = resize_factor3 * dxLvl2->width;
    dxLvl3 = cvCreateImage( cvSize(nwidth, nheight), IPL_DEPTH_32F, 1 );
    dyLvl3 = cvCreateImage( cvSize(nwidth, nheight), IPL_DEPTH_32F, 1 );
    int sf = (int)(1.0 / resize_factor3);
    if( sf > 1 ) {
      cvSmooth(dxLvl2,dxLvl2,CV_BLUR,1+sf,1+sf);
      cvSmooth(dyLvl2,dyLvl2,CV_BLUR,1+sf,1+sf);
    }
    cvResize( dxLvl2, dxLvl3 );
    cvResize( dyLvl2, dyLvl3 );
  } else {
    resize_factor3 = 1.0f;
  }

  // Create SS#3
  currad = s3 - intvl3;
  for( int i=START3-1; i<END3+1; i++ ) {
    ssinfo.SCALE_RADII[i] = currad;
    ssinfo.RELATIVE_SCALE[i] = resize_factor2 * resize_factor3;
    ssinfo.TOP_OFFSET[i] = maxRad;
    ssinfo.SCALE_OFFSET[i] = maxRad / 4.0f;
    ss[i] = createT4ScaleC( dxLvl3, dyLvl3, currad*resize_factor2*resize_factor3, maxRad / 4.0f );
    currad = currad + intvl3;
  }

  // Deallocations
  cvReleaseImage( &dxBase );
  cvReleaseImage( &dyBase );
  if( resize_factor2 < 1.0f ) {
    cvReleaseImage( &dxLvl2 );
    cvReleaseImage( &dyLvl2 );
  }
  if( resize_factor3 < 1.0f ) {
    cvReleaseImage( &dxLvl3 );
    cvReleaseImage( &dyLvl3 );
  }

  return ss;

  /*for( float rad = minRad; rad <= maxRad; rad += (maxRad-minRad)/14 ) {
    IplImage *scale = createT4Scale( dx, dy, rad );
    cvSmooth( scale, scale, CV_BLUR, 5, 5 );
    cvScale( scale, scale, 0.005 );
    showImage(scale);
    //showImage(netdx);
    cvReleaseImage(&scale);
  }*/
}

// Dealloc t4 ss
void deallocateScaleSpace( IplImage** ss ) {
  int total = INTERVALS_SCALE1 + INTERVALS_SCALE2 + INTERVALS_SCALE3 + 6;
  for( int j=0; j<total; j++ ) {
    cvReleaseImage(&ss[j]);
  }
  free(ss);
}
// Option 1: don't use shifted temp buffers
IplImage *createT4Scale( IplImage *dx, IplImage *dy, float radius, float offset ) {

  const int BINS = 20;
  int rpos[BINS];
  int cpos[BINS];

  for( int i = 0; i<BINS; i++ ) {
    float angle = 2*PI*i/BINS - 0.7*PI;
    rpos[i] = radius * sin( angle );
    cpos[i] = radius * cos( angle );
  }

  IplImage *scale = cvCreateImage( cvGetSize( dx ), IPL_DEPTH_32F, 1 );
  cvZero(scale);
  int step = dx->widthStep;
  for( int r=offset+1; r < scale->height-offset-1; r++ ) {
    for( int c=offset+1; c < scale->width-offset-1; c++ ) {
      float sum = 0.0f;
      for( int j=0; j<5; j++ ) 
        sum = sum + ((float*)(dy->imageData + step*(r+rpos[j])))[c+cpos[j]];
      for( int j=5; j<10; j++ ) 
        sum = sum + ((float*)(dx->imageData + step*(r+rpos[j])))[c+cpos[j]];
      for( int j=10; j<15; j++ ) 
        sum = sum + ((float*)(dy->imageData + step*(r+rpos[j])))[c+cpos[j]];
      for( int j=15; j<20; j++ ) 
        sum = sum + ((float*)(dx->imageData + step*(r+rpos[j])))[c+cpos[j]];
      ((float*)(scale->imageData + scale->widthStep*r))[c] = sum;
    }
  }
  return scale;
}

// Option 2: use shifted temp buffers
IplImage *createT4ScaleB( IplImage *dx, IplImage *dy, float radius ) {

  IplImage *scale = cvCreateImage( cvGetSize( dx ), IPL_DEPTH_32F, 1 );
  for( int r=0; r < scale->height; r++ ) {
    for( int c=0; c < scale->width; c++ ) {

    }
  }
  return scale;
}

inline float calcSimularityFactor( float *input, float& sum ) {
  //Get max [unopt]
  float max = 0.0f;
  for( int i=0; i<20; i++ )
    if( input[i] > max )
      max = input[i];
  max = sum / max;
  return max;
}

// Option 1: don't use shifted temp buffers and use mag shifting
IplImage *createT4ScaleC( IplImage *dx, IplImage *dy, float radius, int offset ) {

  int bpfloat = (IPL_DEPTH_32F/8);
  int wstep = dx->widthStep / bpfloat;
  const int BINS = 20;
  int rpos[BINS];
  int cpos[BINS];
  int pos_offset[BINS];
  float magnitudes[BINS];

  for( int i = 0; i<BINS; i++ ) {
    float angle = 2*PI*i/BINS - 0.7*PI;
    rpos[i] = radius * sin( angle );
    cpos[i] = radius * cos( angle );
    pos_offset[i] = rpos[i]*wstep + cpos[i];
  }

  IplImage *scale = cvCreateImage( cvGetSize( dx ), IPL_DEPTH_32F, 1 );
  cvZero(scale);
  int step = dx->widthStep;
  float *outptr = ((float*)(scale->imageData + scale->widthStep*(offset+1)))+(offset+1);
  float *dxin = ((float*)(dx->imageData + dx->widthStep*(offset+1)))+(offset+1);
  float *dyin = ((float*)(dy->imageData + dy->widthStep*(offset+1)))+(offset+1);
  int rstep = wstep-(scale->width-offset-1-(offset+1));
  for( int r=offset+1; r < scale->height-offset-1; r++ ) {
    for( int c=offset+1; c < scale->width-offset-1; c++ ) {
      // Because who needs for loops anyways
      /*magnitudes[0] = ((float*)(dy->imageData + step*(r+rpos[0])))[c+cpos[0]];
      magnitudes[1] = ((float*)(dy->imageData + step*(r+rpos[1])))[c+cpos[1]];
      magnitudes[2] = ((float*)(dy->imageData + step*(r+rpos[2])))[c+cpos[2]];
      magnitudes[3] = ((float*)(dy->imageData + step*(r+rpos[3])))[c+cpos[3]];
      magnitudes[4] = ((float*)(dy->imageData + step*(r+rpos[4])))[c+cpos[4]];
      magnitudes[5] = ((float*)(dx->imageData + step*(r+rpos[5])))[c+cpos[5]];
      magnitudes[6] = ((float*)(dx->imageData + step*(r+rpos[6])))[c+cpos[6]];
      magnitudes[7] = ((float*)(dx->imageData + step*(r+rpos[7])))[c+cpos[7]];
      magnitudes[8] = ((float*)(dx->imageData + step*(r+rpos[8])))[c+cpos[8]];
      magnitudes[9] = ((float*)(dx->imageData + step*(r+rpos[9])))[c+cpos[9]];
      magnitudes[10] = ((float*)(dy->imageData + step*(r+rpos[10])))[c+cpos[10]];
      magnitudes[11] = ((float*)(dy->imageData + step*(r+rpos[11])))[c+cpos[11]];
      magnitudes[12] = ((float*)(dy->imageData + step*(r+rpos[12])))[c+cpos[12]];
      magnitudes[13] = ((float*)(dy->imageData + step*(r+rpos[13])))[c+cpos[13]];
      magnitudes[14] = ((float*)(dy->imageData + step*(r+rpos[14])))[c+cpos[14]];
      magnitudes[15] = ((float*)(dx->imageData + step*(r+rpos[15])))[c+cpos[15]];
      magnitudes[16] = ((float*)(dx->imageData + step*(r+rpos[16])))[c+cpos[16]];
      magnitudes[17] = ((float*)(dx->imageData + step*(r+rpos[17])))[c+cpos[17]];
      magnitudes[18] = ((float*)(dx->imageData + step*(r+rpos[18])))[c+cpos[18]];
      magnitudes[19] = ((float*)(dx->imageData + step*(r+rpos[19])))[c+cpos[19]];*/
      magnitudes[0] = *(dyin+pos_offset[0]);
      magnitudes[1] = *(dyin+pos_offset[1]);
      magnitudes[2] = *(dyin+pos_offset[2]);
      magnitudes[3] = *(dyin+pos_offset[3]);
      magnitudes[4] = *(dyin+pos_offset[4]);
      magnitudes[5] = *(dxin+pos_offset[5]);
      magnitudes[6] = *(dxin+pos_offset[6]);
      magnitudes[7] = *(dxin+pos_offset[7]);
      magnitudes[8] = *(dxin+pos_offset[8]);
      magnitudes[9] = *(dxin+pos_offset[9]);
      magnitudes[10] = *(dyin+pos_offset[10]);
      magnitudes[11] = *(dyin+pos_offset[11]);
      magnitudes[12] = *(dyin+pos_offset[12]);
      magnitudes[13] = *(dyin+pos_offset[13]);
      magnitudes[14] = *(dyin+pos_offset[14]);
      magnitudes[15] = *(dxin+pos_offset[15]);
      magnitudes[16] = *(dxin+pos_offset[16]);
      magnitudes[17] = *(dxin+pos_offset[17]);
      magnitudes[18] = *(dxin+pos_offset[18]);
      magnitudes[19] = *(dxin+pos_offset[19]);

      // Calculate sum
      float sum = magnitudes[0] + magnitudes[1] + magnitudes[2] + magnitudes[3] + magnitudes[4] +
        magnitudes[5] + magnitudes[6] + magnitudes[7] + magnitudes[8] + magnitudes[9] +
        magnitudes[10] + magnitudes[11] + magnitudes[12] + magnitudes[13] + magnitudes[14] +
        magnitudes[15] + magnitudes[16] + magnitudes[17] + magnitudes[18] + magnitudes[19];

      // Calculate simularity scaling factor
      float sim = calcSimularityFactor( magnitudes, sum );

      // Insert value
      *outptr = sum ;//* sim;
      outptr++;
      dxin++;
      dyin++;
    }
    outptr+=rstep;
    dyin+=rstep;
    dxin+=rstep;
  }
  cvSmooth(scale, scale, 2, 9, 9 );
  //showImageRange(scale);
  return scale;
}

//Is this position higher than its 26 surrounding bins?
int isT4Extremum( IplImage** imgChain, int intvl, int r, int c )
{
  float val = getPixel32f( imgChain[intvl], r, c );
  for( int i = -1; i <= 1; i++ )
    for( int j = -1; j <= 1; j++ )
      for( int k = -1; k <= 1; k++ )
        if( val < getPixel32f( imgChain[intvl + i], r + j, c + k ) )
          return false;


  return true;
}

void detectT4Extremum( IplImage** ss, Candidates& cds, SSInfo& ssinfo )
{
  //Optional Threshold
  double threshold = 0;

  //Scale 1 maxima
  for( int i = START1; i < END1; i++ ) {
    int offset = (int) ssinfo.SCALE_OFFSET[i];
    for( int r = offset+1; r < ss[i]->height-1-offset; r++ ) {
      for( int c = offset+1; c < ss[i]->width-1-offset; c++ ) {
        float value = getPixel32f( ss[i], r, c );
        if( value > threshold ) {
          if( isT4Extremum( ss, i, r, c ) )  {
            t4cand cd;
            cd.r = r;
            cd.c = c;
            cd.scl = i;
            cd.mag = value;
            cds.push_back( cd );
          }
        }
      }
    }
  }

  //Scale 2 maxima
  for( int i = START2; i < END2; i++ ) {
    int offset = (int) ssinfo.SCALE_OFFSET[i];
    for( int r = offset+1; r < ss[i]->height-1-offset; r++ ) {
      for( int c = offset+1; c < ss[i]->width-1-offset; c++ ) {
        float value = getPixel32f( ss[i], r, c );
        if( value > threshold ) {
          if( isT4Extremum( ss, i, r, c ) )  {
            t4cand cd;
            cd.r = r;
            cd.c = c;
            cd.scl = i;
            cd.mag = value;
            cds.push_back( cd );
          }
        }
      }
    }
  }

  //Scale 3 maxima
  for( int i = START3; i < END3; i++ ) {
    int offset = (int) ssinfo.SCALE_OFFSET[i];
    for( int r = offset+1; r < ss[i]->height-1-offset; r++ ) {
      for( int c = offset+1; c < ss[i]->width-1-offset; c++ ) {
        float value = getPixel32f( ss[i], r, c );
        if( value > threshold ) {
          if( isT4Extremum( ss, i, r, c ) )  {
            t4cand cd;
            cd.r = r;
            cd.c = c;
            cd.scl = i;
            cd.mag = value;
            cds.push_back( cd );
          }
        }
      }
    }
  }
}

//Todo: package arguments in imageProperties - cleanup function
void interpolateIP( IplImage **ss, Candidates& cds, vector<Candidate*>& kps, float resize_factor,
      float minRad, float maxRad, int height, int width, ImageProperties& imgProp, SSInfo& ssinfo ) {

  // Constants
  const float bins_per_meter_s1 = 10.0f;
  const float bins_per_meter_s2 = 5.0f;
  const float bins_per_meter_s3 = 4.0f;
  const int s1_ppg = 4;
  const int s2_ppg = 3;
  const int s3_ppg = 3;
  
  // Create Logging Bins - LB1
  int r_groups_s1 = imgProp.getImgHeightMeters() * bins_per_meter_s1;
  int c_groups_s1 = imgProp.getImgWidthMeters() * bins_per_meter_s1;
  if( r_groups_s1 <= 0 )
    r_groups_s1 = 1;
  if( c_groups_s1 <= 0 )
    c_groups_s1 = 1;
  float s1_lower = minRad;
  float s1_upper = (maxRad - minRad)/3 + minRad;
  int **bin1 = new int *[r_groups_s1];
  for( int i=0; i<r_groups_s1; i++ ) {
    bin1[i] = new int[c_groups_s1];
    for( int j=0; j<c_groups_s1; j++ )
      bin1[i][j] = 0;
  }
  int r_step_s1 = height / r_groups_s1;
  int c_step_s1 = width / c_groups_s1;

  // Create Logging Bins - LB2
  int r_groups_s2 = imgProp.getImgHeightMeters() * bins_per_meter_s2;
  int c_groups_s2 = imgProp.getImgWidthMeters() * bins_per_meter_s2;
  if( r_groups_s2 <= 0 )
    r_groups_s2 = 1;
  if( c_groups_s2 <= 0 )
    c_groups_s2 = 1;
  float s2_lower = (maxRad - minRad)/3 + minRad;
  float s2_upper = 2*(maxRad - minRad)/3 + minRad;
  int **bin2 = new int *[r_groups_s2];
  for( int i=0; i<r_groups_s2; i++ ) {
    bin2[i] = new int[c_groups_s2];
    for( int j=0; j<c_groups_s2; j++ )
      bin2[i][j] = 0;
  }
  int r_step_s2 = height / r_groups_s2;
  int c_step_s2 = width / c_groups_s2;

  // Create Logging Bins - LB3
  int r_groups_s3 = imgProp.getImgHeightMeters() * bins_per_meter_s3;
  int c_groups_s3 = imgProp.getImgWidthMeters() * bins_per_meter_s3;
  if( r_groups_s3 <= 0 )
    r_groups_s3 = 1;
  if( c_groups_s3 <= 0 )
    c_groups_s3 = 1;
  float s3_lower = 2*(maxRad - minRad)/3 + minRad;
  float s3_upper = maxRad;
  int **bin3 = new int *[r_groups_s3];
  for( int i=0; i<r_groups_s3; i++ ) {
    bin3[i] = new int[c_groups_s3];
    for( int j=0; j<c_groups_s3; j++ )
      bin3[i][j] = 0;
  }
  int r_step_s3 = height / r_groups_s3;
  int c_step_s3 = width / c_groups_s3;
    
  int maxIP = r_groups_s1*c_groups_s1*s1_ppg;
  maxIP = maxIP + r_groups_s2*c_groups_s2*s2_ppg;
  maxIP = maxIP + r_groups_s3*c_groups_s3*s3_ppg;

  // Sort all t4 Candidates based on magnitude
  sort( cds.begin(), cds.end(), compareT4 );

  // For every interest point...
  int counter = 0;
  for( unsigned int i=0; i<cds.size(); i++ ) {

    if( counter == MAX_T4_IP || counter == maxIP )
      break;

    Candidate *kp = new Candidate;
    bool add = false;

    //Scales 1-3
    int scale_num = cds[i].scl;
    float lvl_scale = resize_factor * ssinfo.RELATIVE_SCALE[scale_num];
    kp->r = (cds[i].r - ssinfo.SCALE_OFFSET[scale_num]) / lvl_scale;
    kp->c = (cds[i].c - ssinfo.SCALE_OFFSET[scale_num]) / lvl_scale;
    kp->major =ssinfo.SCALE_RADII[scale_num] / resize_factor;
    kp->minor = kp->major;
    kp->angle = 0;

    //Check if we should add Candidate - bin1
    if( kp->major >= s1_lower && kp->major < s1_upper ) {
      int b1 = ((int) kp->r / ( r_step_s1 ))%r_groups_s1;
      int b2 = ((int) kp->c / ( c_step_s1 ))%c_groups_s1;
      if( bin1[b1][b2] < s1_ppg ) {
        add = true;
        bin1[b1][b2]++;
        //kp->magnitude = bin1[b1][b2] * cds[i].mag;
        kp->magnitude = cds[i].mag;
        kp->method = TEMPLATE;
      }

    } else if( kp->major >= s2_lower && kp->major < s2_upper ) {
      int b1 = ((int) kp->r / ( r_step_s2 ))%r_groups_s2;
      int b2 = ((int) kp->c / ( c_step_s2 ))%c_groups_s2;
      if( bin2[b1][b2] < s2_ppg ) {
        add = true;
        bin2[b1][b2]++;
        //kp->magnitude = bin2[b1][b2] * cds[i].mag;
        kp->magnitude = cds[i].mag;
        kp->method = TEMPLATE;
      }

    } else if( kp->major >= s3_lower && kp->major <= s3_upper ) {
      int b1 = ((int) kp->r / ( r_step_s3 ))%r_groups_s3;
      int b2 = ((int) kp->c / ( c_step_s3 ))%c_groups_s3;
      if( bin3[b1][b2] < s3_ppg ) {
        add = true;
        bin3[b1][b2]++;
        //kp->magnitude = bin3[b1][b2] * cds[i].mag;
        kp->magnitude = cds[i].mag;
        kp->method = TEMPLATE;
      }
    }

    //Add or reject
    if( add ) {
      kps.push_back( kp );  
      counter++;
    } else {
      delete kp;
    }
  }

  // Deallocations
  for( int i = 0; i < r_groups_s1; i++ ) {
    delete[] bin1[i];
  }
  delete[] bin1;
  for( int i = 0; i < r_groups_s2; i++ ) {
    delete[] bin2[i];
  }
  delete[] bin2;
  for( int i = 0; i < r_groups_s3; i++ ) {
    delete[] bin3[i];
  }
  delete[] bin3;
}


//Find Double-Donut Candidate
void findTemplateCandidates( GradientChain& grad, std::vector<Candidate*>& kps, ImageProperties& imgProp, IplImage* mask ) {

  // Normalize image scale
  float resize_factor = MIN_RAD_TEMPLATE / grad.minRad;
  IplImage *dx = grad.dx;
  IplImage *dy = grad.dy;
  if( resize_factor < RESIZE_FACTOR_REQUIRED ) {
    int nheight = resize_factor * grad.dx->height;
    int nwidth = resize_factor * grad.dx->width;
    dx = cvCreateImage( cvSize(nwidth, nheight), IPL_DEPTH_32F, 1 );
    dy = cvCreateImage( cvSize(nwidth, nheight), IPL_DEPTH_32F, 1 );
    cvResize( grad.dx, dx );
    cvResize( grad.dy, dy );
  } else {
    resize_factor = 1.0f;
  }
  float minRad = grad.minRad * resize_factor;
  float maxRad = grad.maxRad * resize_factor;

#ifdef TEMPLATE_ENABLE_BENCHMARKINGING
  tp_bm_output.open( temp_bm_fn.c_str(), fstream::out | fstream::app );
  tp_exe_times.clear();
  initializeTimer();
  startTimer();
#endif
  
  // Create Scale Space
  SSInfo scaleSpaceInfo;
  IplImage **ss = createScaleSpace( dx, dy, minRad, maxRad, scaleSpaceInfo, mask );

#ifdef TEMPLATE_ENABLE_BENCHMARKINGING
  tp_exe_times.push_back( getTimeSinceLastCall() );  
#endif

  // Identify extrema in scale space (ordered by magnitude)
  Candidates cds;
  detectT4Extremum( ss, cds, scaleSpaceInfo );

#ifdef TEMPLATE_ENABLE_BENCHMARKINGING
  tp_exe_times.push_back( getTimeSinceLastCall() );  
#endif

  // Interpolate and adjust Candidates
  interpolateIP( ss, cds, kps, resize_factor, grad.minRad/grad.scale, grad.maxRad/grad.scale, 
    grad.dx->height/grad.scale, grad.dx->width/grad.scale, imgProp, scaleSpaceInfo );

#ifdef TEMPLATE_ENABLE_BENCHMARKINGING
  tp_exe_times.push_back( getTimeSinceLastCall() );  
#endif

  // Deallocate memory
  deallocateScaleSpace( ss );
  if( resize_factor < RESIZE_FACTOR_REQUIRED ) {
    cvReleaseImage(&dx);
    cvReleaseImage(&dy);
  }

#ifdef TEMPLATE_ENABLE_BENCHMARKINGING
  for( int i = 0; i < tp_exe_times.size(); i++ )
    tp_bm_output << tp_exe_times[i] << " ";
  tp_bm_output << endl;
  tp_bm_output.close();
#endif
}

//Find quick hough kp
void findQHCandidates( IplImage *img_gs, std::vector<Candidate*>& kps, float minRad, float maxRad ) {

  // Normalize image scale
  CvMemStorage* storage = cvCreateMemStorage(0);
  cvSmooth( img_gs, img_gs, 2, 7, 7 );
  CvSeq* circles = cvHoughCircles( img_gs, storage, CV_HOUGH_GRADIENT, 10, minRad/2, 30, 10, minRad, maxRad );

  // Create cds
  for (int i = 0; i < circles->total; i++) {
    
    /*float* p = (float*)cvGetSeqElem( circles, i );         
    cvCircle( img, cvPoint(cvRound(p[0]),cvRound(p[1])),     
      3, CV_RGB(0,255,0), -1, 8, 0 );         
    cvCircle( img, cvPoint(cvRound(p[0]),cvRound(p[1])),              
      cvRound(p[2]), CV_RGB(255,0,0), 3, 8, 0 );   */

    float* p = (float*)cvGetSeqElem( circles, i );
    Candidate *kp = new Candidate;
    kp->c = p[0];
    kp->r = p[1];
    kp->major = p[2];
    kp->minor = p[2];
    kp->angle = 0;
    kps.push_back( kp );
  }
}
