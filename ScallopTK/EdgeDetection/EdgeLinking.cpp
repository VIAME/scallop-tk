
#include "EdgeLinking.h"

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//                                 Definitions
//------------------------------------------------------------------------------

inline int returnRelativeDir( const int& dx1, const int& dx2, const int& dy1, const int& dy2 ) {
  int ddx = dx2 - dx1;
  int ddy = dy2 - dy1;
  if ( ddx == ddy ) 
    return 0; 
  else if ( dy2*dx1 > dy1*dx2 )
    return 1;
  return 2;
}

int cornetDetect() {
  return 0;
}

int groupInitialEdges( CvPoint* pos, int *ids, int size ) {

  // Check conditions
  assert(size>2);

  // Calculate dx/dy 1st derivatives
  int *dx = (int*) malloc( size * sizeof(int) );
  int *dy = (int*) malloc( size * sizeof(int) );
  for( int i=1; i<size; i++ ) {
    dx[i] = pos[i].x - pos[i-1].x;
    dy[i] = pos[i].y - pos[i-1].y;
  }
  dx[0] = pos[0].x - pos[size-1].x; 
  dy[0] = pos[0].y - pos[size-1].y; 

  // Calculate directions
  int *dir = (int*) malloc( size * sizeof(int) );
  for( int i=1; i<size; i++ ) {
    dir[i] = returnRelativeDir(dx[i-1],dx[i],dy[i-1],dy[i]);
  }
  dir[0] = returnRelativeDir(dx[size-1],dx[0],dy[size-1],dy[0]);

  // Create labels
  int num_groups = 1;
  int cur_label = 0;
  int last_dir = dir[0];
  ids[0] = cur_label;
  for( int i=1; i<size; i++ ) {
    if( last_dir == 0 && dir[i] != 0 ) {
      last_dir = dir[i];
      ids[i] = cur_label;
    } else if( dir[i] == 0 || dir[i] == last_dir ) {
      ids[i] = cur_label;
    } else if(i<size-2) {
      cur_label++;
      num_groups++;
      last_dir = dir[i];
      ids[i] = cur_label;
      ids[i+1] = cur_label;
      i=i+1;
    } else {
      ids[i] = cur_label;
    }
  }

  // Deallocations
  free( dir );
  free( dx );
  free( dy );

  return num_groups;  
}

void showConnectedPoints( CvPoint* pos, int size, IplImage *bin ) {

  IplImage *cc_color2 = cvCreateImage(cvGetSize(bin),IPL_DEPTH_8U,3);
  CvScalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
  for( int i=1; i<size; i++ ) {
    cvLine(cc_color2,pos[i-1],pos[i],color);
  }
  cvLine(cc_color2,pos[size-1],pos[0],color);
  showImage(cc_color2);
  cvReleaseImage(&cc_color2);
}

void showGroupedPoints( CvPoint* pos, int *ids, int size, IplImage *bin ) {

  assert(size>0);
  IplImage *cc_color2 = cvCreateImage(cvGetSize(bin),IPL_DEPTH_8U,3);
  CvScalar color = CV_RGB( rand()&255, rand()&255, rand()&255 );
  int id = ids[0];
  for( int i=0; i<size; i++ ) {
    //if( ids[i] != id ) {
      //color = CV_RGB( rand()&255, rand()&255, rand()&255 );
      //id = ids[i];
    //}
    cvSet2D(cc_color2,pos[i].y,pos[i].x,color);
  }
  showImage(cc_color2);
  cvReleaseImage(&cc_color2);
}

void printPoints( CvPoint* pos, int* ids, int size ) {

  cout << "CONTOUR:\n";
  for( int i=0; i<size; i++ ) 
    cout << pos[i].x << " " << pos[i].y << " " << ids[i] <<  endl;
}

int isStable(CvPoint* pts, int size) {

  return 0;
}

inline double dist_squared( CvPoint p1, CvPoint p2 ) {
  return (p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y);
}


int findStableMatches( CvSeq *seq, float minRad, float maxRad, CandidatePtrVector& kps, IplImage* bin ) {

  // Return value
  int retVal = -1;

  // Threshold Contour entries size
  int elements = seq->total;
  if( elements < 8 ) {
    return retVal;
  }

  // Gather statistics
  CvRect rect = cvBoundingRect( seq );
  int high = ( rect.height < rect.width ? rect.width : rect.height );
  int low = ( rect.height < rect.width ? rect.height : rect.width );

  // If bounding box is very small simply return
  if( low < minRad*2  ) {
    return retVal;
  }
  
  // Allocate Contour array
  CvPoint *group_pos = (CvPoint*) malloc(elements * sizeof(CvPoint));
  cvCvtSeqToArray(seq, group_pos, CV_WHOLE_SEQ);

  // Calculate arc and downsampling statistics
  double arc_length = cvArcLength( seq );
  double arc_approx = arc_length / 10;
  double rect_approx = 12*(float)high / (float)low;
  double downsample = 2 * elements / (rect_approx + arc_approx);
  double ds_length = arc_length / 4;

  // Perform downsampling
  int maxSize = downsample * elements;
  int newSize = 0;
  CvPoint *dsed = (CvPoint*) malloc(maxSize * sizeof(CvPoint));
  dsed[0] = CvPoint( group_pos[0] );
  CvPoint last = CvPoint( dsed[0] );
  newSize++;
  for( int i = 1; i < elements; i++ ) {
    double dist_so_far = dist_squared( group_pos[i], last );
    if( dist_so_far > ds_length && newSize < maxSize ) {
      dsed[newSize] = CvPoint( group_pos[i] );
      newSize++;
      last = CvPoint( group_pos[i] );
    }
  }

  // Check to make sure reduced Contour size is sufficient [quickfix: todo revise above]
  if( newSize < 6 ) {
    free(group_pos);
    free(dsed);
    return -1;
  }

  // Fit Ellipse
  CvPoint2D32f* input = (CvPoint2D32f*)malloc(newSize*sizeof(CvPoint2D32f));
  for( int i=0; i<newSize; i++ ) {
    input[i].x = dsed[i].x;
    input[i].y = dsed[i].y;
  }
  CvBox2D* box = (CvBox2D*)malloc(sizeof(CvBox2D));
  cvFitEllipse( input, newSize, box );

  // Threshold size
  float esize = PI*box->size.height*box->size.width/4.0f;
  if( esize < PI*maxRad*maxRad ) {

    // Add
    Candidate *kp = new Candidate;
    kp->angle = box->angle;
    kp->r = box->center.y;
    kp->c = box->center.x;
    kp->minor = box->size.width/2;
    kp->major = box->size.height/2;
    kp->magnitude = 0;
    kp->method = ADAPTIVE;
    kps.push_back( kp );
    retVal = 0;

  } else {

    // Interest point too large
    retVal = 1;
  }

  // Deallocations
  free(box);
  free(input);
  free(group_pos);
  free(dsed);

  return retVal;
}

