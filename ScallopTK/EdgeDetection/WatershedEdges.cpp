
#include "WatershedEdges.h"

//------------------------------------------------------------------------------
//                            Function Definitions
//------------------------------------------------------------------------------

const float R1_RATIO = 0.25;
const float R2_RATIO = 2.00;
const float SIGMA1 = 1.25f;
const int MASK_BORDER_SIZE = 2;

/*
void extractWatershedEdge( IplImage *img, candidate* kp, float scale ) {

	// Calculate outer/inner axis lengths for watershed markers
	float in_major = kp->major * R1_RATIO * scale;
	float in_minor = kp->minor * R1_RATIO * scale;
	float out_major = kp->major * R2_RATIO * scale;
	float out_minor = kp->minor * R2_RATIO * scale;

	// Calculate Bounding Box Size
	float angle = (kp->angle)*PI/180; 
	double tr = atan( -out_minor * tan( angle ) / out_major );
	double tc = atan( (out_minor / out_major) / tan( angle ) );
	int r_min = kp->r*scale + out_major*cos(tr)*cos(angle) - out_minor*sin(tr)*sin(angle);
	int c_min = kp->c*scale + out_major*cos(tc)*sin(angle) + out_minor*sin(tc)*cos(angle);
	int r_max = 2*kp->r*scale - r_min;
	int c_max = 2*kp->c*scale - c_min;

	// Adjust - Edit above and remove later
	if( r_min > r_max ) {
		int temp = r_min;
		r_min = r_max;
		r_max = temp;
	}
	if( c_min > c_max ) {
		int temp = c_min;
		c_min = c_max;
		c_max = temp;
	}	

	// Increase by constant factor
	r_min = r_min - MASK_BORDER_SIZE;
	r_max = r_max + MASK_BORDER_SIZE;
	c_min = c_min - MASK_BORDER_SIZE;
	c_max = c_max + MASK_BORDER_SIZE;
	
	// Adjust for Image Boundaries
	if( r_min < 0 )
		r_min = 0;
	if( r_max >= img->height )
		r_max = img->height - 1;
	if( c_min < 0 )
		c_min = 0;
	if( c_max >= img->width )
		c_max = img->width - 1;

	// Calculate window widths
	int c_size = c_max - c_min;
	int r_size = r_max - r_min;

	//EXIT CASES - Region too small 
	if( c_max <= 0 || r_max <= 0  || c_min >= img->width || r_min >= img->height || c_size < 3 || r_size < 3 )
		return;

	// Isolate ROI
	cvSetImageROI(img, cvRect(c_min, r_min, c_size, r_size));

	// Create filter mask - Optimize later
	IplImage *mask = cvCreateImage( cvSize(c_size, r_size), IPL_DEPTH_32S, 1 );
	drawEllipseRing( mask, kp->r*scale-r_min, kp->c*scale-c_min, kp->angle, in_major, in_minor, out_major );
	
	// Extract watershed contour
	cvWatershed(img, mask);
	kp->stats->wsMask = mask;
	kp->stats->wsOffset.x = c_min;
	kp->stats->wsOffset.y = r_min;

	// Set NULL ROI
	cvResetImageROI(img);
	
	// Show IP
	//showIP( img, mask, kp );
}*/

void calcAvgMax( IplImage *img, float& avg, float& max, float& min ) {

	// Collect input image properties
	int nChan = img->nChannels;
	int depth = img->depth;
	int width = img->width;
	int height = img->height;
	int step = img->widthStep;

	//Sort all values in filter result
	avg = 0;
	max = 0;
	min = 1e20;
	for( int i=0; i<height; i++ ) {
		for( int j=0; j<width; j++ ) {
			float value = (((float*)(img->imageData + i*step))[j]);
			avg += value;
			if( max < value )
				max = value;
			if( min > value )
				min = value;
		}
	}
	avg = avg / ( width * height );
}

// Extract contours from kps
void extractScallopContours( GradientChain& Gradients, vector<candidate*> cds ) {

	// Create base of pyramid (3-level incrementally scaled) if need to
	float lvl_intvl = ( Gradients.maxRad - Gradients.minRad ) / 3;
	float resize_factor1 = MIN_RAD_WATERSHED / Gradients.minRad;
	float resize_factor2 = MIN_RAD_WATERSHED / (Gradients.minRad + lvl_intvl);
	float resize_factor3 = MIN_RAD_WATERSHED / (Gradients.minRad + 2*lvl_intvl);
	IplImage *level[3];
	level[0] = Gradients.WatershedInput;
	if( resize_factor1 < 0.97f ) {
		int nwidth = resize_factor1 * level[0]->width;
		int nheight = resize_factor1 * level[0]->height;
		level[0] = cvCreateImage( cvSize(nwidth, nheight), level[0]->depth, level[0]->nChannels );
		cvResize( Gradients.WatershedInput, level[0] );
	} else {
		resize_factor1 = 1.0f;
	}

	// Create level chain
	level[1] = Gradients.WatershedInput;
	if( resize_factor2 < 0.97f ) {
		float relative = resize_factor2 / resize_factor1;
		int nwidth = relative * level[0]->width;
		int nheight = relative * level[0]->height;
		level[1] = cvCreateImage( cvSize(nwidth, nheight), level[0]->depth, level[0]->nChannels );
		cvResize( level[0], level[1] );
	} else {
		resize_factor2 = 1.0f;
	}
	level[2] = Gradients.WatershedInput;
	if( resize_factor3 < 0.97f ) {
		float relative = resize_factor3 / resize_factor2;
		int nwidth = relative * level[1]->width;
		int nheight = relative * level[1]->height;
		level[2] = cvCreateImage( cvSize(nwidth, nheight), level[0]->depth, level[0]->nChannels );
		cvResize( level[1], level[2] );
	} else {
		resize_factor3 = 1.0f;
	}

	// Calculate level stats
	float minRad = Gradients.minRad * resize_factor1;
	float maxRad = Gradients.maxRad * resize_factor1;
	float scales[3], start[3], end[3];
	scales[0] = Gradients.scale * resize_factor1;
	scales[1] = Gradients.scale * resize_factor2;
	scales[2] = Gradients.scale * resize_factor3;
	start[0] = Gradients.minRad / Gradients.scale;
	start[1] = end[0] = (Gradients.minRad + lvl_intvl) / Gradients.scale;
	start[2] = end[1] = (Gradients.minRad + lvl_intvl*2) / Gradients.scale;
	end[2] = Gradients.maxRad / Gradients.scale;
	
	// Create temp candidate point for input (because we need to scale each kp)
	candidate cd;

	// Run watersheds
	for( unsigned int i=0; i<cds.size(); i++ ) {

		// Choose which level to extract from (based off minor axis)
		int lvl = 0;
		/*if( cds[i]->minor <= end[0] )
			lvl = 0;
		else if( cds[i]->minor <= end[1] )
			lvl = 1;
		else
			lvl = 2;*/

		// Run extraction algorithm
		//extractWatershedEdge( level[lvl], cds[i], scales[lvl] );
		//extractWatershedEdge( aux, cds[i], 1.0 );

		// Adjust output for scale
		//TODO
	}

	// Deallocations
	if( resize_factor1 < 1.0f ) 
		cvReleaseImage( &level[0] );	
	if( resize_factor2 < 1.0f ) 
		cvReleaseImage( &level[1] );
	if( resize_factor3 < 1.0f ) 
		cvReleaseImage( &level[2] );
}
