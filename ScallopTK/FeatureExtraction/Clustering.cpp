
#include "Clustering.h"

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//                                 Definitions
//------------------------------------------------------------------------------

/*
void kmeansTextureClustering( candidate *cd, IplImage *deletethisargument ) {

	if( !cd->stats->active )
		return;

	// Perform 2 group kmeans clustering
	CvTermCriteria crit2 = cvTermCriteria( CV_TERMCRIT_ITER, 10, 1.0 );
	cvKMeans2( cd->stats->colors, 2, cd->stats->ids2group, crit2 );

	// Perform 3 group kmeans clustering
	//CvTermCriteria crit3 = cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0 );
	//cvKMeans2( cd->stats->colors, 3, cd->stats->ids3group, crit3 );

	// Restructure clustered components
	IplImage* mask = cd->stats->baseImgMask;
	int *position_ptr = (int*)cd->stats->positions->data.ptr;
	int *ids2_ptr = (int*)cd->stats->ids2group->data.ptr;
	//int *ids3_ptr = (int*)cd->stats->ids3group->data.ptr;
	int position_step = cd->stats->positions->step / sizeof(int);
	int ids2_step = cd->stats->ids2group->step / sizeof(int);
	//int ids3_step = cd->stats->ids3group->step / sizeof(int);
	assert( cd->stats->positions->step % sizeof(int) == 0 );
	assert( cd->stats->colors->step % sizeof(int) == 0 );
	int entries = cd->stats->kentries;
	IplImage* g2image = cvCreateImage( cvGetSize( mask ), IPL_DEPTH_8U, 1 );
	//IplImage* g3image = cvCreateImage( cvGetSize( mask ), IPL_DEPTH_8U, 1 );
	cvZero( g2image );
	//cvZero( g3image );
	for( int i=0; i < entries; i++ ) {
		((uchar*)(g2image->imageData + g2image->widthStep*position_ptr[0]))[position_ptr[1]] = *ids2_ptr + 1;
		//((uchar*)(g3image->imageData + g3image->widthStep*position_ptr[0]))[position_ptr[1]] = *ids3_ptr + 1;
		ids2_ptr += ids2_step;
		//ids3_ptr += ids3_step;
		position_ptr += position_step;
	}

	// Perform connected component analysis

}*/
