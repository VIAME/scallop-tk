//------------------------------------------------------------------------------
// Title: helper.cpp
// Author: Matthew Dawkins
// Description: This file contains common helper functions
//------------------------------------------------------------------------------

#include "HelperFunctions.h"

// Display an image and wait for user input
void showImage( IplImage* img ) {
  cvNamedWindow( "image", CV_WINDOW_AUTOSIZE );
  cvShowImage( "image", img );
  cvWaitKey(0);
  cvDestroyWindow( "image" );
}

void showImageNW( IplImage* img ) {
  cvShowImage( "output", img );
  cvWaitKey(5);
}

//Copys an allocated kp
Candidate *copyCandidate( Candidate* kp ) {

  Candidate *temp = new Candidate;
  temp->r = kp->r;
  temp->c = kp->c;
  temp->angle = kp->angle;
  temp->major = temp->major;
  temp->minor = temp->minor;
  return temp;
}

//Checks the image extension to determine image type
int getImageType( const string& filename ) { 
  string ext = filename.substr(filename.find_last_of(".") + 1);
  if( ext == "jpg" || ext == "JPG" )
    return JPEG;
  else if( ext == "tiff" || ext == "TIFF" )
    return RAW_TIF;
  else if( ext == "tif" || ext == "TIF" )
    return RAW_TIFF;
  else if( ext == "bmp" || ext == "BMP" )
    return BMP;
  else if( ext == "png" || ext == "PNG" )
    return PNG;
  return UNKNOWN;
}

// Remove imagees of unknown type from the list
void cullNonImages( vector<string>& fn_list ) {

  for( int i=fn_list.size()-1; i>=0; i-- ) {
    if( getImageType( fn_list[i] ) == UNKNOWN ) {
      fn_list.erase( fn_list.begin()+i );
    }
  }
}

//Display Candidates within some rdange
void showCandidates( IplImage *img, CandidatePtrVector& kps, float min, float max ) {  
  IplImage* local = cvCloneImage( img );
  for( unsigned int i=0; i<kps.size(); i++ ) {  
    if( kps[i]->major < min || kps[i]->major > max ) continue;

    cvEllipse(local, cvPoint( (int)kps[i]->c, (int)kps[i]->r ), 
      cvSize( kps[i]->major, kps[i]->minor ), 
      (kps[i]->angle*180/PI)-90, 0, 360, cvScalar(0,1,0), 1 );
  }
  showImage( local );
  cvReleaseImage( &local );
}

void showCandidatesNW( IplImage *img, CandidatePtrVector& kps, float min, float max ) {  
  IplImage* local = cvCloneImage( img );
  for( unsigned int i=0; i<kps.size(); i++ ) {
    if( kps[i] == NULL ) continue;
    if( kps[i]->major < min || kps[i]->major > max ) continue;

    cvEllipse(local, cvPoint( (int)kps[i]->c, (int)kps[i]->r ), 
      cvSize( kps[i]->major, kps[i]->minor ), 
      kps[i]->angle, 0, 360, cvScalar(0,1,0), 1 );
  }
  showImageNW( local );
  cvReleaseImage( &local );
}

void saveMatrix( CvMat* mat, string filename ) {
  ofstream out( filename.c_str() );
  for( int i=0; i<mat->height; i++ ) {
    for( int j=0; j<mat->width; j++ ) {
      out << cvmGet( mat, i, j ) << " ";
    }
    out << "\n";
  }
  out.close();
}

void showImageRange( IplImage* img ) {

  IplImage *newimg = cvCloneImage( img );
  int fsize = img->width*img->height*img->nChannels;
  float *ptr = (float*) img->imageData;
  vector<float> value_list(fsize,0); 
  for( int i=0; i<fsize; i++ ) {
    value_list[i] = *ptr;
    ptr = ptr + 1;
  }
  /*for( int i=0; i<img->height; i++ ) {
    for( int j=0; j<img->width; j++ ) {
      value_list[counter] = (((float*)(img->imageData + i*img->widthStep))[j]);
      counter++;
    }
  }*/
  sort(value_list.begin(), value_list.end());
  int p5 = 0.025f*img->width*img->height;
  int p95 = 0.995f*img->width*img->height;
  cout << "Display Range: " << value_list[p5] << " " << value_list[p95] << endl;
  cvScale(newimg, newimg, 1/(value_list[p95]-value_list[p5]), -value_list[p5]/(value_list[p95]-value_list[p5]));
  cvNamedWindow( "image", CV_WINDOW_AUTOSIZE );
  cvShowImage( "image", newimg );
  cvWaitKey(0);
  cvDestroyWindow( "image" );
  cvReleaseImage(&newimg);
}

