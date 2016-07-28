
#include "GaussianEdges.h"

//------------------------------------------------------------------------------
//                            Function Definitions
//------------------------------------------------------------------------------

// Takes a verticle gaussian derivative
IplImage *gaussDerivVerticle( IplImage *input, double sigma ) {
	IplImage *output = cvCreateImage( cvGetSize( input ), IPL_DEPTH_32F, input->nChannels );
	int filter_size = sigma * KERNEL_SIZE_PER_SIGMA;
	filter_size = filter_size + (filter_size+1)%2;
	CvMat* M = cvCreateMat(filter_size,1,CV_32FC1);
	int center = filter_size / 2;
	float sig2 = sigma * sigma;
	float sig3 = sigma * sig2;
	for( int i=0; i<filter_size; i++ ) {
		float pos = i - center;
		float value = -(pos/sig3)*exp(-pos*pos/(2*sig2));
		cvmSet(M, i, 0, value );
	}
	cvFilter2D(input,output,M);
	cvReleaseMat(&M);
	return output;
}

// Takes a horizontal gaussian derivative
IplImage *gaussDerivHorizontal( IplImage *input, double sigma ) {
	IplImage *output = cvCreateImage( cvGetSize( input ), IPL_DEPTH_32F, input->nChannels );
	int filter_size = sigma * KERNEL_SIZE_PER_SIGMA;
	filter_size = filter_size + (filter_size+1)%2;
	CvMat* M = cvCreateMat(1,filter_size,CV_32FC1);
	int center = filter_size / 2;
	float sig2 = sigma * sigma;
	float sig3 = sigma * sig2;
	for( int i=0; i<filter_size; i++ ) {
		float pos = i - center;
		float value = -(pos/sig3)*exp(-pos*pos/(2*sig2));
		cvmSet(M, 0, i, value );
	}
	cvFilter2D(input,output,M);
	cvReleaseMat(&M);
	return output;
}


// Takes a verticle box derivative
IplImage *boxDerivVerticle( IplImage *input ) {
	IplImage *output = cvCreateImage( cvGetSize( input ), IPL_DEPTH_32F, input->nChannels );
	CvMat* M = cvCreateMat(3,3,CV_32FC1);
	cvmSet(M, 0, 0, 1.0f );
	cvmSet(M, 0, 1, 2.0f );
	cvmSet(M, 0, 2, 1.0f );
	cvmSet(M, 1, 0, 0.0f );
	cvmSet(M, 1, 1, 0.0f );
	cvmSet(M, 1, 2, 0.0f );
	cvmSet(M, 2, 0, -1.0f );
	cvmSet(M, 2, 1, -2.0f );
	cvmSet(M, 2, 2, -1.0f );
	cvFilter2D(input,output,M);
	cvReleaseMat(&M);
	return output;
}

// Takes a verticle box derivative
IplImage *boxDerivHorizontal( IplImage *input ) {
	IplImage *output = cvCreateImage( cvGetSize( input ), IPL_DEPTH_32F, input->nChannels );
	CvMat* M = cvCreateMat(3,3,CV_32FC1);
	cvmSet(M, 0, 0, 1.0f );
	cvmSet(M, 0, 1, 0.0f );
	cvmSet(M, 0, 2, -1.0f );
	cvmSet(M, 1, 0, 2.0f );
	cvmSet(M, 1, 1, 0.0f );
	cvmSet(M, 1, 2, -2.0f );
	cvmSet(M, 2, 0, 1.0f );
	cvmSet(M, 2, 1, 0.0f );
	cvmSet(M, 2, 2, -1.0f );
	cvFilter2D(input,output,M);
	cvReleaseMat(&M);
	return output;
}

