//------------------------------------------------------------------------------
// Title: display.cpp
// Author: Matthew Dawkins
//------------------------------------------------------------------------------

#include "Display.h"

void initOutputDisplay() {
  cvNamedWindow( "ScallopDisplayWindow", CV_WINDOW_AUTOSIZE );
}

void killOuputDisplay() {
  cvDestroyWindow( "ScallopDisplayWindow" );
}

void displayImage( IplImage* img, string wname ) {
  IplImage *nimg = cvCreateImage( cvSize( DISPLAY_WIDTH, DISPLAY_HEIGHT ), img->depth, img->nChannels );
  cvResize( img, nimg );
  cvShowImage( "ScallopDisplayWindow", nimg );
  cvWaitKey( 15 );
  cvReleaseImage(&nimg);
}

void displayInterestPointImage( IplImage* img, vector<Candidate*>& cds ) {
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

void displayResultsImage( IplImage* img, vector<Candidate*>& cds ) {
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

void displayResultsImage( IplImage* img, vector<Detection*>& cds, string Filename )
{
  IplImage* local = cvCloneImage( img );
  for( unsigned int i=0; i<cds.size(); i++ ) {  
    CvScalar color;
    if( cds[i]->IsBrownScallop )
      color = cvScalar(0,1,0); 
    else if( cds[i]->IsWhiteScallop )
      color = cvScalar(1,1,1); 
    else if( cds[i]->IsBuriedScallop )
      color = cvScalar(0.8,0.5,0.2); 
    else if( cds[i]->IsDollar )
      color = cvScalar(0,0,1);
    else 
      color = cvScalar(0.1,0.5,0.6);

    cvEllipse(local, cvPoint( (int)cds[i]->c, (int)cds[i]->r ), 
      cvSize( cds[i]->major, cds[i]->minor ), 
      (cds[i]->angle*180/PI)-90, 0, 360, color, 2 );
  }
  displayImage( local, Filename );
  cvReleaseImage( &local );
}