string intToString(const int &i) {
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();
    return s;
}

void calcMinMax( IplImage *img ) {
  float min[3] = { INF, INF, INF };
  float max[3] = { -INF, -INF, -INF };
  int fsize = img->height*img->width;
  for( int r=0; r<img->height; r++ ) {
    for( int c=0; c<img->width; c++ ) {
      CvScalar s = cvGet2D(img, r, c);
      for( int i=0; i<img->nChannels; i++ ) {
        if( s.val[i] > max[i] )
          max[i] = s.val[i];
        if( s.val[i] < min[i] )
          min[i] = s.val[i];
      }
    }
  }
  cout << endl << endl;
  for( int i=0; i<img->nChannels; i++ ) {
    cout << "Ch" << i << ": ";
    cout << "Min: " << min[i] << " ";
    cout << "Max: " << max[i] << "\n";
  }
}

int stringToInt(const string& s) {
    return atoi(s.c_str());
}

// Show min and max scallop size superimposed on image
void showScallopMinMax( IplImage* img, float minRad, float maxRad ) {

  //Clone image so we can draw unto it
  IplImage* toOutput = cvCloneImage( img );

  //Position circles at center
  int posW = img->width / 2;
  int posH = img->height / 2;

  //Draw circles
  cvCircle( toOutput, cvPoint( posW, posH ), (int)minRad, cvScalar( 0, (int)pow(2.0f, img->depth) - 1, 0), 2 );
  cvCircle( toOutput, cvPoint( posW, posH ), (int)maxRad, cvScalar( 0, (int)pow(2.0f, img->depth) - 1, 0), 2 );

  //Show image
  showImage( toOutput );

  //Deallocate memory
  cvReleaseImage( &toOutput );
}

void showImages( IplImage* img1, IplImage *img2 ) {
  cvNamedWindow( "image1", CV_WINDOW_AUTOSIZE );
  cvShowImage( "image1", img1 );
  cvNamedWindow( "image2", CV_WINDOW_AUTOSIZE );
  cvShowImage( "image2", img2 );
  cvWaitKey(0);
  cvDestroyWindow( "image1" );
  cvDestroyWindow( "image2" );
}

void showIP( IplImage* img, IplImage *img2, Candidate *ip ) {

  //Copy Image
  IplImage *copy = cvCloneImage( img );

  //Draw IP
  cvEllipse(copy, cvPoint( (int)ip->c, (int)ip->r ), 
    cvSize( ip->minor, ip->major ), 
    (ip->angle), 0, 360, cvScalar(0,255,0), 2 );

  //Show Images
  cvNamedWindow( "image1", CV_WINDOW_AUTOSIZE );
  cvShowImage( "image1", copy );
  cvNamedWindow( "image2", CV_WINDOW_AUTOSIZE );
  cvShowImage( "image2", img2 );
  cvWaitKey(0);
  cvDestroyWindow( "image1" );
  cvDestroyWindow( "image2" );

  //Deallocations
  cvReleaseImage( &copy );
}

// EXTERNAL CODE BTRE
void PrintMat(CvMat *A)
{
  int i, j;
  for (i = 0; i < A->rows; i++)
  {
    printf("\n");
    switch (CV_MAT_DEPTH(A->type))
    {
    case CV_32F:
    case CV_64F:
      for (j = 0; j < A->cols; j++)
        printf ("%8.3f ", (float)cvGetReal2D(A, i, j));
      break;
    case CV_8U:
    case CV_16U:
      for(j = 0; j < A->cols; j++)
        printf ("%6d",(int)cvGetReal2D(A, i, j));
      break;
    default:
      break;
    }
  }
  printf("\n");
}