// Takes a diagonal gaussian derivative
IplImage *gaussDerivAngle2( IplImage *input, double sigma ) {
	IplImage *output = cvCreateImage( cvGetSize( input ), IPL_DEPTH_32F, input->nChannels );
	int filter_size = sigma * KERNEL_SIZE_PER_SIGMA / sqrt((float)2);
	filter_size = filter_size + (filter_size+1)%2 + 2;
	CvMat* M = cvCreateMat(filter_size,filter_size,CV_32FC1);
	cvSetZero(M);
	int center = filter_size / 2;
	float sig2 = sigma * sigma;
	float sig3 = sigma * sig2;
	for( int i=0; i<filter_size; i++ ) {

		//Calculate Value
		float pos = i - center;
		float value = -(pos/sig3)*exp(-pos*pos/(2*sig2));

		//Bilinear Interpolate into bins
		float subpix = pos / sqrt(2.0f);
		float top = ceil( subpix );
		float bot = floor( subpix );
		float botdif = subpix - bot; 
		float topdif = 1 - botdif;
		top = top + center;
		bot = bot + center;
		float v1 = cvmGet(M, bot, bot);
		float v2 = cvmGet(M, bot, top);
		float v3 = cvmGet(M, top, bot);
		float v4 = cvmGet(M, top, top);
		cvmSet(M, bot, bot, value * topdif * topdif + v1);
		cvmSet(M, bot, top, value * topdif * botdif + v2);
		cvmSet(M, top, bot, value * botdif * topdif + v3);
		cvmSet(M, top, top, value * botdif * botdif + v4);
	}
	cvFilter2D(input,output,M);
	cvReleaseMat(&M);
	return output;
}


// Takes a diagonal gaussian derivative
IplImage *gaussDerivAngle4( IplImage *input, double sigma ) {
	IplImage *output = cvCreateImage( cvGetSize( input ), IPL_DEPTH_32F, input->nChannels );
	int filter_size = sigma * KERNEL_SIZE_PER_SIGMA / sqrt((float)2);
	filter_size = filter_size + (filter_size+1)%2 + 2;
	CvMat* M = cvCreateMat(filter_size,filter_size,CV_32FC1);
	cvSetZero(M);
	int center = filter_size / 2;
	float sig2 = sigma * sigma;
	float sig3 = sigma * sig2;
	for( int i=0; i<filter_size; i++ ) {

		//Calculate Value
		float pos = i - center;
		float value = -(pos/sig3)*exp(-pos*pos/(2*sig2));

		//Bilinear Interpolate into bins
		float subpix = pos / sqrt(2.0f);
		float top = ceil( subpix );
		float bot = floor( subpix );
		float botdif = subpix - bot; 
		float topdif = 1 - botdif;
		top = top + center;
		bot = bot + center;
		float v1 = cvmGet(M, filter_size-bot-1, bot);
		float v2 = cvmGet(M, filter_size-bot-1, top);
		float v3 = cvmGet(M, filter_size-top-1, bot);
		float v4 = cvmGet(M, filter_size-top-1, top);
		cvmSet(M, filter_size-bot-1, bot, value * topdif * topdif + v1);
		cvmSet(M, filter_size-bot-1, top, value * topdif * botdif + v2);
		cvmSet(M, filter_size-top-1, bot, value * botdif * topdif + v3);
		cvmSet(M, filter_size-top-1, top, value * botdif * botdif + v4);
	}
	cvFilter2D(input,output,M);
	cvReleaseMat(&M);
	return output;
}

IplImage *merge3Chan( IplImage *input ) {
	IplImage *output = cvCreateImage( cvGetSize( input ), input->depth, 1 );
	IplImage* ch1 = cvCreateImage( cvGetSize( input ), input->depth, 1 );
	IplImage* ch2 = cvCreateImage( cvGetSize( input ), input->depth, 1 );
	IplImage* ch3 = cvCreateImage( cvGetSize( input ), input->depth, 1 );
	cvAdd( ch1, ch2, output );
	cvAdd( ch3, output, output );
	cvReleaseImage(&ch1);
	cvReleaseImage(&ch2);
	cvReleaseImage(&ch3);
	return output;
}

