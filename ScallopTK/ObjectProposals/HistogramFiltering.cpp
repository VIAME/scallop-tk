
#include "HistogramFiltering.h"

//------------------------------------------------------------------------------
//                           Filter Class Definition
//------------------------------------------------------------------------------

//Class constructor, loads the desired filter from file
bool hfFilter::loadFromFile( const string& filter_fn, bool allocSecondary ) {

  //Default Init
  filterLoaded = false;
  filter3d = NULL;
  secondary3d = NULL;

  //Read header data and checksums
  string fn_w_dir = filter_fn;
  ifstream filtin (fn_w_dir.c_str(), ios::in | ios::binary);
  if( !filtin.is_open() ) {
    cerr << "ERROR: Color filter file does not exist in this directory!" << endl;
    return false;
  }

  //Read header information
  char* buffer = new char[HEADER_BYTES];
  filtin.read( buffer, HEADER_BYTES );
  float* float_rdr = (float*) buffer;
  int* int_rdr = (int*) buffer;
  int Checksum1 = int_rdr[0];
  numChan = int_rdr[1];
  histBinsCh1 = int_rdr[2];
  histBinsCh2 = int_rdr[3];
  histBinsCh3 = int_rdr[4];
  startCh1 = float_rdr[5]; 
  startCh2 = float_rdr[6]; 
  startCh3 = float_rdr[7]; 
  endCh1 = float_rdr[8];
  endCh2 = float_rdr[9];
  endCh3 = float_rdr[10];
  int Checksum2 = int_rdr[11];

  //Check checksum values
  if( Checksum1 != CHECKSUM || Checksum2 != CHECKSUM ) {
    cerr << "ERROR: Invalid or Corrupt Filter File" << endl;
    return false;
  }
  if( numChan != 3 ) {
    cerr << "ERROR: Invalid color filter number of channels" << endl;
    return false;
  }

  //Dealloc initial read
  delete[] buffer;

  //Read filter data
  int bytesRemaining = histBinsCh1*histBinsCh2*histBinsCh3*4 + 4;
  buffer = new char[bytesRemaining];
  filtin.read( buffer, bytesRemaining );
  float_rdr = (float*) buffer;
  int_rdr = (int*) buffer;

  //Scaling factors to help with formating
  size = histBinsCh1*histBinsCh2*histBinsCh3;
  ch1_scale = histBinsCh3*histBinsCh2;
  ch2_scale = histBinsCh3;

  //Format filter data
  filter3d = new float[size];
  if( allocSecondary )
    secondary3d = new float[size];
  
  //Copy into filter3d buffer
  for( int i=0; i<size; i++ ) {
    filter3d[i] = float_rdr[i];
  }

  //Check final checksum
  if( int_rdr[size] != CHECKSUM ) {
    cerr << "ERROR: Invalid or Corrupt Filter File!" << endl;
    return false;
  }

  //Calc bpr for fastfilter
  bprCh1 = histBinsCh1 / (endCh1 - startCh1);
  bprCh2 = histBinsCh2 / (endCh2 - startCh2);
  bprCh3 = histBinsCh3 / (endCh3 - startCh3);

  //Dealloc and indicate success
  delete[] buffer;
  filterLoaded = true;
  return true;
}

// Class deconstructor (Deallocates filters)
hfFilter::~hfFilter() {
  if( filter3d != NULL ) {
    delete[] filter3d;
  }
  if( secondary3d != NULL ) {
    delete[] secondary3d;
  }
}
  
// Classifies a 3-d point based off of the loaded histogram
float hfFilter::classifyPoint3d( float* pt ) {
  int b1 = (int)(histBinsCh1 * (pt[0] - startCh1) / (endCh1 - startCh1));
  int b2 = (int)(histBinsCh2 * (pt[1] - startCh2) / (endCh2 - startCh2));
  int b3 = (int)(histBinsCh3 * (pt[2] - startCh3) / (endCh3 - startCh3));

  if( b1 < 0 || b1 >= histBinsCh1 || 
    b2 < 0 || b2 >= histBinsCh2 || 
    b3 < 0 || b3 >= histBinsCh3 ) {
    return 0.0f;
  }
  
  return *(filter3d+b1*ch1_scale+b2*ch2_scale+b3);
}