void drawFilledEllipse( IplImage *input, float r, float c, float angle, float major, float minor ) {
  float majsq = major * major;
  float minsq = minor * minor;
  float absq = majsq * minsq;
  float cosf = cos( (angle)*PI/180 );
  float sinf = sin( (angle)*PI/180 );
  for( int i=0; i<input->height; i++ ) {
    for( int j=0; j<input->width; j++ ) {
      int posru = i-r;
      int poscu = j-c;
      int posr = posru * cosf - poscu * sinf;
      int posc = poscu * cosf + posru * sinf;
      float rsq = posr * posr;
      float csq = posc * posc;
      float distsq = rsq + csq;
      float righthand = absq / ( minsq*rsq/distsq + majsq*csq/distsq );
      if( distsq <= righthand ) {
        ((float*)(input->imageData + input->widthStep*i))[j] = 0.0f;
      } else {
        ((float*)(input->imageData + input->widthStep*i))[j] = 1.0f;
      }
    }
  }
}

void updateMask( IplImage *input, float r, float c, float angle, float major, float minor, tag obj ) {
  float majsq = major * major;
  float minsq = minor * minor;
  float absq = majsq * minsq;
  float cosf = cos( (angle)*PI/180 );
  float sinf = sin( (angle)*PI/180 );
  for( int i=0; i<input->height; i++ ) {
    for( int j=0; j<input->width; j++ ) {
      int posru = i-r;
      int poscu = j-c;
      int posr = posru * cosf - poscu * sinf;
      int posc = poscu * cosf + posru * sinf;
      float rsq = posr * posr;
      float csq = posc * posc;
      float distsq = rsq + csq;
      float righthand = absq / ( minsq*rsq/distsq + majsq*csq/distsq );
      if( distsq <= righthand || distsq == 0 ) {
        ((unsigned char*)(input->imageData + input->widthStep*i))[j] = obj;
      }
    }
  }
}

void updateMaskRing( IplImage *input, float r, float c, float angle, float major1, float minor1, float major2, tag obj ) {
  assert( major2 > major1 );
  float ratio = major2 / major1;
  float ratsq = ratio * ratio;
  float majsq = major1 * major1;
  float minsq = minor1 * minor1;
  float absq = majsq * minsq;
  float cosf = cos( (angle)*PI/180 );
  float sinf = sin( (angle)*PI/180 );
  for( int i=0; i<input->height; i++ ) {
    for( int j=0; j<input->width; j++ ) {
      int posru = i-r;
      int poscu = j-c;
      int posr = posru * cosf - poscu * sinf;
      int posc = poscu * cosf + posru * sinf;
      float rsq = posr * posr;
      float csq = posc * posc;
      float distsq = rsq + csq;
      float righthand = absq / ( minsq*rsq/distsq + majsq*csq/distsq );
      if( distsq <= righthand ) {
      } else if( distsq <= righthand*ratsq ) {
        ((unsigned char*)(input->imageData + input->widthStep*i))[j] = obj;
      }
    }
  }
}

void drawEllipseRing( IplImage *input, float r, float c, float angle, float major1, float minor1, float major2 ) {
  assert( major2 > major1 );
  float ratio = major2 / major1;
  float ratsq = ratio * ratio;
  float majsq = major1 * major1;
  float minsq = minor1 * minor1;
  float absq = majsq * minsq;
  float cosf = cos( (angle)*PI/180 );
  float sinf = sin( (angle)*PI/180 );
  for( int i=0; i<input->height; i++ ) {
    for( int j=0; j<input->width; j++ ) {
      int posru = i-r;
      int poscu = j-c;
      int posr = posru * cosf - poscu * sinf;
      int posc = poscu * cosf + posru * sinf;
      float rsq = posr * posr;
      float csq = posc * posc;
      float distsq = rsq + csq;
      float righthand = absq / ( minsq*rsq/distsq + majsq*csq/distsq );
      if( distsq <= righthand ) {
        ((float*)(input->imageData + input->widthStep*i))[j] = 1.0f;
      } else if( distsq <= righthand*ratsq ) {
        ((float*)(input->imageData + input->widthStep*i))[j] = 0.0f;
      } else {
        ((float*)(input->imageData + input->widthStep*i))[j] = 2.0f;
      }
    }
  }
}