IplImage *mergeAbs3Chan( IplImage *input ) {
	IplImage *output = cvCreateImage( cvGetSize( input ), input->depth, 1 );
	IplImage* ch1 = cvCreateImage( cvGetSize( input ), input->depth, 1 );
	IplImage* ch2 = cvCreateImage( cvGetSize( input ), input->depth, 1 );
	IplImage* ch3 = cvCreateImage( cvGetSize( input ), input->depth, 1 );
	cvSplit( input, ch1, ch2, ch3, NULL );
	cvScale( ch1, ch1, 2.0f);
	cvAbs( ch1, ch1 );
	cvAbs( ch2, ch2 );
	cvAbs( ch3, ch3 );
	//showImageRange( ch1 );
	cvAdd( ch1, ch2, output );
	cvAdd( ch3, output, output );
	cvReleaseImage(&ch1);
	cvReleaseImage(&ch2);
	cvReleaseImage(&ch3);
	return output;
}


// Calculate a color gradient 8 bit image
/*IplImage *singleGradient8bit( IplImage *input, float sigma, bool normalize ) {

	// Declare output
	IplImage *output = cvCreateImage(cvGetSize(input), IPL_DEPTH_8U, 3 );
	IplImage *gradient = calculateNetGradientMag( input, sigma );

	// Normalize
	if( normalize ) {

		// Split 3 channels
		IplImage* ch1 = cvCreateImage( cvGetSize(gradient), IPL_DEPTH_32F, 1 );
		IplImage* ch2 = cvCreateImage( cvGetSize(gradient), IPL_DEPTH_32F, 1 );
		IplImage* ch3 = cvCreateImage( cvGetSize(gradient), IPL_DEPTH_32F, 1 );
		IplImage* merged = cvCreateImage( cvGetSize(gradient), IPL_DEPTH_32F, 3 );
		cvSplit( gradient, ch1, ch2, ch3, NULL );
				
		// Scale range of each
		float avg, max, min;
		calcAvgMax( ch1, avg, max, min );
		cvScale( ch1, ch1, 2/(max+avg) );
		calcAvgMax( ch2, avg, max, min );
		cvScale( ch2, ch2, 2/(max+avg) );
		calcAvgMax( ch3, avg, max, min );
		cvScale( ch3, ch3, 2/(max+avg) );		

		// Merge into output
		cvMerge( ch1, ch2, ch3, NULL, merged );
		cvScale( merged, output, 255.0 );

		// Deallocate
		cvReleaseImage(&ch1);
		cvReleaseImage(&ch2);
		cvReleaseImage(&ch3);
		cvReleaseImage(&merged);

	} else {
		// No normalization
		cvScale( gradient, output, 10.0 );
	}

	// Deallocations
	cvReleaseImage( &gradient );
	return output;
}*/

/*void createWatershedMap( GradientChain& gc, IplImage *rgb8u ) {
	assert( gc.netCCGrad->width == gc.cannyEdges->width );
	gc.WatershedInput = cvCreateImage( cvGetSize( gc.cannyEdges ), IPL_DEPTH_8U, 3 );
	IplImage *ch1 = cvCreateImage( cvGetSize( gc.dMergedSig1 ), IPL_DEPTH_8U, 1 );
	IplImage *ch2 = cvCreateImage( cvGetSize( gc.netCCGrad ), IPL_DEPTH_8U, 1 );
	IplImage *ch3 = cvCreateImage( cvGetSize( gc.netCCGrad ), IPL_DEPTH_8U, 1 );
	cvScale(gc.dMergedSig1,ch1,255.0);
	cvScale(gc.netCCGrad,ch2,255.0);
	cvScale(gc.gsEdge, ch3, 255.0);
	cvMerge(ch1, ch2, ch3, NULL, gc.WatershedInput);
	cvScale( gc.WatershedInput, gc.WatershedInput, 0.3 );
	cvAdd( gc.WatershedInput, rgb8u, gc.WatershedInput );
	cvReleaseImage(&ch1);
	cvReleaseImage(&ch2);
	cvReleaseImage(&ch3);
}*/

