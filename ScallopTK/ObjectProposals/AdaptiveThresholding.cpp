
#include "AdaptiveThresholding.h"

//------------------------------------------------------------------------------
//                                 Definitions
//------------------------------------------------------------------------------

namespace ScallopTK
{

// DEPRECATED
// Calculate percentile values of filter results
/*void calcFilterStats( IplImage *img, hfFilterStats& stats ) {

  // Collect input image properties
  int nChan = img->nChannels;
  int depth = img->depth;
  int width = img->width;
  int height = img->height;
  int step = img->widthStep;

  //Create map for sorting values
  vector<float> value_list(width*height,0); 

  //Sort all values in filter result
  int counter = 0;
  for( int i=0; i<height; i++ ) {
    for( int j=0; j<width; j++ ) {
      value_list[counter] = (((float*)(img->imageData + i*step))[j]);
      counter++;
    }
  }
  sort(value_list.begin(), value_list.end());

  //Enter into filter stat array
  int entries = value_list.size();
  //Max/min values
  stats[0] = value_list[0];
  stats[HF_STAT_SIZE-1] = value_list[entries-1];
  //All others
  for( int i = 1; i < HF_STAT_SIZE-1; i++ ) {
    int bin = (int)((float)entries*hfStatPercentiles[i]);    
    stats[i] = value_list[bin];
  }
}*/

//DEPRECATED
// Calculate percentile values of filter results
/*void calcFilterStats( IplImage *img, float& p_low, float& p_high, int downsample ) {

  // Collect input image properties
  int nChan = img->nChannels;
  int depth = img->depth;
  int width = img->width;
  int height = img->height;
  int step = img->widthStep;

  //Create map for sorting values
  vector<float> value_list(width*height,0); 

  //Sort all values in filter result
  int counter = 0;
  for( int i=0; i<height; i++ ) {
    for( int j=0; j<width; j++ ) {
      value_list[counter] = (((float*)(img->imageData + i*step))[j]);
      counter++;
    }
  }
  sort(value_list.begin(), value_list.end());

  //Enter into filter stat array
  int entries = value_list.size();
  //Max/min values
  int cut = 20;
  p_low = value_list[value_list.size()/cut];
  p_high = value_list[value_list.size()-value_list.size()/cut];
}*/

void calcFilterStats( IplImage *img, atStats& stats ) {

  // Calculate percentiles
  float *ptr = (float*) img->imageData;
  int sze = img->width * img->height;
  float *endptr = ptr + sze;
  int skippage = sze / AT_SAMPLES;
  if( skippage < 1 )
    skippage = 1;
  int entries = sze / skippage;
  vector<float> val_list(entries,0.0f);
  int counter = 0;
  while( counter < entries ) {
    val_list[counter] = *ptr;
    counter++;
    ptr = ptr + skippage;
  }
  sort( val_list.begin(), val_list.end() );
  
  // Determine where 0.0 (start threshold) lies
  // Binary search would have been more efficient, but irrelevant 
  // because this takes so short a time...
  int pivot = val_list.size();
  for( int i=0; i<val_list.size(); i++ ) {
    if( val_list[i] > 0.0 ) {
      pivot = i;
      break;
    }
  }

  // Calculate lower %tiles
  if( pivot > 0 ) {
    
    int ind_list[AT_LOWER_SIZE];
    int range = pivot;
    for( int i=0; i<AT_LOWER_SIZE; i++ ) {
      ind_list[i] = (int)(AT_LOWER_PER[i]*(float)range);
      if( ind_list[i] < 0 )
        ind_list[i] = 0;
      stats.lower_intvls[i] = val_list[ ind_list[i] ];
    }

  } else {

    //These vals will probabilistically never be used
    float val = 0.00;
    float dec = 0.01;
    for( int i=0; i<AT_LOWER_SIZE; i++ )
      stats.lower_intvls[i] = (val-=dec);
  }

  // Calculate higher %tiles
  if( pivot < val_list.size() ) {

    int ind_list[AT_UPPER_SIZE];
    int range = val_list.size() - pivot;
    for( int i=0; i<AT_UPPER_SIZE; i++ ) {
      ind_list[i] = (int)(AT_UPPER_PER[i]*(float)range) + pivot;
      if( ind_list[i] >= val_list.size() )
        ind_list[i] = val_list.size() - 1;
      stats.upper_intvls[i] = val_list[ ind_list[i] ];
    }

  } else {

    //These vals will probabilistically never be used
    float val = 0.00;
    float inc = 0.01;
    for( int i=0; i<AT_UPPER_SIZE; i++ )
      stats.upper_intvls[i] = (val+=inc);
  }

}

int hfBinaryClassify(IplImage *bin, float minRad, float maxRad, CandidatePtrVector& kps ) {

  // Create OpenCV storage block
  CvMemStorage *mem = cvCreateMemStorage(0);
  CvSeq *Contours, *ptr;

  // Identify Contours in binary image
  cvFindContours(bin,mem,&Contours,sizeof(CvContour),CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE,cvPoint(0,0));

  //Process
  int highcount = 0;
  int lowcount = 0;
  for( ptr = Contours; ptr != NULL; ptr = ptr->h_next ) {
    int ret = findStableMatches( ptr, minRad, maxRad, kps, bin );
    if( ret == -1 )
      lowcount++;
    if( ret == 1 )
      highcount++;
  }

  //Create return
  int retval = 0x0;
  if( lowcount > 100000 )
    retval = retval | AT_LOWER;
  if( highcount > 0 )
    retval = retval | AT_RAISE;

  //Deallocate
  cvReleaseMemStorage(&Contours->storage);
  return 0;
}

int hfLowBinarySearch(hfResults* imgs, atStats* stats, CandidatePtrVector& kps, float minRad, float maxRad, int position ) {

  // Recursive end condition
  if( position >= AT_LOWER_SIZE )
    return position;

  // Threshold image
  IplImage *threshed = cvCreateImage( cvGetSize(imgs->NetScallops), IPL_DEPTH_8U, 1 );
  cvThreshold(imgs->NetScallops,threshed, stats->lower_intvls[position],1.0f,CV_THRESH_BINARY);  

  // Classify binary groups
  int option = hfBinaryClassify(threshed, minRad, maxRad, kps );

  // Call recursive search at a different threshold
  if( option & AT_LOWER ) {
    hfLowBinarySearch(imgs, stats, kps, minRad, maxRad, 2*position );
  }
  if( option & AT_RAISE ) {
    hfLowBinarySearch(imgs, stats, kps, minRad, maxRad, 2*position+1 );
  }

  // Deallocate memory
  cvReleaseImage(&threshed);
  return position;
}

int hfHighBinarySearch(hfResults* imgs, atStats* stats, CandidatePtrVector& kps, float minRad, float maxRad, int position ) {

  // Recursive end condition
  if( position >= AT_UPPER_SIZE )
    return position;

  // Threshold image
  IplImage *threshed = cvCreateImage( cvGetSize(imgs->NetScallops), IPL_DEPTH_8U, 1 );
  cvThreshold(imgs->NetScallops,threshed,stats->upper_intvls[position],1.0f,CV_THRESH_BINARY);  

  // Classify binary groups
  int option = hfBinaryClassify(threshed, minRad, maxRad, kps );

  // Call recursive search at a different threshold
  if( option & AT_LOWER ) {
    hfHighBinarySearch(imgs, stats, kps, minRad, maxRad, 2*position );
  }
  if( option & AT_RAISE ) {
    hfHighBinarySearch(imgs, stats, kps, minRad, maxRad, 2*position+1 );
  }

  // Deallocate memory
  cvReleaseImage(&threshed);
  return position;
}

int hfSeedSearch(hfResults* imgs, atStats* stats, CandidatePtrVector& kps, float minRad, float maxRad ) {

  // Threshold image
  IplImage *threshed = cvCreateImage( cvGetSize(imgs->NetScallops), IPL_DEPTH_8U, 1 );
  cvThreshold(imgs->NetScallops,threshed, 0.0f,1.0f,CV_THRESH_BINARY);  

  // Classify binary groups
  int option = hfBinaryClassify(threshed, minRad, maxRad, kps );

  // Call recursive search at a different threshold
  if( option & AT_LOWER ) {
    hfLowBinarySearch(imgs, stats, kps, minRad, maxRad, 1 );
  }
  if( option & AT_RAISE ) {
    hfHighBinarySearch(imgs, stats, kps, minRad, maxRad, 1 );
  }

  // Deallocate memory
  cvReleaseImage(&threshed);
  return option;
}

void performAdaptiveFiltering( hfResults* color, CandidatePtrVector& cds, float minRad, bool doubleIntrp ) {

  // Resize and smooth image as desired
  IplImage *img = color->NetScallops;
  float resize_factor = MPFMR_ADAPTIVE / color->minRad;
  if( resize_factor < RESIZE_FACTOR_REQUIRED ) {
    int new_height = color->NetScallops->height * resize_factor;
    int new_width = color->NetScallops->width * resize_factor;
    img = cvCreateImage(cvSize(new_width,new_height),IPL_DEPTH_32F,1);
    cvResize(color->NetScallops,img);
  } else {
    resize_factor = 1.0f;
  }
    
  // Calculate filter stats (percentiles)
  atStats netStats;
  calcFilterStats( img, netStats );
  hfSeedSearch(color,&netStats,cds,color->minRad*resize_factor,color->maxRad*resize_factor);

  // Adjust kps for scale  
  float scale = 1 / (resize_factor * color->scale );
  if( scale != 1.0 ) {
    for( unsigned int i=0; i<cds.size(); i++ ) {
      cds[i]->r *= scale;
      cds[i]->c *= scale;
      cds[i]->major *= scale;
      cds[i]->minor *= scale;
    }
  }

  // Deallocations
  if( resize_factor < RESIZE_FACTOR_REQUIRED ) 
    cvReleaseImage( &img );

  // If double interpret not enabled exit
  if( !doubleIntrp )
    return;

  // Add kps based on shape. This is a simple method to respond to the
  // issue that two close objects may be registered as the same kp.
  int ini_size = cds.size();
  for( unsigned int i=0; i<ini_size; i++ ) {
    double maxMinRatio = cds[i]->major / cds[i]->minor;
    //cout << maxMinRatio << " " << cds[i]->major << " " << cds[i]->minor << " " << cds[i]->angle << endl;
    if( maxMinRatio > 1.45 && cds[i]->minor > minRad*1.5 ) {
      //int ip_to_add = (int)ceil( maxMinRatio - 0.4 );
      int ip_to_add = 2;
      float angle_cos = cos( (90+cds[i]->angle)*PI/180 );
      float angle_sin = sin( (90+cds[i]->angle)*PI/180 );
      double intv_r = angle_sin*(cds[i]->major-cds[i]->minor);
      double intv_c = angle_cos*(cds[i]->major-cds[i]->minor);
      double start_r = cds[i]->r + intv_r;
      double start_c = cds[i]->c + intv_c;
      intv_r = -2*intv_r/(ip_to_add-1);
      intv_c = -2*intv_c/(ip_to_add-1);
      for( int j=0; j < ip_to_add; j++ ) {
        Candidate *kp = new Candidate;
        kp->angle = 0;
        kp->r = start_r;
        kp->c = start_c;
        kp->minor = cds[i]->minor;
        kp->major = cds[i]->minor;
        kp->magnitude = 0;
        kp->method = ADAPTIVE;
        cds.push_back( kp );
        start_r += intv_r;
        start_c += intv_c;
      }
    }
  }  
}

}
