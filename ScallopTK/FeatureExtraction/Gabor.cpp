
#include "Gabor.h"

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

CvMat *createGaborFilter( float sigma, float theta, float lambda, float psi, float gamma );

//------------------------------------------------------------------------------
//                                  Constants
//------------------------------------------------------------------------------

#define NUM_FILTERS 6
//int samppos[5][2] = { {32, 32}, {32, 17}, {17, 32}, {46, 32}, {32, 46} };

//------------------------------------------------------------------------------
//                                 Definitions
//------------------------------------------------------------------------------

// DEPRECATED
/*void performGaborFiltering( Candidate *cd ) {

  // Return if invalid
  if( !cd->is_active )
    return;

  // Create linear filters
  CvMat *filterBank[NUM_FILTERS];
  filterBank[0] = createGaborFilter( 1.2, 0.0, 6.4, 3.8, 1.97 );
  filterBank[1] = createGaborFilter( 1.2, PI/2, 6.4, 3.8, 1.97 );
  filterBank[2] = createGaborFilter( 1.2, PI/6, 6.4, 3.8, 1.97 );
  filterBank[3] = createGaborFilter( 0.4, 0.0, 2.4, 5.8, 1.23 );
  filterBank[4] = createGaborFilter( 1.2, PI/2, 7.4, 5.8, 2.47 );
  filterBank[5] = createGaborFilter( 1.8, PI/3, 5.4, 1.8, 2.17 );

  // Create images to store results
  IplImage *results[NUM_FILTERS];
  for( int i=0; i<NUM_FILTERS; i++ )
    results[i] = cvCreateImage( cvSize(64,64), IPL_DEPTH_32F, 1 );

  // Filter 64x64 images
  for( int i=0; i<NUM_FILTERS; i++ )
    cvFilter2D( cd->SummaryImage, results[i], filterBank[i] );

  // Collect results at designated points 
  int index = 0;
  const int entries = NUM_FILTERS * 5;
  float features[entries];
  for( int i=0; i<NUM_FILTERS; i++ ) {
    cvSmooth( results[i], results[i], CV_BLUR, 7, 7 );
    cd->GaborFeatures[index++] = ((float*)(results[i]->imageData+results[i]->widthStep*samppos[0][0]))[samppos[0][1]];
    cd->GaborFeatures[index++] = ((float*)(results[i]->imageData+results[i]->widthStep*samppos[1][0]))[samppos[1][1]];
    cd->GaborFeatures[index++] = ((float*)(results[i]->imageData+results[i]->widthStep*samppos[2][0]))[samppos[2][1]];
    cd->GaborFeatures[index++] = ((float*)(results[i]->imageData+results[i]->widthStep*samppos[3][0]))[samppos[3][1]];
    cd->GaborFeatures[index++] = ((float*)(results[i]->imageData+results[i]->widthStep*samppos[4][0]))[samppos[4][1]];
  }

  //showIP( unused, results[0], cd );
  //showImageRange( results[0] );

  // Deallocate results
  for( int i=0; i<NUM_FILTERS; i++ ) {
    cvReleaseImage( &results[i] );
    cvReleaseMat( &filterBank[i] );
  }
}*/