// Create a chain of all gradient images we need across all operations
GradientChain createGradientChain( IplImage *img_lab, IplImage *img_gs_32f, IplImage *img_gs_8u, IplImage *img_rgb_8u, hfResults *color, float minRad, float maxRad ) {

	// Create chain
	GradientChain output;

	// Resize input img if needed
	float maxMinRequired = max( MIN_RAD_WATERSHED, MIN_RAD_TEMPLATE );
	float resize_factor = maxMinRequired / minRad;
	IplImage *input = img_lab;
	if( resize_factor < 0.95f ) {
		int nheight = resize_factor * img_lab->height;
		int nwidth = resize_factor * img_lab->width;
		input = cvCreateImage( cvSize(nwidth, nheight), IPL_DEPTH_32F, img_lab->nChannels );
		cvResize( img_lab, input );
	} else {
		resize_factor = 1.0f;
	}

	// Smooth input img if needed (note: will modify but its last time we use img)
	cvSmooth( input, input, CV_BLUR, 5, 5 );

	// Set stats
	output.maxRad = maxRad*resize_factor;
	output.minRad = minRad*resize_factor;
	output.scale = resize_factor;

	// Create Lab derivatives
	float adj_sigma_1 = LAB_GRAD_SIGMA * minRad / MIN_RAD_TEMPLATE;
	output.dxColorSig1 = gaussDerivHorizontal( input, adj_sigma_1 );
	output.dyColorSig1 = gaussDerivVerticle( input, adj_sigma_1 );
	output.dxMergedSig1 = mergeAbs3Chan( output.dxColorSig1 );
	output.dyMergedSig1 = mergeAbs3Chan( output.dyColorSig1 );
	cvScale(output.dxMergedSig1,output.dxMergedSig1,1.0/23.0);
	cvScale(output.dyMergedSig1,output.dyMergedSig1,1.0/23.0);
	output.dMergedSig1 = cvCreateImage( cvGetSize( output.dxMergedSig1 ), IPL_DEPTH_32F, 1 );
	cvAdd( output.dxMergedSig1, output.dyMergedSig1, output.dMergedSig1 );

	// Create color derivative
	output.dxCCGrad = gaussDerivHorizontal( color->EnvironmentMap, ENV_GRAD_SIGMA );
	output.dyCCGrad = gaussDerivVerticle( color->EnvironmentMap, ENV_GRAD_SIGMA );
	cvAbs( output.dyCCGrad, output.dyCCGrad );
	cvAbs( output.dxCCGrad, output.dxCCGrad );
	cvScale(output.dxCCGrad,output.dxCCGrad,1.0/0.50);
	cvScale(output.dyCCGrad,output.dyCCGrad,1.0/0.50);
	output.netCCGrad = cvCreateImage( cvGetSize( color->NetScallops ), IPL_DEPTH_32F, 1 );
	cvAdd( output.dxCCGrad, output.dyCCGrad, output.netCCGrad );

	// Create grayscale edge map
	output.gsEdge = cvCreateImage( cvGetSize( img_gs_32f ), IPL_DEPTH_32F, 1 );
	IplImage *bx = boxDerivHorizontal( img_gs_32f );
	IplImage *by = boxDerivVerticle( img_gs_32f );
	cvAbs( bx, bx );
	cvAbs( by, by );
	cvScale(by, by, 1.0f/1.70f);
	cvScale(bx, bx, 1.0f/1.70f);
	cvAdd( bx, by, output.gsEdge );

	// Extract canny edges
	output.cannyEdges = cvCreateImage( cvGetSize(output.dxMergedSig1), IPL_DEPTH_8U, 1 );
	cvSmooth( img_gs_8u, img_gs_8u, 2, 7, 7 );
	cvCanny( img_gs_8u, output.cannyEdges, 18, 28, 3 );

	// Create net template input
	output.dx = cvCreateImage( cvGetSize(output.dxMergedSig1), IPL_DEPTH_32F, 1 );
	output.dy = cvCreateImage( cvGetSize(output.dxMergedSig1), IPL_DEPTH_32F, 1 );
	cvAdd( output.dxCCGrad, output.dxMergedSig1, output.dx );
	cvAdd( output.dyCCGrad, output.dyMergedSig1, output.dy );
	cvAdd( output.dx, bx, output.dx );
	cvAdd( output.dy, by, output.dy );
	
	// Create net watershed map (deprecated)
	//createWatershedMap( output, img_rgb_8u );

	// Take Lab derivative magntitude and direction
	IplImage *lab_dx = cvCreateImage( cvGetSize( img_lab ), img_lab->depth, 3 );
	IplImage *lab_dy = cvCreateImage( cvGetSize( img_lab ), img_lab->depth, 3 );
	cvSobel( img_lab, lab_dx, 1, 0, 3 );
	cvSobel( img_lab, lab_dy, 0, 1, 3 );
	IplImage *lab_mag = cvCreateImage( cvGetSize( img_lab ), img_lab->depth, 1 );
	IplImage *lab_ori = cvCreateImage( cvGetSize( img_lab ), img_lab->depth, 1 );

	// Make a single pass on lab images to calc magnitude and orientation approx
	int lab_entries = lab_dx->width * lab_dy->height;
	float *ptr_dx = (float*)lab_dx->imageData;
	float *ptr_dy = (float*)lab_dy->imageData;
	float *ptr_mag = (float*)lab_mag->imageData;
	float *ptr_ori = (float*)lab_ori->imageData;	
	for( int i=0; i<lab_entries; i++ ) {
		float dx_mag = ptr_dx[0]*ptr_dx[0] + ptr_dx[1]*ptr_dx[1] + ptr_dx[2]*ptr_dx[2];
		float dy_mag = ptr_dy[0]*ptr_dy[0] + ptr_dy[1]*ptr_dy[1] + ptr_dy[2]*ptr_dy[2];
		dx_mag = sqrt( dx_mag );
		dy_mag = sqrt( dy_mag );
		if( *ptr_dx < 0.0f )
			dx_mag *= -1.0f;
		if( *ptr_dy < 0.0f )
			dy_mag *= -1.0f;
		*ptr_mag = sqrt(dx_mag*dx_mag+dy_mag*dy_mag);
		*ptr_ori = cvFastArctan(dy_mag, dx_mag);
		ptr_dx = ptr_dx + 3;
		ptr_dy = ptr_dy + 3;
		ptr_mag = ptr_mag + 1;
		ptr_ori = ptr_ori + 1;
	}

	// Set output lab mag/ori
	output.dLabMag = lab_mag;
	output.dLabOri = lab_ori;

	// Make a single pass on color class images to calc magnitude approx
	
		
	// Deallocate extraneous
	if( resize_factor != 1.0f ) 
		cvReleaseImage(&input);
	cvReleaseImage(&by);
	cvReleaseImage(&bx);
	cvReleaseImage( &lab_dx );
	cvReleaseImage( &lab_dy );

	return output;
}

// Deallocate gradient chain
void deallocateGradientChain( GradientChain& chain ) {

	cvReleaseImage( &chain.dxColorSig1 );
	cvReleaseImage( &chain.dxMergedSig1 );
	cvReleaseImage( &chain.dyColorSig1 );
	cvReleaseImage( &chain.dyMergedSig1 );	
	cvReleaseImage( &chain.dMergedSig1 );

	cvReleaseImage( &chain.dLabMag );
	cvReleaseImage( &chain.dLabOri );

	cvReleaseImage( &chain.dxCCGrad );
	cvReleaseImage( &chain.dyCCGrad );
	cvReleaseImage( &chain.netCCGrad );

	cvReleaseImage( &chain.gsEdge );
	cvReleaseImage( &chain.dx );
	cvReleaseImage( &chain.dy );

	//cvReleaseImage( &chain.WatershedInput );

	cvReleaseImage( &chain.cannyEdges );
}
