//------------------------------------------------------------------------------
// Title: DoG.cpp
// Description: Difference of Gaussian Candidate Detection, a modified 
//              version of Rob Hess's OpenCV SIFT
//------------------------------------------------------------------------------

#include "DoG.h"

namespace ScallopTK
{

//------------------------------------------------------------------------------
//                         Internal Function Prototypes
//------------------------------------------------------------------------------

//Functions for creating cross-channel trapezoidal DoG structure
IplImage* formatBase( IplImage* img, float sigma, bool upscale, float maxRad );
IplImage* downsample( IplImage* img );
IplImage*** buildGaussTrap( IplImage* base, int octvs, int intvls, double sigma );
IplImage*** buildDoGTrap( IplImage*** gaussTrap, int octvs, int intvls );
IplImage*** mergeDoGChannels( IplImage*** DoGTrap, int octvs, int intvls );
IplImage*** absDoGChannels( IplImage*** DoGTrap, int octvs, int intvls );

//Candidate selection and filtering
void detectExtremum( IplImage*** DoGTrap, int octvs, int intvls, double contr_thr, int curv_thr, CandidatePtrVector& kps, float minRad, float maxRad  );
void detectMinExtremum( IplImage*** DoGTrap, int octvs, int intvls, double contr_thr, int curv_thr, CandidatePtrVector& kps, float minRad, float maxRad  );
void detectMaxExtremum( IplImage*** DoGTrap, int octvs, int intvls, double contr_thr, int curv_thr, CandidatePtrVector& kps, float minRad, float maxRad  );
bool interpExtremum( IplImage*** DoGTrap, int octv, int intvl, int r, int c, int intvls, double contr_thr, DoG_Candidate& point );
void interpStep( IplImage*** DoGTrap, int octv, int intvl, int r, int c, double* xi, double* xr, double* xc );
CvMat* deriv3D( IplImage*** DoGTrap, int octv, int intvl, int r, int c );
CvMat* hessian3D( IplImage*** DoGTrap, int octv, int intvl, int r, int c );
double interpContr( IplImage*** DoGTrap, int octv, int intvl, int r, int c, double xi, double xr, double xc );
int isTooEdgeLike( IplImage* DoG, int r, int c, int curv_thr );
void calcFeatureScales( CandidatePtrVector& kps, double sigma, int intvls );

//Orientation Assignment
double dominantOrientation( double* hist, int n );
void smoothHistogram( double* hist, int n );
int calcGradientMagOri( IplImage* img, int r, int c, double* mag, double* ori );
double* orientationHist( IplImage* img, int r, int c, int n, int rad, double sigma);
void calculateOrientation( CandidatePtrVector& kps, IplImage*** gaussTrap );
void addOriToFeature( double* hist, int n, double mag_thr, Candidate* point, CandidatePtrVector& kps );

//Helper functions
void adjustForScale( CandidatePtrVector& kps, bool upscale, float maxRad );
void releaseTrap( IplImage**** pyr, int octvs, int n );

//------------------------------------------------------------------------------
//                       Main DoG Function Definition
//------------------------------------------------------------------------------

bool findDoGCandidates( IplImage* input, CandidatePtrVector& kps, float minRad, float maxRad, int mode ) {

  //Calculate gaussian trapezoid characteristics
  bool upscale = (minRad < DOG_UPSCALE_THRESHOLD ? 1 : 0);
  float sigma = DOG_SIGMA;
  int octaves = (int)(log(maxRad/sigma)/log(2.0f)) + 1;
  int intervals = DOG_INTERVALS_PER_OCT;

  //Format the base of each gaussian trapezoid (1 for each channel)
  IplImage* init = formatBase( input, sigma, upscale, maxRad );

  //Compensate scanning radii
  minRad = minRad / DOG_COMPENSATION;

  //Build gaussian trapezoid
  IplImage*** gaussTrap = buildGaussTrap( init, octaves, intervals, sigma );  
  IplImage*** DoGTrap = buildDoGTrap( gaussTrap, octaves, intervals );

  //Find Candidates
  if( mode == DOG_MIN ) {
    detectMinExtremum( DoGTrap, octaves, intervals, 0.04f, 10, kps, minRad, maxRad );  
  } else if( mode == DOG_MAX ) {
    detectMaxExtremum( DoGTrap, octaves, intervals, 0.04f, 10, kps, minRad, maxRad );  
  } else {
    detectExtremum( DoGTrap, octaves, intervals, 0.04f, 10, kps, minRad, maxRad );
  }
  
  //Adjust Candidates for scale & border
  adjustForScale( kps, upscale, maxRad );

  //Deallocate variables used
  releaseTrap( &gaussTrap, octaves, intervals + 3 );
  releaseTrap( &DoGTrap, octaves, intervals + 2 );
  cvReleaseImage(&init);  
  return true;
}

//------------------------------------------------------------------------------
//                          DoG Trapezoid Formation
//------------------------------------------------------------------------------

//Double if necessary and smooth to base of first octave
IplImage* formatBase( IplImage* img, float sigma, bool upscale, float maxRad )
{
  // Add border of maxRad size around the image 
  // (to simply extrema searching near image edges)
  CvSize newSize;
  newSize.height = img->height + maxRad;
  newSize.width = img->width + maxRad;
  IplImage* base = cvCreateImage( newSize, img->depth, img->nChannels );
  float sig_diff;
  CvPoint offset;
  offset.x = maxRad / 2;
  offset.y = maxRad / 2;
  CvScalar median;
  median.val[0] = quickMedian( img, 1000 );
  cvCopyMakeBorder( img, base, offset, IPL_BORDER_CONSTANT, median );

  // If we need to upscale the image do so
  if( upscale ) {
    sig_diff = (float)sqrt( sigma * sigma - DOG_INIT_SIGMA * DOG_INIT_SIGMA * 4 );
    IplImage *upped = cvCreateImage( cvSize( img->width*2, img->height*2 ),
      IPL_DEPTH_32F, img->nChannels );
    cvResize( base, upped, CV_INTER_CUBIC );
    cvSmooth( upped, upped, CV_GAUSSIAN, 0, 0, sig_diff, sig_diff );
    cvReleaseImage( &base );
    base = upped;
  } else {
    sig_diff = (float)sqrt( sigma * sigma - DOG_INIT_SIGMA * DOG_INIT_SIGMA );
    cvSmooth( base, base, CV_GAUSSIAN, 0, 0, sig_diff, sig_diff );
  }
  return base;
}

//Builds a gaussian trapezoid so that we can take DoGs
IplImage*** buildGaussTrap( IplImage* base, int octvs, int intvls, double sigma )
{
  // Declare required variables
  IplImage*** gaussTrap;
  double* sig = (double *)calloc( intvls + 3, sizeof(double));
  int i, o;

  // Allocated memory for trapezoidal structure
  gaussTrap = (IplImage ***) calloc( octvs, sizeof( IplImage** ) );
  for( i = 0; i < octvs; i++ )
    gaussTrap[i] = (IplImage **)calloc( intvls + 3, sizeof( IplImage* ) );

  // Create array of smoothing increments for each interval
  sig[0] = sigma;
  double k = pow( 2.0, 1.0 / intvls );
  for( i = 1; i < intvls + 3; i++ )
  {
    double sig_prev = pow( k, i - 1 ) * sigma;
    double sig_total = sig_prev * k;
    sig[i] = sqrt( sig_total * sig_total - sig_prev * sig_prev );
  }

  // Create images for each octave and interval
  for( o = 0; o < octvs; o++ )
    for( i = 0; i < intvls + 3; i++ )
    {
      if( o == 0  &&  i == 0 )
        gaussTrap[o][i] = cvCloneImage(base);

      /* base of new octvave is halved image from end of previous octave */
      else if( i == 0 )
        gaussTrap[o][i] = downsample( gaussTrap[o-1][intvls] );

      /* blur the current octave's last image to create the next one */
      else {
        gaussTrap[o][i] = cvCreateImage( cvGetSize(gaussTrap[o][i-1]),
          IPL_DEPTH_32F, gaussTrap[o][i-1]->nChannels );
        cvSmooth( gaussTrap[o][i-1], gaussTrap[o][i],
          CV_GAUSSIAN, 0, 0, sig[i], sig[i] );
      }
    }
  free( sig );
  return gaussTrap;
}

//Downsample an image by half
IplImage* downsample( IplImage* img )
{
  IplImage* smaller = cvCreateImage( cvSize(img->width / 2, img->height / 2),
    img->depth, img->nChannels );
  cvResize( img, smaller, CV_INTER_NN );
  return smaller;
}


//Takes the DoG from our pre-built gaussian trap
IplImage*** buildDoGTrap( IplImage*** GaussTrap, int octvs, int intvls )
{
  IplImage*** DoGTrap;

  DoGTrap = (IplImage ***) calloc( octvs, sizeof( IplImage** ) );
  for( int i = 0; i < octvs; i++ )
    DoGTrap[i] = (IplImage **)calloc( intvls + 2, sizeof(IplImage*) );

  for( int o = 0; o < octvs; o++ ) {
    for( int i = 0; i < intvls + 2; i++ ) {
      DoGTrap[o][i] = cvCreateImage( cvGetSize(GaussTrap[o][i]), IPL_DEPTH_32F, GaussTrap[o][i]->nChannels );
      cvSub( GaussTrap[o][i+1], GaussTrap[o][i], DoGTrap[o][i], NULL );
      //showImageRange( DoGTrap[o][i] );
    }
  }
  return DoGTrap;
}

//Merges DoG channels
IplImage*** mergeDoGChannels( IplImage*** DoGTrap, int octvs, int intvls )
{
  IplImage*** FullDoG;

  FullDoG = (IplImage ***) calloc( octvs, sizeof( IplImage** ) );
  for( int i = 0; i < octvs; i++ )
    FullDoG[i] = (IplImage **)calloc( intvls + 2, sizeof(IplImage*) );
  
  for( int o = 0; o < octvs; o++ ) {
    for( int i = 0; i < intvls + 2; i++ ) {
      //Split channels
      FullDoG[o][i] = cvCreateImage( cvGetSize(DoGTrap[o][i]), IPL_DEPTH_32F, 1 );
      IplImage* imgL = cvCreateImage( cvGetSize(DoGTrap[o][i]), IPL_DEPTH_32F, 1 );
      IplImage* imgA = cvCreateImage( cvGetSize(DoGTrap[o][i]), IPL_DEPTH_32F, 1 );
      IplImage* imgB = cvCreateImage( cvGetSize(DoGTrap[o][i]), IPL_DEPTH_32F, 1 );
      cvSplit( DoGTrap[o][i], imgL, imgA, imgB, NULL );
      //average3CF( DoGTrap[o][i] );
      //Take absolute value of each channel
      cvAbs(imgL, imgL);
      cvAbs(imgA, imgA);
      cvAbs(imgB, imgB);

      //Sum each channel
      cvAdd( imgL, imgA, FullDoG[o][i] );
      cvAdd( FullDoG[o][i], imgB, FullDoG[o][i] );
      cvReleaseImage(&imgL);
      cvReleaseImage(&imgA);
      cvReleaseImage(&imgB);
    }
  }

  return FullDoG;
}

//Takes absolute value of DoG channels
IplImage*** absDoGChannels( IplImage*** DoGTrap, int octvs, int intvls )
{
  IplImage*** FullDoG;

  FullDoG = (IplImage ***) calloc( octvs, sizeof( IplImage** ) );
  for( int i = 0; i < octvs; i++ )
    FullDoG[i] = (IplImage **)calloc( intvls + 2, sizeof(IplImage*) );
  
  for( int o = 0; o < octvs; o++ ) {
    for( int i = 0; i < intvls + 2; i++ ) {
      //Split channels
      FullDoG[o][i] = cvCreateImage( cvGetSize(DoGTrap[o][i]), IPL_DEPTH_32F, 1 );
      //average3CF( DoGTrap[o][i] );
      //Take absolute value of each channel
      cvAbs( DoGTrap[o][i], FullDoG[o][i] );
      
    }
  }

  return FullDoG;
}

//------------------------------------------------------------------------------
//                    Extrema Localization and Filtering
//------------------------------------------------------------------------------

int isMax( IplImage*** DoGTrap, int octv, int intvl, int r, int c )
{
  float val = getPixel32f( DoGTrap[octv][intvl], r, c );
  int i, j, k;

  /* check for maximum */
  for( i = -1; i <= 1; i++ )
    for( j = -1; j <= 1; j++ )
      for( k = -1; k <= 1; k++ )
        if( val > getPixel32f( DoGTrap[octv][intvl + i], r + j, c + k ) )
          return false;


  return true;
}

int isMin( IplImage*** DoGTrap, int octv, int intvl, int r, int c )
{
  float val = getPixel32f( DoGTrap[octv][intvl], r, c );
  int i, j, k;

  /* check for maximum */
  for( i = -1; i <= 1; i++ )
    for( j = -1; j <= 1; j++ )
      for( k = -1; k <= 1; k++ )
        if( val < getPixel32f( DoGTrap[octv][intvl + i], r + j, c + k ) )
          return false;


  return true;
}

//Detect and Filter Scale Space Extrenum
void detectMinExtremum( IplImage*** DoGTrap, int octvs, int intvls, double contr_thr, int curv_thr, CandidatePtrVector& kps, float minRad, float maxRad )
{
  for( int o = 0; o < octvs; o++ ) {
    for( int i = 1; i <= intvls; i++ ) {

      float nextScaleSigma = 2*DOG_SIGMA*pow(2.0f,o+(i+1)/intvls);
      float prevScaleSigma = 2*DOG_SIGMA*pow(2.0f,o+(i-1)/intvls);

      if( nextScaleSigma <= minRad || prevScaleSigma >= maxRad )
        continue;

      for( int r = DOG_SCAN_START; r < DoGTrap[o][i]->height-DOG_SCAN_START; r++ ) {
        for( int c = DOG_SCAN_START; c < DoGTrap[o][i]->width-DOG_SCAN_START; c++ ) {
          if( isMin( DoGTrap, o, i, r, c ) )  
          {  
            DoG_Candidate point;
            if( interpExtremum(DoGTrap, o, i, r, c, intvls, contr_thr, point) ) 
            {
              Candidate* to_add = new Candidate;
              to_add->r = point.y;
              to_add->c = point.x;
              to_add->major = DOG_COMPENSATION*DOG_SIGMA*
                pow(2.0f,point.octv+(point.intvl+point.subintvl)/intvls);
              to_add->minor = to_add->major;
              to_add->angle = 0.0;
              to_add->method = DOG;
              to_add->magnitude = getPixel32f( DoGTrap[o][i], r, c );
              kps.push_back( to_add );
            }
          }
        }
      }
    }
  }
}

//Detect and Filter Scale Space Extrenum
void detectMaxExtremum( IplImage*** DoGTrap, int octvs, int intvls, double contr_thr, int curv_thr, CandidatePtrVector& kps, float minRad, float maxRad )
{
  for( int o = 0; o < octvs; o++ ) {
    for( int i = 1; i <= intvls; i++ ) {

      float nextScaleSigma = 2*DOG_SIGMA*pow(2.0f,o+(i+1)/intvls);
      float prevScaleSigma = 2*DOG_SIGMA*pow(2.0f,o+(i-1)/intvls);

      if( nextScaleSigma <= minRad || prevScaleSigma >= maxRad )
        continue;

      for( int r = DOG_SCAN_START; r < DoGTrap[o][i]->height-DOG_SCAN_START; r++ ) {
        for( int c = DOG_SCAN_START; c < DoGTrap[o][i]->width-DOG_SCAN_START; c++ ) {
          if( isMax( DoGTrap, o, i, r, c ) )  
          {  
            DoG_Candidate point;
            if( interpExtremum(DoGTrap, o, i, r, c, intvls, contr_thr, point) ) 
            {
              Candidate* to_add = new Candidate;
              to_add->r = point.y;
              to_add->c = point.x;
              to_add->major = DOG_COMPENSATION*DOG_SIGMA*
                pow(2.0f,point.octv+(point.intvl+point.subintvl)/intvls);
              to_add->minor = to_add->major;
              to_add->angle = 0.0;
              to_add->method = DOG;
              to_add->magnitude = getPixel32f( DoGTrap[o][i], r, c );
              kps.push_back( to_add );
            }
          }
        }
      }
    }
  }
}

//Detect and Filter Scale Space Extrenum
void detectExtremum( IplImage*** DoGTrap, int octvs, int intvls, double contr_thr, int curv_thr, CandidatePtrVector& kps, float minRad, float maxRad )
{
  for( int o = 0; o < octvs; o++ ) {
    for( int i = 1; i <= intvls; i++ ) {

      float nextScaleSigma = 2*DOG_SIGMA*pow(2.0f,o+(i+1)/intvls);
      float prevScaleSigma = 2*DOG_SIGMA*pow(2.0f,o+(i-1)/intvls);

      if( nextScaleSigma <= minRad || prevScaleSigma >= maxRad )
        continue;

      for( int r = DOG_SCAN_START; r < DoGTrap[o][i]->height-DOG_SCAN_START; r++ ) {
        for( int c = DOG_SCAN_START; c < DoGTrap[o][i]->width-DOG_SCAN_START; c++ ) {
          if( isMin( DoGTrap, o, i, r, c ) || isMax( DoGTrap, o, i, r, c ) )  
          {  
            DoG_Candidate point;
            if( interpExtremum(DoGTrap, o, i, r, c, intvls, contr_thr, point) ) 
            {
              Candidate* to_add = new Candidate;
              to_add->r = point.y;
              to_add->c = point.x;
              to_add->major = DOG_COMPENSATION*DOG_SIGMA*
                pow(2.0f,point.octv+(point.intvl+point.subintvl)/intvls);
              to_add->minor = to_add->major;
              to_add->angle = 0.0;
              to_add->method = DOG;
              to_add->magnitude = getPixel32f( DoGTrap[o][i], r, c );
              kps.push_back( to_add );
            }
          }
        }
      }
    }
  }
}

bool interpExtremum( IplImage*** DoGTrap, int octv, int intvl, int r, int c, int intvls, double contr_thr, DoG_Candidate& point )
{
  double xi, xr, xc, contr;
  int i = 0;

  while( i < DOG_MAX_INTERP_STEPS )
  {
    interpStep( DoGTrap, octv, intvl, r, c, &xi, &xr, &xc );
    if( abs( xi ) < 0.5  &&  abs( xr ) < 0.5  &&  abs( xc ) < 0.5 )
      break;

    c += cvRound( xc );
    r += cvRound( xr );
    intvl += cvRound( xi );

    if( intvl < 1  ||
      intvl > intvls  ||
      c < DOG_SCAN_START  ||
      r < DOG_SCAN_START  ||
      c >= DoGTrap[octv][0]->width - DOG_SCAN_START  ||
      r >= DoGTrap[octv][0]->height - DOG_SCAN_START )
    {
      return false;
    }

    i++;
  }

  /* ensure convergence of interpolation */
  //if( i >= DOG_MAX_INTERP_STEPS )
  //  return false;

  /*contr = interpContr( DoGTrap, octv, intvl, r, c, xi, xr, xc );
  if( abs( contr ) < contr_thr / intvls )
    return false;*/

  point.x = ( c + xc ) * pow( 2.0f, octv );
  point.y = ( r + xr ) * pow( 2.0f, octv );
  point.r = r;
  point.c = c;
  point.octv = octv;
  point.intvl = intvl;
  point.subintvl = xi;

  return true;
}

//Interprets Step
void interpStep( IplImage*** DoGTrap, int octv, int intvl, int r, int c,
             double* xi, double* xr, double* xc )
{
  CvMat* dD, * H, * H_inv, X;
  double x[3] = { 0 };

  dD = deriv3D( DoGTrap, octv, intvl, r, c );
  H = hessian3D( DoGTrap, octv, intvl, r, c );
  H_inv = cvCreateMat( 3, 3, CV_64FC1 );
  cvInvert( H, H_inv, CV_SVD );
  cvInitMatHeader( &X, 3, 1, CV_64FC1, x, CV_AUTOSTEP );
  cvGEMM( H_inv, dD, -1, NULL, 0, &X, 0 );

  cvReleaseMat( &dD );
  cvReleaseMat( &H );
  cvReleaseMat( &H_inv );

  *xi = x[2];
  *xr = x[1];
  *xc = x[0];
}

//Creates a derivative OpenCV matrix
CvMat* deriv3D( IplImage*** DoGTrap, int octv, int intvl, int r, int c )
{
  CvMat* dI;
  double dx, dy, ds;

  dx = ( getPixel32f( DoGTrap[octv][intvl], r, c+1 ) -
    getPixel32f( DoGTrap[octv][intvl], r, c-1 ) ) / 2.0;
  dy = ( getPixel32f( DoGTrap[octv][intvl], r+1, c ) -
    getPixel32f( DoGTrap[octv][intvl], r-1, c ) ) / 2.0;
  ds = ( getPixel32f( DoGTrap[octv][intvl+1], r, c ) -
    getPixel32f( DoGTrap[octv][intvl-1], r, c ) ) / 2.0;

  dI = cvCreateMat( 3, 1, CV_64FC1 );
  cvmSet( dI, 0, 0, dx );
  cvmSet( dI, 1, 0, dy );
  cvmSet( dI, 2, 0, ds );

  return dI;
}

//Creates a Hessian OpenCV matrix
CvMat* hessian3D( IplImage*** DoGTrap, int octv, int intvl, int r, int c )
{
  CvMat* H;
  double v, dxx, dyy, dss, dxy, dxs, dys;

  v = getPixel32f( DoGTrap[octv][intvl], r, c );
  dxx = ( getPixel32f( DoGTrap[octv][intvl], r, c+1 ) + 
      getPixel32f( DoGTrap[octv][intvl], r, c-1 ) - 2 * v );
  dyy = ( getPixel32f( DoGTrap[octv][intvl], r+1, c ) +
      getPixel32f( DoGTrap[octv][intvl], r-1, c ) - 2 * v );
  dss = ( getPixel32f( DoGTrap[octv][intvl+1], r, c ) +
      getPixel32f( DoGTrap[octv][intvl-1], r, c ) - 2 * v );
  dxy = ( getPixel32f( DoGTrap[octv][intvl], r+1, c+1 ) -
      getPixel32f( DoGTrap[octv][intvl], r+1, c-1 ) -
      getPixel32f( DoGTrap[octv][intvl], r-1, c+1 ) +
      getPixel32f( DoGTrap[octv][intvl], r-1, c-1 ) ) / 4.0;
  dxs = ( getPixel32f( DoGTrap[octv][intvl+1], r, c+1 ) -
      getPixel32f( DoGTrap[octv][intvl+1], r, c-1 ) -
      getPixel32f( DoGTrap[octv][intvl-1], r, c+1 ) +
      getPixel32f( DoGTrap[octv][intvl-1], r, c-1 ) ) / 4.0;
  dys = ( getPixel32f( DoGTrap[octv][intvl+1], r+1, c ) -
      getPixel32f( DoGTrap[octv][intvl+1], r-1, c ) -
      getPixel32f( DoGTrap[octv][intvl-1], r+1, c ) +
      getPixel32f( DoGTrap[octv][intvl-1], r-1, c ) ) / 4.0;

  H = cvCreateMat( 3, 3, CV_64FC1 );
  cvmSet( H, 0, 0, dxx );
  cvmSet( H, 0, 1, dxy );
  cvmSet( H, 0, 2, dxs );
  cvmSet( H, 1, 0, dxy );
  cvmSet( H, 1, 1, dyy );
  cvmSet( H, 1, 2, dys );
  cvmSet( H, 2, 0, dxs );
  cvmSet( H, 2, 1, dys );
  cvmSet( H, 2, 2, dss );

  return H;
}

double interpContr( IplImage*** DoGTrap, int octv, int intvl, int r,
              int c, double xi, double xr, double xc )
{
  CvMat* dD, X, T;
  double t[1], x[3] = { xc, xr, xi };

  cvInitMatHeader( &X, 3, 1, CV_64FC1, x, CV_AUTOSTEP );
  cvInitMatHeader( &T, 1, 1, CV_64FC1, t, CV_AUTOSTEP );
  dD = deriv3D( DoGTrap, octv, intvl, r, c );
  cvGEMM( dD, &X, 1, NULL, 0, &T,  CV_GEMM_A_T );
  cvReleaseMat( &dD );

  return getPixel32f( DoGTrap[octv][intvl], r, c ) + t[0] * 0.5;
}

//Use hessian to quell edge responses
int isTooEdgeLike( IplImage* DoG, int r, int c, int curv_thr )
{
  double d, dxx, dyy, dxy, tr, det;

  // principal curvatures are computed using the trace and det of Hessian
  d = getPixel32f(DoG, r, c);
  dxx = getPixel32f( DoG, r, c+1 ) + getPixel32f( DoG, r, c-1 ) - 2 * d;
  dyy = getPixel32f( DoG, r+1, c ) + getPixel32f( DoG, r-1, c ) - 2 * d;
  dxy = ( getPixel32f(DoG, r+1, c+1) - getPixel32f(DoG, r+1, c-1) -
      getPixel32f(DoG, r-1, c+1) + getPixel32f(DoG, r-1, c-1) ) / 4.0;
  tr = dxx + dyy;
  det = dxx * dyy - dxy * dxy;

  // negative determinant -> curvatures have different signs; reject feature
  if( det <= 0 )
    return true;

  if( tr * tr / det < ( curv_thr + 1.0 )*( curv_thr + 1.0 ) / curv_thr )
    return false;
  return true;
}

//Calculate Candidate scales at the subscale level
void calcFeatureScales( CandidatePtrVector& kps, double sigma, int intvls )
{
  for( unsigned int i=0; i < kps.size(); i++ )
  {
    /*double intvl = kps[i]->intvl + kps[i]->subintvl;
    kps[i]->scl = 2 * sigma * pow( 2.0, kps[i]->octv + intvl / intvls );
    kps[i]->scl_octv = sigma * pow( 2.0, intvl / intvls );*/
  }
}

//Adjust scales if we initially upscaled our base image
void adjustForScale( CandidatePtrVector& kps, bool upscale, float maxRad )
{
  if( upscale ) {
    for( unsigned int i = 0; i < kps.size(); i++ )
    {
      kps[i]->r = kps[i]->r / 2.0 - (int)(maxRad / 2);
      kps[i]->c = kps[i]->c / 2.0 - (int)(maxRad / 2);
      kps[i]->major /= 2.0;
      kps[i]->minor /= 2.0;
    }
  } else {
    for( unsigned int i = 0; i < kps.size(); i++ )
    {
      kps[i]->r = kps[i]->r - (int)(maxRad / 2);
      kps[i]->c = kps[i]->c - (int)(maxRad / 2);
    }
  }
}

//------------------------------------------------------------------------------
//                        SIFT Orientation Assignment
//------------------------------------------------------------------------------

#ifdef UNUSED_FOR_NOW

//Calculate Candidate orientation
void calculateOrientation( CandidatePtrVector& kps, IplImage*** gaussTrap )
{
  /*
  unsigned int init_size = kps.size();
  for( unsigned int i=0; i < init_size; i++ )
  {
    double* hist = orientationHist( gaussTrap[kps[i]->octv][kps[i]->intvl],
            kps[i]->r, kps[i]->c, SIFT_ORI_HIST_BINS,
            cvRound( SIFT_ORI_RADIUS * kps[i]->scl_octv ),
            SIFT_ORI_SIG_FCTR * kps[i]->scl_octv );

    for( int j = 0; j < SIFT_ORI_SMOOTH_PASSES; j++ )
      smoothHistogram( hist, SIFT_ORI_HIST_BINS );

    double omax = dominantOrientation( hist, SIFT_ORI_HIST_BINS );

    addOriToFeature( hist, SIFT_ORI_HIST_BINS, omax * SIFT_ORI_PEAK_RATIO, kps[i], kps );
    free( hist );
  }*/
}

void addOriToFeature( double* hist, int n, double mag_thr, Candidate* point, CandidatePtrVector& kps )
{
  /*double PI2 = CV_PI * 2.0;
  bool first = true;

  for( int i = 0; i < n; i++ )
  {
    int l = ( i == 0 )? n - 1 : i-1;
    int r = ( i + 1 ) % n;

    if( hist[i] > hist[l]  &&  hist[i] > hist[r]  &&  hist[i] >= mag_thr )
    {
      double bin = i + ( 0.5 * (hist[l]-hist[r]) / (hist[l] - 2.0*hist[i] + hist[r]) );
      bin = ( bin < 0 )? n + bin : ( bin >= n )? bin - n : bin;

      //Add new feature
      if( first ) {
        point->ori = ( ( PI2 * bin ) / n ) - CV_PI;
        first = false;
      } else {
        Candidate *newkp = copyCandidate( point );
        newkp->ori = ( ( PI2 * bin ) / n ) - CV_PI;
        kps.push_back( newkp );
      }
    }
  }*/
}

//Create orientation histogram
double* orientationHist( IplImage* img, int r, int c, int n, int rad, double sigma)
{
  //Variables
  double PI2 = CV_PI * 2.0;
  double mag, ori, w;
  int bin;
  double* hist = (double*)calloc( n, sizeof( double ) );
  double exp_denom = 2.0 * sigma * sigma;

  //Voting
  for( int i = -rad; i <= rad; i++ ) {
    for( int j = -rad; j <= rad; j++ ) {
      if( calcGradientMagOri( img, r + i, c + j, &mag, &ori ) )
      {
        w = exp( -( i*i + j*j ) / exp_denom );
        bin = cvRound( n * ( ori + CV_PI ) / PI2 );
        bin = ( bin < n )? bin : 0;
        hist[bin] += w * mag;
      }
    }
  }

  return hist;
}

//Calculate local gradient magnitude and orientation
int calcGradientMagOri( IplImage* img, int r, int c, double* mag, double* ori )
{
  double dx, dy;

  if( r > 0  &&  r < img->height - 1  &&  c > 0  &&  c < img->width - 1 )  {
    dx = getPixel32f( img, r, c+1 ) - getPixel32f( img, r, c-1 );
    dy = getPixel32f( img, r-1, c ) - getPixel32f( img, r+1, c );
    *mag = sqrt( dx*dx + dy*dy );
    *ori = atan2( dy, dx );
    return true;
  } else {
    return false;
  }
}

void smoothHistogram( double* hist, int n )
{
  double prev, tmp, h0 = hist[0];
  int i;

  prev = hist[n-1];
  for( i = 0; i < n; i++ )
  {
    tmp = hist[i];
    hist[i] = 0.25 * prev + 0.5 * hist[i] + 
      0.25 * ( ( i+1 == n )? h0 : hist[i+1] );
    prev = tmp;
  }
}

double dominantOrientation( double* hist, int n )
{
  double omax;
  int maxbin, i;

  omax = hist[0];
  maxbin = 0;
  for( i = 1; i < n; i++ ) {
    if( hist[i] > omax ) {
      omax = hist[i];
      maxbin = i;
    }
  }
  return omax;
}

#endif

//------------------------------------------------------------------------------
//                             Memory Management
//------------------------------------------------------------------------------

//Deallocate a trapezoidal structure
void releaseTrap( IplImage**** pyr, int octvs, int n )
{
  if( *pyr == NULL ) 
    return;

  int i, j;
  for( i = 0; i < octvs; i++ )
  {
    for( j = 0; j < n; j++ )
      cvReleaseImage( &(*pyr)[i][j] );
    free( (*pyr)[i] );
  }
  free( *pyr );
  *pyr = NULL;
}

}