inline int determine8quad( int& x, int&y ) {
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
  cerr << "THIS WILL NEVER BE REACHED\n";
}

void drawColorRing( IplImage *input, float r, float c, float angle, float major1, float minor1, float major2, float major3, int (&bins)[COLOR_BINS] ) {
  assert( major2 > major1 );
  assert( major3 > major2 );
  float ratio1 = major2 / major1;
  float ratio2 = major3 / major1;
  float rat1sq = ratio1 * ratio1;
  float rat2sq = ratio2 * ratio2;
  float majsq = major1 * major1;
  float minsq = minor1 * minor1;
  float absq = majsq * minsq;
  float cosf = cos( (angle)*PI/180 );
  float sinf = sin( (angle)*PI/180 );
  for( int i=0; i<input->height; i++ ) {
    for( int j=0; j<input->width; j++ ) {
      int posru = i-r;
      int poscu = j-c;
      float posr = cosf * posru - sinf * poscu;
      float posc = cosf * poscu + sinf * posru;
      float rsq = posr * posr;
      float csq = posc * posc;
      float distsq = rsq + csq;
      float righthand = absq / ( minsq*rsq/distsq + majsq*csq/distsq );
      //cout << posru << " " << poscu << " " << r << " " << c << " " << distsq << " " << righthand << endl;
      if( distsq <= righthand || distsq == 0 ) {
        int position = determine8quad(posru,poscu) + 24;
        ((char*)(input->imageData + input->widthStep*i))[j] = position;
        bins[position]++;
      } else if( distsq <= righthand*rat1sq ) {
        int position = determine8quad(posru,poscu) + 16;
        ((char*)(input->imageData + input->widthStep*i))[j] = position;
        bins[position]++;
      } else if( distsq <= righthand*rat2sq ) {
        int position = determine8quad(posru,poscu) + 8;
        ((char*)(input->imageData + input->widthStep*i))[j] = position;
        bins[position]++;
      } else {
        int position = determine8quad(posru,poscu);
        ((char*)(input->imageData + input->widthStep*i))[j] = position;
        bins[position]++;
      }
    }
  }
}

//Deallocates Candidate vector
void deallocateCandidates( CandidatePtrVector &kps ) {
  for( unsigned int i=0; i < kps.size(); i++ ) 
    if( kps[i] != NULL ) {
      
      if( kps[i]->summaryImage != NULL )
        cvReleaseImage( &kps[i]->summaryImage );
      if( kps[i]->colorQuadrants != NULL )
        cvReleaseImage( &kps[i]->colorQuadrants );
      for( unsigned int j=0; j<NUM_HOG; j++ )
        if( kps[i]->hogResults[j] != NULL )
          cvReleaseMat( &kps[i]->hogResults[j] );

      delete kps[i];
    }
  kps.empty();
}

void showScallopsNW( IplImage *img, CandidatePtrVector& kps ) {
  IplImage* local = cvCloneImage( img );
  for( unsigned int i=0; i<kps.size(); i++ ) {  
    CvScalar color;
    if( kps[i]->classification == 1 )
      color = cvScalar(0,1,0); 
    else if( kps[i]->classification == 2 )
      color = cvScalar(1,1,1); 
    else if( kps[i]->classification == 3 )
      color = cvScalar(1,0,0); 
    else if( kps[i]->classification == 4 )
      color = cvScalar(1,1,0); 
    else 
      continue;
    cvEllipse(local, cvPoint( (int)kps[i]->c, (int)kps[i]->r ), 
      cvSize( kps[i]->major, kps[i]->minor ), 
      (kps[i]->angle*180/PI)-90, 0, 360, color, 1 );
  }
  cvShowImage( "output", local );
  cvWaitKey(5);
  cvReleaseImage( &local );
}

