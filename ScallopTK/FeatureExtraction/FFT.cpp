
#include "FFT.h"

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//                                 Definitions
//------------------------------------------------------------------------------

/*
void calculateRFFT( Candidate *cd, IplImage *base ) {
  if( !cd->stats->active )
    return;
  IplImage *input = cd->stats->imgAdj64;
  IplImage *output = cvCreateImage( cvGetSize( input ), input->depth, input->nChannels );
  cvDFT( input, output, CV_DXT_FORWARD | CV_DXT_SCALE );
  cd->stats->rfft = output;
}

void calculateCFFT( Candidate *cd, IplImage *base ) {
  if( !cd->stats->active )
    return;
  IplImage *input = cd->stats->imgAdjLogPolar;
  IplImage *output = cvCreateImage( cvGetSize( input ), input->depth, input->nChannels );
  cvDFT( input, output, CV_DXT_FORWARD | CV_DXT_SCALE );
  cd->stats->cfft = output;
}

// Rearrange the quadrants of Fourier image so that the origin is at
// the image center
// src & dst arrays of equal size & type
void cvShiftDFT(CvArr * src_arr, CvArr * dst_arr )
{
  CvMat * tmp;
  CvMat q1stub, q2stub;
  CvMat q3stub, q4stub;
  CvMat d1stub, d2stub;
  CvMat d3stub, d4stub;
  CvMat * q1, * q2, * q3, * q4;
  CvMat * d1, * d2, * d3, * d4;

  CvSize size = cvGetSize(src_arr);
  CvSize dst_size = cvGetSize(dst_arr);
  int cx, cy;

  if(dst_size.width != size.width ||
    dst_size.height != size.height){
  }

  if(src_arr==dst_arr){
    tmp = cvCreateMat(size.height/2, size.width/2, cvGetElemType
      (src_arr));
  }

  cx = size.width/2;
  cy = size.height/2; // image center

  q1 = cvGetSubRect( src_arr, &q1stub, cvRect(0,0,cx, cy) );
  q2 = cvGetSubRect( src_arr, &q2stub, cvRect(cx,0,cx,cy) );
  q3 = cvGetSubRect( src_arr, &q3stub, cvRect(cx,cy,cx,cy) );
  q4 = cvGetSubRect( src_arr, &q4stub, cvRect(0,cy,cx,cy) );
  d1 = cvGetSubRect( src_arr, &d1stub, cvRect(0,0,cx,cy) );
  d2 = cvGetSubRect( src_arr, &d2stub, cvRect(cx,0,cx,cy) );
  d3 = cvGetSubRect( src_arr, &d3stub, cvRect(cx,cy,cx,cy) );
  d4 = cvGetSubRect( src_arr, &d4stub, cvRect(0,cy,cx,cy) );

  if(src_arr!=dst_arr){
    if( !CV_ARE_TYPES_EQ( q1, d1 )){

    }
    cvCopy(q3, d1, 0);
    cvCopy(q4, d2, 0);
    cvCopy(q1, d3, 0);
    cvCopy(q2, d4, 0);
  } else {
    cvCopy(q3, tmp, 0);
    cvCopy(q1, q3, 0);
    cvCopy(tmp, q1, 0);
    cvCopy(q4, tmp, 0);
    cvCopy(q2, q4, 0);
    cvCopy(tmp, q2, 0);
  }
}*/

