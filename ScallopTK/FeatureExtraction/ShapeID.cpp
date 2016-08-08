
#include "ShapeID.h"

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

inline int determine8quad_sm( int& x, int&y );

//------------------------------------------------------------------------------
//                                 Definitions
//------------------------------------------------------------------------------

/*
void calculateShapeFeatures( Candidate *cd, IplImage *unused ) {

  if( !cd->stats->active )
    return;

  // Extract contour from watershed result
  IplImage *wsResult = cd->stats->wsMask;
  struct cstats* stats = cd->stats;
  vector<CvPoint2D32f> contour_list;
  for( int r=2; r<wsResult->height-2; r++ ) {
    for( int c=2; c<wsResult->width-2; c++ ) {
      if( ((int*)(wsResult->imageData + wsResult->widthStep*r))[c] == -1 ) {
        CvPoint2D32f p;
        p.x = c;
        p.y = r;
        contour_list.push_back( p );
      }
    }
  }

  if( contour_list.size() < 6 ){
    for( int i=0; i<9; i++ ) {
      cd->stats->shape_stats[i] = 0.0;
    }
    cd->stats->shape_stats[9] = cd->major;
    cd->stats->shape_stats[10] = sqrt(1.0 - (cd->major/cd->minor)*(cd->major/cd->minor));
    return;
  }

  // Fit ellipse on extracted contour
  CvBox2D* box = (CvBox2D*)malloc(sizeof(CvBox2D));
  cv::fitEllipse( &contour_list[0], contour_list.size(), box );
  double angle = stats->angle = box->angle;
  double new_cr   = stats->r    = box->center.y + stats->wsOffset.y;
  double new_cc   = stats->c    = box->center.x + stats->wsOffset.x;
  double minor = stats->minor = box->size.height / 2.0;
  double major = stats->major = box->size.width / 2.0;
  double old_cr   = cd->r;
  double old_cc   = cd->c;
  double avg_cr   = (old_cr + new_cr)/2;
  double avg_cc   = (old_cc + new_cc)/2;

  // Calculate MSE in different quadraunts
  double MSE[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  double BinCount[8] = { 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001 };
  float majsq = major * major;
  float minsq = minor * minor;
  float absq = majsq * minsq;
  float cosf = cos( (angle)*PI/180 );
  float sinf = sin( (angle)*PI/180 );
  avg_cr = avg_cr - stats->wsOffset.y;
  avg_cc = avg_cc - stats->wsOffset.x;
  new_cr = new_cr - stats->wsOffset.y;
  new_cc = new_cc - stats->wsOffset.x;
  for( unsigned int i=0; i < contour_list.size(); i++ ) {
    int adj_r = contour_list[i].y - new_cr;
    int adj_c = contour_list[i].x - new_cc;
    int avg_adj_r = contour_list[i].y - avg_cr;
    int avg_adj_c = contour_list[i].x - avg_cc;
    int region = determine8quad_sm( avg_adj_c, avg_adj_r );
    int posr = adj_r * cosf - adj_r * sinf;
    int posc = adj_c * cosf + adj_c * sinf;
    float rsq = posr * posr;
    float csq = posc * posc;
    float distsq = rsq + csq;
    if( distsq == 0 )
      distsq = 0.001;
    float lower = ( minsq*rsq/distsq + majsq*csq/distsq );
    if( lower == 0 )
      lower = 1;
    float righthand = absq / lower;
    float error = distsq + righthand - 2*sqrt(distsq)*sqrt(righthand);
    MSE[region] = MSE[region] + error;
    BinCount[region]++;
  }

  // Normalize MSE bins and calculate net
  double scaleFactor = 1/sqrt(absq/2); //partial perimeter approx
  double netMSE = 0.0f;
  double BinSum = 0;
  for( int i=0; i<8; i++ ) {
    netMSE = netMSE + MSE[i];
    BinSum = BinSum + BinCount[i];
    MSE[i] = MSE[i] / BinCount[i];
    MSE[i] = scaleFactor * MSE[i];
    cd->stats->shape_stats[i] = MSE[i];
  }
  netMSE = scaleFactor * netMSE / BinSum;
  cd->stats->shape_stats[8] = netMSE;
  cd->stats->shape_stats[9] = major;
  cd->stats->shape_stats[10] = sqrt(1.0 - (major/minor)*(major/minor));
  
  // Convert contour to polar cooridantes
  //TODO

  // Run FFT on polar coordinates
  //TODO
   
}*/

void calculateSizeFeatures( Candidate *cd, ImageProperties& ip, float initResize ) {

  if( !cd->is_active )
    return;
  float major_meters, minor_meters, area_msq, perimeter_m;
  float aPS = ip.getAvgPixelSizeMeters() / initResize;
  major_meters = cd->major * aPS;
  minor_meters = cd->minor * aPS;
  cd->major_meters = major_meters;
  perimeter_m = 2*PI*sqrt( (major_meters*major_meters + minor_meters*minor_meters)/2 );
  area_msq = PI * major_meters * minor_meters;
  cd->SizeFeatures[0] = area_msq;
  cd->SizeFeatures[1] = perimeter_m;
  cd->SizeFeatures[2] = major_meters;
  cd->SizeFeatures[3] = minor_meters;
  cd->SizeFeatures[4] = cd->major;
  cd->SizeFeatures[5] = cd->minor;
  cd->SizeFeatures[6] = cd->major / cd->minor;
  if( cd->has_edge_features && cd->nminor != 0 ) {
    cd->SizeFeatures[7] = cd->nmajor / cd->nminor;
    cd->SizeFeatures[8] = cd->nmajor / cd->major;
  } else {
    cd->SizeFeatures[7] = 1;
    cd->SizeFeatures[8] = 1;
  }

}
/*
inline int determine8quad_sm( int& x, int&y ) {
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
}*/
