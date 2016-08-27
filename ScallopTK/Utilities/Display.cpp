//------------------------------------------------------------------------------
// Title: display.cpp
// Author: Matthew Dawkins
//------------------------------------------------------------------------------

#include "Display.h"

namespace ScallopTK
{

void initOutputDisplay() {
  cvNamedWindow( DISPLAY_WINDOW_NAME.c_str(), CV_WINDOW_AUTOSIZE );
}

void killOuputDisplay() {
  cvDestroyWindow( DISPLAY_WINDOW_NAME.c_str() );
}

IplImage* resizeImage( IplImage* img ) {
  float sf = 1.0f;

  if( img->height > DISPLAY_MAX_HEIGHT ) {
    sf = std::min( sf, DISPLAY_MAX_HEIGHT / img->height );
  }
  if( img->width > DISPLAY_MAX_HEIGHT ) {
    sf = std::max( sf, DISPLAY_MAX_WIDTH / img->width );
  }

  IplImage *output = cvCreateImage(
    cvSize( img->width * sf, img->height * sf ), img->depth, img->nChannels );

  cvResize( img, output );
  return output;
}

void displayImage( IplImage* img ) {
  IplImage *resized = resizeImage( img );
  cvShowImage( DISPLAY_WINDOW_NAME.c_str(), resized );
  cvWaitKey( 15 );
  cvReleaseImage(&resized);
}

void displayInterestPointImage( IplImage* img, CandidatePtrVector& cds ) {
  IplImage* local = cvCloneImage( img );
  for( unsigned int i=0; i<cds.size(); i++ ) {
    if( cds[i] == NULL ) continue;
    CvScalar colour;
    if( cds[i]->method == ADAPTIVE ) {
      colour = cvScalar( 1.0, 1.0, 0 );
    } else if ( cds[i]->method == DOG ) {
      colour = cvScalar( 0.0, 1.0, 0 );
    } else if ( cds[i]->method == TEMPLATE ) { 
      colour = cvScalar( 1.0, 0.0, 0.0 );
    } else if ( cds[i]->method == CANNY ) {
      colour = cvScalar( 1.0, 0, 1.0 );
    } else {
      colour = cvScalar( 0.0, 0.0, 1.0 );
    }
    cvEllipse(local, cvPoint( (int)cds[i]->c, (int)cds[i]->r ), 
      cvSize( cds[i]->minor, cds[i]->major ), 
      cds[i]->angle, 0, 360, colour, 1 );
  }
  displayImage( local );
  cvReleaseImage( &local );
}

void displayResultsImage( IplImage* img, CandidatePtrVector& cds ) {
  IplImage* local = cvCloneImage( img );
  for( unsigned int i=0; i<cds.size(); i++ ) {  
    CvScalar color;
    if( cds[i]->classification == SCALLOP_BROWN )
      color = cvScalar(0,1,0); 
    else if( cds[i]->classification == SCALLOP_WHITE )
      color = cvScalar(1,1,1); 
    else if( cds[i]->classification == SCALLOP_BURIED )
      color = cvScalar(1,0,0); 
    else 
      continue;
    cvEllipse(local, cvPoint( (int)cds[i]->c, (int)cds[i]->r ), 
      cvSize( cds[i]->major, cds[i]->minor ), 
      (cds[i]->angle*180/PI)-90, 0, 360, color, 1 );
  }
  displayImage( local );
  cvReleaseImage( &local );
}

void displayResultsImage( IplImage* img, DetectionPtrVector& cds, string Filename )
{
  IplImage* local = cvCloneImage( img );
  for( unsigned int i=0; i<cds.size(); i++ ) {  
    CvScalar color;
    if( cds[i]->isBrownScallop )
      color = cvScalar(0,1,0); 
    else if( cds[i]->isWhiteScallop )
      color = cvScalar(1,1,1); 
    else if( cds[i]->isBuriedScallop )
      color = cvScalar(0.8,0.5,0.2); 
    else if( cds[i]->isSandDollar )
      color = cvScalar(0,0,1);
    else 
      color = cvScalar(0.1,0.5,0.6);

    cvEllipse(local, cvPoint( (int)cds[i]->c, (int)cds[i]->r ), 
      cvSize( cds[i]->major, cds[i]->minor ), 
      (cds[i]->angle*180/PI)-90, 0, 360, color, 2 );
  }
  displayImage( local );
  cvReleaseImage( &local );
}

}