void calculateGaborFeatures( IplImage *img_gs_32f, vector<Candidate*>& cds ) {

  // Create linear filters
  CvMat *filterBank[NUM_FILTERS];
  filterBank[0] = createGaborFilter( 1.2, 0.0, 6.4, 3.8, 1.97 );
  filterBank[1] = createGaborFilter( 1.2, PI/2, 6.4, 3.8, 1.97 );
  filterBank[2] = createGaborFilter( 1.2, PI/6, 6.4, 3.8, 1.97 );
  filterBank[3] = createGaborFilter( 0.4, 0.0, 2.4, 5.8, 1.23 );
  filterBank[4] = createGaborFilter( 1.2, PI/2, 7.4, 5.8, 2.47 );
  filterBank[5] = createGaborFilter( 1.8, PI/3, 5.4, 1.8, 2.17 );

  // Create images to store results
  IplImage *results[NUM_FILTERS];
  for( int i=0; i<NUM_FILTERS; i++ )
    results[i] = cvCreateImage( cvGetSize(img_gs_32f), IPL_DEPTH_32F, 1 );

  // Filter images
  for( int i=0; i<NUM_FILTERS; i++ ) {
    cvFilter2D( img_gs_32f, results[i], filterBank[i] );
    cvSmooth( results[i], results[i], CV_BLUR, 5 );
  }

  // Compile vars for scan
  float *img_ptr[NUM_FILTERS];
  int fl_step = results[0]->widthStep / sizeof( float );
  for( int i=0; i<NUM_FILTERS; i++ ) {
    img_ptr[i] = (float*)results[i]->imageData;
  }
  int imwidth = results[0]->width;
  int imheight = results[0]->height;

  // Collect results at designated points 
  for( unsigned int i = 0; i < cds.size(); i++ ) {

    if( !cds[i]->is_active )
      continue;

    Candidate *cd = cds[i];
    int index = 0;
    const int entries = NUM_FILTERS * 5;
    for( int j=0; j<NUM_FILTERS; j++ ) {
      
      // Samp pos 1
      int r = cd->r;
      int c = cd->c;
      if( r > 0 && c > 0 && r < imheight && c < imwidth )
        cd->GaborFeatures[index++] = (img_ptr[j]+fl_step*r)[c];
      else 
        cd->GaborFeatures[index++] = 0.0f;

      // Samp pos 2
      r = cd->r + cd->major * 0.63;
      c = cd->c;
      if( r > 0 && c > 0 && r < imheight && c < imwidth )
        cd->GaborFeatures[index++] = (img_ptr[j]+fl_step*r)[c];
      else 
        cd->GaborFeatures[index++] = 0.0f;

      // Samp pos 3
      r = cd->r;
      c = cd->c + cd->major * 0.63;
      if( r > 0 && c > 0 && r < imheight && c < imwidth )
        cd->GaborFeatures[index++] = (img_ptr[j]+fl_step*r)[c];
      else 
        cd->GaborFeatures[index++] = 0.0f;

      // Samp pos 4
      r = cd->r;
      c = cd->c - cd->major * 0.63;
      if( r > 0 && c > 0 && r < imheight && c < imwidth )
        cd->GaborFeatures[index++] = (img_ptr[j]+fl_step*r)[c];
      else 
        cd->GaborFeatures[index++] = 0.0f;

      // Samp pos 5
      r = cd->r - cd->major * 0.63;
      c = cd->c;
      if( r > 0 && c > 0 && r < imheight && c < imwidth )
        cd->GaborFeatures[index++] = (img_ptr[j]+fl_step*r)[c];
      else 
        cd->GaborFeatures[index++] = 0.0f;

      // Samp pos 6
      r = cd->r + cd->major;
      c = cd->c;
      if( r > 0 && c > 0 && r < imheight && c < imwidth )
        cd->GaborFeatures[index++] = (img_ptr[j]+fl_step*r)[c];
      else 
        cd->GaborFeatures[index++] = 0.0f;

    }
  }

  // Deallocate results
  for( int i=0; i<NUM_FILTERS; i++ ) {
    cvReleaseImage( &results[i] );
    cvReleaseMat( &filterBank[i] );
  }
}

// Modeled after wikipedia entry
CvMat *createGaborFilter( float sigma, float theta, float lambda, float psi, float gamma ) {

  // Calculate Bounding Box
  float sigma_x = sigma;
  float sigma_y = sigma / gamma;
  float sint = sin( theta );
  float cost = cos( theta );
  int nstds = 3;
  float xmax32f = max( abs(nstds*sigma_x*cost), abs(nstds*sigma_y*sint) );
  int xmax = ceil( max(xmax32f,1.0f) );
  float ymax32f = max( abs(nstds*sigma_x*sint), abs(nstds*sigma_y*cost) );
  int ymax = ceil( max(ymax32f,1.0f) );  

  // Create Matrix 
  int width = 2*ymax+1;
  int height = 2*xmax + 1;
  int cr = ymax + 1;
  int cc = xmax + 1;
  CvMat *output = cvCreateMat( height, width, CV_32FC1 );
  float *out_ptr = (float*)output->data.ptr;
  assert( output->step == width*4 );

  // Fill in Matrix values
  float front = 1/(PI*2*sigma_x*sigma_y);
  float sigxsq = sigma_x*sigma_x;
  float sigysq = sigma_y*sigma_y;
  for( int r=0; r<height; r++ ) {
    for( int c=0; c<width; c++ ) {
      float x = c-cc;
      float y = r-cr;
      float x_theta = x*cost+y*sint;
      float y_theta = -x*sint+y*cost;
      float value = front * exp( -0.5f*(x_theta*x_theta/(sigxsq) + y_theta*y_theta/sigysq) );
      value = value * cos( 2*PI/lambda*x_theta+psi );
      out_ptr[0] = value;
      out_ptr = out_ptr + 1;
    }
  }

  return output;
}