//FAST FILTER - UNSTABLE IF USED INCORRECTLY
IplImage *hfFilter::classify3dImage( IplImage *img ) {
  assert( img->nChannels == 3 );
  assert( img->depth == IPL_DEPTH_32F );
  IplImage *output = cvCreateImage( cvGetSize( img ), IPL_DEPTH_32F, 1 );
  int istep = img->widthStep;
  int ostep = output->widthStep;
  int height = output->height;
  int width = output->width;
  float *ivalue = (float*)(img->imageData);
  float *ovalue = (float*)(output->imageData);
  char *test = output->imageData;
  for( int i=0; i<height*width; i++ ) {
    int b1 = (int)((ivalue[0] - startCh1) * bprCh1);
    int b2 = (int)((ivalue[1] - startCh2) * bprCh2);
    int b3 = (int)((ivalue[2] - startCh3) * bprCh3);

    if( b1 < 0 || b1 >= histBinsCh1 || 
      b2 < 0 || b2 >= histBinsCh2 || 
      b3 < 0 || b3 >= histBinsCh3 ) {
      *ovalue = 0.0f;
    } else {    
      *ovalue = *(filter3d+ch1_scale*b1+ch2_scale*b2+b3);
    }

    ivalue = ivalue + 3;
    ovalue = ovalue + 1;
  }
  return output;
}

void hfFilter::flushSecondary() {
  float *ptr = secondary3d;
  for( int i = 0; i < size; i++ ) {
    *(ptr+i) = 0.0f;
  }
}

void hfFilter::mergeSecondary( float ratio, float secondaryDown ) {

  for( int i = 0; i < size; i++ ) {
    filter3d[i] = ratio * secondary3d[i] * secondaryDown
          + (1-ratio) * filter3d[i];
  }
}

int hfFilter::insertInSecondary( float *val ) {
  int b1 = (int)((val[0] - startCh1) * bprCh1);
  int b2 = (int)((val[1] - startCh2) * bprCh2);
  int b3 = (int)((val[2] - startCh3) * bprCh3);

  if( b1 < 0 || b1 >= histBinsCh1 || 
    b2 < 0 || b2 >= histBinsCh2 || 
    b3 < 0 || b3 >= histBinsCh3 ) {
    return 0;
  } 
  float *ptr = secondary3d + b1*ch1_scale+b2*ch2_scale+b3;
  *ptr = *ptr + 1.0f;
  return 1;
}

//-------------------------------SECOND------------------------------------

void salFilter::allocMap( int binsPerDim ) {

  // Initialize Histogram
  size = binsPerDim*binsPerDim*binsPerDim;
  ch1_scale = binsPerDim*binsPerDim;
  ch2_scale = binsPerDim;
  histBinsCh1 = binsPerDim;
  histBinsCh2 = binsPerDim;
  histBinsCh3 = binsPerDim;
  filter3d = new float[size];
  flushFilter();
}


void salFilter::flushFilter() {
  float *ptr = filter3d;
  for( int i = 0; i < size; i++ ) {
    *(ptr+i) = 0.0f;
  }
}

//incomplete
void salFilter::smoothHist() {

  // Create new filter buffer
  float *newb = new float[size];
  for( int i = 0; i < size; i++ ) {
    newb[i] = 0.0f;
  }

  // 3d Convolution
  const float INNER_FACTOR = 1.0f;
  const float OUTER_FACTOR = 1.0f;
  for( int i = 1; i < histBinsCh1 - 1; i++ ) {
    for( int j = 1; j < histBinsCh2 - 1; j++ ) {
      for( int k = 1; k < histBinsCh3 - 1; k++ ) {

        float val = *(filter3d+ch1_scale*i+ch2_scale*j+k);

        float *center = newb+ch1_scale*i+ch2_scale*j+k;
        *center = *center + val * INNER_FACTOR;

        float *s1 = center-1;
        float *s2 = center+1;
        float *s3 = center-ch1_scale;
        float *s4 = center+ch1_scale;
        float *s5 = center-ch2_scale;
        float *s6 = center+ch2_scale;

        float adj_val = val * OUTER_FACTOR;

        *s1 = *s1 + adj_val;
        *s2 = *s2 + adj_val;
        *s3 = *s3 + adj_val;
        *s4 = *s4 + adj_val;
        *s5 = *s5 + adj_val;
        *s6 = *s6 + adj_val;
      }
    }
  }

  // Deallocate old filter buffer
  delete[] filter3d;
  filter3d = newb;
}

salFilter::~salFilter() {
  if( filter3d != NULL )
    delete[] filter3d;
}