void showScallops( IplImage *img, CandidatePtrVector& kps ) {
  IplImage* local = cvCloneImage( img );
  for( unsigned int i=0; i<kps.size(); i++ ) {  
    CvScalar color;
    if( kps[i]->classification == 1 )
      color = cvScalar(0,1,0); 
    else if( kps[i]->classification == 2 )
      color = cvScalar(1,1,1); 
    else if( kps[i]->classification == 3 )
      color = cvScalar(1,0,0); 
    else if( kps[i]->classification == 4 )
      color = cvScalar(1,1,0); 
    else 
      continue;
    cvEllipse(local, cvPoint( (int)kps[i]->c, (int)kps[i]->r ), 
      cvSize( kps[i]->major, kps[i]->minor ), 
      (kps[i]->angle), 0, 360, color, 1 );
  }
  showImage( local );
  cvReleaseImage( &local );
}

void saveScallops( IplImage *img, DetectionPtrVector& kps, const string& fn ) {
  IplImage* local = cvCloneImage( img );
  for( unsigned int i=0; i<kps.size(); i++ ) {  
    CvScalar color;
    if( kps[i]->isBrownScallop )
      color = cvScalar(0,1,0); 
    else if( kps[i]->isWhiteScallop )
      color = cvScalar(1,1,1); 
    else if( kps[i]->isBuriedScallop )
      color = cvScalar(0.8,0.5,0.2); 
    else if( kps[i]->isSandDollar )
      color = cvScalar(0,0,1);
    else 
      color = cvScalar(0.1,0.5,0.6);
  
    cvEllipse(local, cvPoint( (int)kps[i]->c, (int)kps[i]->r ), 
      cvSize( kps[i]->minor, kps[i]->major ), 
      (kps[i]->angle), 0, 360, color, 2 );
  }
  IplImage *toOut = cvCreateImage( cvGetSize( local ), IPL_DEPTH_8U, 3 );
  cvScale( local, toOut, 255.0 );
  cvSaveImage( fn.c_str(), toOut );
  cvReleaseImage( &toOut );
  cvReleaseImage( &local );
}

void saveImage( IplImage *img, const string &fn ) {
  IplImage *toOut; 
  if( img->depth == IPL_DEPTH_32F ) {
    toOut = cvCreateImage( cvGetSize( img ), IPL_DEPTH_8U, 3 );
    cvScale( img, toOut, 255.0 );
  } else {
    toOut = cvCloneImage( img );
  }  
  cvSaveImage( fn.c_str(), toOut );
  cvReleaseImage( &toOut );
}


void saveCandidates( IplImage *img, CandidatePtrVector& cds, const string& fn ) {
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
      colour = cvScalar( 0.0, 0, 1.0 );
    }
    cvEllipse(local, cvPoint( (int)cds[i]->c, (int)cds[i]->r ), 
      cvSize( cds[i]->minor, cds[i]->major ), 
      cds[i]->angle, 0, 360, colour, 1 );
  }
  IplImage *toOut = cvCreateImage( cvGetSize( local ), IPL_DEPTH_8U, 3 );
  cvScale( local, toOut, 255.0 );
  cvSaveImage( fn.c_str(), toOut );
  cvReleaseImage( &toOut );
  cvReleaseImage( &local );
}