IplImage *salFilter::classify3dImage( IplImage *img ) {
  assert( img->nChannels == 3 );
  assert( img->depth == IPL_DEPTH_32F );
  IplImage *output = cvCreateImage( cvGetSize( img ), IPL_DEPTH_32F, 1 );
  int istep = img->widthStep;
  int ostep = output->widthStep;
  int height = output->height;
  int width = output->width;
  float *ivalue = (float*)(img->imageData);
  float *ovalue = (float*)(output->imageData);
  char *test = output->imageData;

  for( int i=0; i<height*width; i++ ) {
    int b1 = (int)((ivalue[0]) * histBinsCh1);
    int b2 = (int)((ivalue[1]) * histBinsCh2);
    int b3 = (int)((ivalue[2]) * histBinsCh3);

    if( b1 < 0 || b1 >= histBinsCh1 || 
      b2 < 0 || b2 >= histBinsCh2 || 
      b3 < 0 || b3 >= histBinsCh3 ) {
      *ovalue = 0.0f;
    } else {    
      *ovalue = *(filter3d+ch1_scale*b1+ch2_scale*b2+b3);
    }
    
    ivalue = ivalue + 3;
    ovalue = ovalue + 1;
  }
  
  return output;
}

void salFilter::buildMap( IplImage *img ) {

  int pixels_to_skip = 1;

  float *img_ptr = (float*) img->imageData;
  float *img_end = ((float*) img->imageData) + img->width * img->height * 3;

  while( img_ptr < img_end ) {

    int b1 = (int)((img_ptr[0]) * histBinsCh1);
    int b2 = (int)((img_ptr[1]) * histBinsCh2);
    int b3 = (int)((img_ptr[2]) * histBinsCh3);

    if( b1 >= 0 && b1 < histBinsCh1 && 
      b2 >= 0 && b2 < histBinsCh2 && 
      b3 >= 0 && b3 < histBinsCh3 ) 
    {

      float *ptr = filter3d+ch1_scale*b1+ch2_scale*b2+b3;
      *ptr = *ptr - 1.0;
    }

    img_ptr = img_ptr + pixels_to_skip*3;
  }

  isValid = true;
}


//-------------------------------SECOND------------------------------------

// Class to construct our filter and perform classifications with it
bool ColorClassifier::loadFilters( const string& dir, const string& seedname ) {
  if( !WhiteScallop.loadFromFile( dir + "scallop_brown" + seedname, true ) ) {
    cerr << "ERROR: Cannot load white scallop filter\n";
    return false;
  }
  if( !BrownScallop.loadFromFile( dir + "scallop_white" + seedname, true ) ) {
    cerr << "ERROR: Cannot load white scallop filter\n";
    return false;
  }
  /*if( !Clam.loadFromFile( "clams_all" + seedname, true ) ) {
    cerr << "ERROR: Cannot load white scallop filter\n";
    return false;
  }*/
  if( !Environment.loadFromFile( dir + "environment" + seedname, true ) ) {
    cerr << "ERROR: Cannot load white scallop filter\n";
    return false;
  }
  if( !SandDollars.loadFromFile( dir + "dollars_all" + seedname, true ) ) {
    cerr << "ERROR: Cannot load white scallop filter\n";
    return false;
  }

  // Configure filter to load saliency map into
  SaliencyModel.allocMap( 80 );

  filtersLoaded = true;
  return true;
}

hfResults *ColorClassifier::classifiyImage( IplImage *img ) {
  hfResults * ptr = new hfResults;
  ptr->BrownScallopClass = BrownScallop.classify3dImage( img );
  ptr->WhiteScallopClass = WhiteScallop.classify3dImage( img );
  ptr->SandDollarsClass = SandDollars.classify3dImage( img );
  ptr->EnvironmentalClass = Environment.classify3dImage( img );
  ptr->NetScallops = cvCreateImage( cvGetSize(img), IPL_DEPTH_32F, 1 );
  cvMax( ptr->BrownScallopClass, ptr->WhiteScallopClass, ptr->NetScallops );
  cvSub( ptr->NetScallops, ptr->EnvironmentalClass, ptr->NetScallops );
  return ptr;
}

void ColorClassifier::Update( IplImage *img, IplImage *mask, int Detections[] ) {

  // Reset all secondary filters to 0  
  Environment.flushSecondary();

  if( Detections[SCALLOP_WHITE] > 0 )
    WhiteScallop.flushSecondary();

  //if( Detections[DOLLAR] > 0 )
  //  SandDollars.flushSecondary();

  if( Detections[SCALLOP_BROWN] || Detections[SCALLOP_BURIED] > 0 )
    BrownScallop.flushSecondary();

  // Build secondary filters - TODO REWRITE OH GOD MY EYES
  int brown_scallop_count = 0;
  int envi_count = 0;
  int white_scallop_count = 0;
  int sand_dollar_count = 0;
  unsigned char *mask_ptr = (unsigned char*) mask->imageData;
  unsigned char *ctag = mask_ptr;
  float *img_ptr = (float*) img->imageData;
  float *img_end = ((float*) img->imageData) + img->width * img->height * 3;
  int position = 0;
  int xtra = mask->widthStep - mask->width;
  int width = mask->width;
  int envi_skip = DEFAULT_ENVI_SKIP;
  int obj_skip = DEFAULT_OBJ_SKIP;
  int nsamples = img->width * img->height;
  float sf = MAX_ESTIM_TO_SCAN / ( OBJ_ENVI_RATIO*nsamples/obj_skip + (1-OBJ_ENVI_RATIO)*nsamples/envi_skip); 
  if( sf < RESIZE_FACTOR_REQUIRED ) {
    envi_skip = envi_skip * 1.0f/sf;
    obj_skip = obj_skip * 1.0f/sf;
  }
  while( img_ptr < img_end ) {

    // Insert val into correct histogram
    ctag = mask_ptr + position + (position/width)*xtra;
    tag cls = *ctag;
    if( cls <= ENVIRONMENT ) {
      envi_count += Environment.insertInSecondary( img_ptr );
      img_ptr = img_ptr + envi_skip*3;
      position = position + envi_skip;
    } else if ( cls == SCALLOP_BROWN || cls == SCALLOP_BURIED ) {
      brown_scallop_count += BrownScallop.insertInSecondary( img_ptr );
      img_ptr = img_ptr + obj_skip*3;
      position = position + obj_skip;
    } else if ( cls == SCALLOP_WHITE ) {
      white_scallop_count += WhiteScallop.insertInSecondary( img_ptr );
      img_ptr = img_ptr + obj_skip*3;
      position = position + obj_skip;
    } else if ( cls == DOLLAR ) {
      //sand_dollar_count += SandDollars.insertInSecondary( img_ptr );
      img_ptr = img_ptr + obj_skip*3;
      position = position + obj_skip;
    } else {
      envi_count += Environment.insertInSecondary( img_ptr );
      img_ptr = img_ptr + envi_skip*3;
      position = position + envi_skip;
    }
  }

  // Merge secondary filters into primary if change
  Environment.mergeSecondary( DEFAULT_MERGE_RATIO, 1.0f/envi_count );

  float WHITE_MR = Detections[SCALLOP_WHITE]*0.004f;
  if( Detections[SCALLOP_WHITE] )
    WhiteScallop.mergeSecondary( WHITE_MR, 1.0f/envi_count );

  //float DOLLAR_MR = Detections[DOLLAR]*0.004f;
  //if( Detections[DOLLAR] )
  //  SandDollars.mergeSecondary( DOLLAR_MR, 1.0f/envi_count );

  float BROWN_MR = (Detections[SCALLOP_BROWN]+Detections[SCALLOP_BURIED])*0.004f;
  if( Detections[SCALLOP_BROWN] )
    BrownScallop.mergeSecondary( BROWN_MR, 1.0f/envi_count );
}

// Deallocate filter results
void hfDeallocResults( hfResults* res ) {
  if( res == NULL )
    return;
  cvReleaseImage( &(res->BrownScallopClass) );
  cvReleaseImage( &(res->WhiteScallopClass) );
  cvReleaseImage( &(res->SandDollarsClass) );
  cvReleaseImage( &(res->EnvironmentalClass) );
  cvReleaseImage( &(res->NetScallops) );
  cvReleaseImage( &(res->EnvironmentMap) );
  cvReleaseImage( &(res->SaliencyMap) );
  delete res;
}

void quickPercentiles( IplImage* img, float p1, float p2, float &op1, float& op2 ) {
  float *ptr = (float*) img->imageData;
  int sze = img->width * img->height;
  float *endptr = ptr + sze;
  int skippage = 20;
  if( sze / 1300 > 20 )
    skippage = sze / 1300;
  vector<float> val_list(sze/skippage+1,0.0f);
  int counter = 0;
  while( ptr < endptr ) {
    val_list[counter] = *ptr;
    counter++;
    ptr = ptr + skippage;
  }
  sort( val_list.begin(), val_list.end() );
  op1 = val_list[p1*sze/skippage];
  op2 = val_list[p2*sze/skippage];
}