float quickMedian( IplImage* img, int max_to_sample ) {
  float *ptr = (float*) img->imageData;
  int sze = img->width * img->height;
  float *endptr = ptr + sze;
  int skippage = 20;
  if( sze / max_to_sample > 20 )
    skippage = sze / max_to_sample;
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

void showIPNW( IplImage* img, IplImage *img2, Candidate *ip ) {

  //Copy Image
  IplImage *copy = cvCloneImage( img );

  //Draw IP
  cvEllipse(copy, cvPoint( (int)ip->c, (int)ip->r ), 
    cvSize( ip->minor, ip->major ), 
    (ip->angle), 0, 360, cvScalar(0,255,0), 2 );

  //Show Images
  cvShowImage( "output", copy );
  cvShowImage( "output2", img2 );
  cvWaitKey(10);

  //Deallocations
  cvReleaseImage( &copy );
}

void showIPNW( IplImage* img, Candidate *ip ) {

  //Copy Image
  IplImage *copy = cvCloneImage( img );

  //Draw IP
  cvEllipse(copy, cvPoint( (int)ip->c, (int)ip->r ), 
    cvSize( ip->minor, ip->major ), 
    (ip->angle), 0, 360, cvScalar(0,255,0), 2 );
  /*if( ip->hasEdgeFeatures )
    cvEllipse(copy, cvPoint( (int)ip->nc, (int)ip->nr ), 
      cvSize( ip->nminor, ip->nmajor ), 
      (ip->nangle), 0, 360, cvScalar(0,180,30), 1 );*/

  //Show Images
  cvShowImage( "output", copy );
  cvWaitKey(10);

  //Deallocations
  cvReleaseImage( &copy );
}

void initalizeCandidateStats( CandidatePtrVector cds, int imheight, int imwidth ) {
  for( int i=0; i<cds.size(); i++ ) {

    // Initialize Candidate Variables
    cds[i]->isActive = true;
    cds[i]->summaryImage = NULL;
    cds[i]->colorQuadrants = NULL;
    cds[i]->hasEdgeFeatures = false;
    cds[i]->isCorner = false;
    for( unsigned int j=0; j<NUM_HOG; j++ )
      cds[i]->hogResults[j] = NULL;

    // Determine if Candidate is on image border
    const double ICS_MAJOR_INC_FACTOR = 1.33;
    int lr = cds[i]->r - cds[i]->major * ICS_MAJOR_INC_FACTOR;
    int ur = cds[i]->r + cds[i]->major * ICS_MAJOR_INC_FACTOR;
    int lc = cds[i]->c - cds[i]->major * ICS_MAJOR_INC_FACTOR;
    int uc = cds[i]->c + cds[i]->major * ICS_MAJOR_INC_FACTOR;
    if( lr < 0 || lc < 0 || ur < 0 || uc < 0 )
      cds[i]->isCorner = true;
    else if( lr >= imheight || lc >= imwidth || ur >= imheight || uc >= imwidth )
      cds[i]->isCorner = true;

    // Determine which octets around IP are beyond border
    const double ICS_MAJOR_INC_FACTOR_2 = 1.33;
    if( cds[i]->isCorner ) {
      float ptr[8] = { 0.38268f, -0.38268f, 0.38268f, -0.38268f, 0.92388f, -0.92388f, 0.92388f, -0.92388f };
      float ptc[8] = { 0.92388f, 0.92388f, -0.92388f, -0.92388f, 0.38268f, 0.38268f, -0.38268f, -0.38268f };
      for( int j=0; j<8; j++ ) {
        ptr[j] = ptr[j] * cds[i]->major * ICS_MAJOR_INC_FACTOR_2;
        ptc[j] = ptc[j] * cds[i]->major * ICS_MAJOR_INC_FACTOR_2;
        int oct = determine8quads( ptc[j], ptr[j] );
        ptr[j] = ptr[j] + cds[i]->r;
        ptc[j] = ptc[j] + cds[i]->c;
        if( ptr[j] < 0 || ptc[j] < 0 || ptc[j] >= imwidth || ptr[j] >= imheight )
          cds[i]->isSideBorder[oct] = true;
        else 
          cds[i]->isSideBorder[oct] = false;
      }
    } else {
      for( int j=0; j<8; j++ )
        cds[i]->isSideBorder[j] = false;
    }

    // Make sure Candidate is of sufficient size
    const double MIN_PIX_TO_DETECT = 3;
    if( cds[i]->major < MIN_PIX_TO_DETECT )
      cds[i]->isActive = false;
  }
}

void removeBorderCandidates( CandidatePtrVector& cds, IplImage *img )
{
  // Top down greedy search - very slow but who cares its for training only
  for( int j = cds.size() - 1; j >= 0; j-- )
  {
    // Determine if we should kill this Candidate (too close to border)
    bool RemoveCandidate = false;

    if( cds[j]->r < 10 || cds[j]->c < 10 )
    {
      RemoveCandidate = true;
    }
    else if( cds[j]->r > img->height-10 || cds[j]->c > img->width-10 )
    {
      RemoveCandidate = true;
    }

    if( RemoveCandidate )
    {
      delete cds[j];
      cds.erase(cds.begin()+j);
    }
  }
}

vector< string > tokenizeString( std::string s )
{
  vector< string > output;

  string buf;
  stringstream ss( s );

  while( ss >> buf )
    output.push_back( buf );

  return output;
}