hfResults *ColorClassifier::performColorClassification( IplImage* img, float minRad, float maxRad ) {

  // Declare pointer to output
  hfResults *results;

  // Perform Class-by-Class Classification
  float resizeFactor = MPFMR_COLOR_CLASS / minRad;
  if( resizeFactor < RESIZE_FACTOR_REQUIRED ) {
    IplImage *temp = cvCreateImage( cvSize((int)(resizeFactor*img->width),(int)(resizeFactor*img->height)),
                                        img->depth, img->nChannels );
    cvResize(img, temp, CV_INTER_LINEAR);
    results = classifiyImage( temp );
    cvReleaseImage(&temp);
    results->minRad = minRad * resizeFactor;
    results->maxRad = maxRad * resizeFactor;
    results->scale = resizeFactor;
  } else {
    results = classifiyImage( img );
    results->minRad = minRad;
    results->maxRad = maxRad;
    results->scale = 1.0f;
  }

  // Create environment map
  float p1, p2;
  quickPercentiles( results->EnvironmentalClass, 0.04, 0.40, p1, p2 ); 
  results->EnvironmentMap = cvCreateImage( cvGetSize( results->EnvironmentalClass ), IPL_DEPTH_32F, 1 );
  cvScale( results->EnvironmentalClass, results->EnvironmentMap, -1.0f/(p2-p1), p2/(p2-p1) );
  cvThreshold( results->EnvironmentMap, results->EnvironmentMap, 0.0, 0.0, CV_THRESH_TOZERO );
  cvSmooth( results->EnvironmentMap, results->EnvironmentMap, CV_BLUR, 5, 5 );

  // Create saliency map
  SaliencyModel.flushFilter();
  SaliencyModel.buildMap( img );
  SaliencyModel.smoothHist();
  results->SaliencyMap = SaliencyModel.classify3dImage( img );
  cvSmooth( results->SaliencyMap, results->SaliencyMap, 2, 3, 3 );

  // Return results
  return results;
}

void detectColoredBlobs( hfResults* color, CandidatePtrVector& cds ) {
  
  // Add border unto classification results and resize if needed
  float resize_factor = MPFMR_COLOR_DOG/color->minRad;
  IplImage* input;
  float minRad = color->minRad;
  float maxRad = color->maxRad;
  if( resize_factor < RESIZE_FACTOR_REQUIRED ) {
    int nwidth = (int)(resize_factor * color->NetScallops->width);
    int nheight = (int)(resize_factor * color->NetScallops->height);
    input = cvCreateImage( cvSize(nwidth,nheight), IPL_DEPTH_32F, 1 );
    cvResize(color->NetScallops, input, CV_INTER_LINEAR);
    minRad = color->minRad * resize_factor;
    maxRad = color->maxRad * resize_factor;
  } else {
    input = cvCloneImage(color->NetScallops);
    resize_factor = 1.0f;
  }

  // Smooth input
  cvSmooth( input, input, 2, 3, 3 );

  // Find DoG Candidates in image
  findDoGCandidates( input, cds, minRad, maxRad, DOG_MAX );

  // Adjust cds for scaling factor
  float scaleFactor = 1.0 / (color->scale * resize_factor);
  for( unsigned int i=0; i<cds.size(); i++ ) {
    scaleCD( cds[i], scaleFactor );
  }

  // Dealloc
  cvReleaseImage( &input );
}

void detectSalientBlobs( hfResults* color, CandidatePtrVector& cds ) {
  
  // Add border unto classification results and resize if needed
  float resize_factor = MPFMR_COLOR_DOG/color->minRad;
  IplImage* input;
  float minRad = color->minRad;
  float maxRad = color->maxRad;
  if( resize_factor < RESIZE_FACTOR_REQUIRED ) {
    int nwidth = (int)(resize_factor * color->SaliencyMap->width);
    int nheight = (int)(resize_factor * color->SaliencyMap->height);
    input = cvCreateImage( cvSize(nwidth,nheight), IPL_DEPTH_32F, 1 );
    cvResize(color->SaliencyMap, input, CV_INTER_LINEAR);
    minRad = color->minRad * resize_factor;
    maxRad = color->maxRad * resize_factor;
  } else {
    input = cvCloneImage(color->SaliencyMap);
    resize_factor = 1.0f;
  }

  // Smooth input image
  cvSmooth( input, input, 2, 3, 3 );

  // Find DoG Candidates in image
  findDoGCandidates( input, cds, minRad, maxRad, DOG_ALL );

  // Adjust cds for scaling factor
  float scaleFactor = 1 / (color->scale * resize_factor);
  for( unsigned int i=0; i<cds.size(); i++ ) {
    scaleCD( cds[i], scaleFactor );
  }

  // Dealloc
  cvReleaseImage( &input );
}
