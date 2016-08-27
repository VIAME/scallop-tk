
#include "HoG.h"

namespace ScallopTK
{

//------------------------------------------------------------------------------
//                             Function Prototypes
//------------------------------------------------------------------------------

CvMat* calculateHOG_window(IplImage** integrals, CvRect window, int normalization, int bins);
IplImage** calculateIntegralHOG(IplImage* in);

//------------------------------------------------------------------------------
//                                 Definitions
//------------------------------------------------------------------------------

HoGFeatureGenerator::HoGFeatureGenerator( IplImage *img_gs, float minR, float maxR, int index ) {

  // Check image type
  assert( img_gs->depth == IPL_DEPTH_32F );
  assert( img_gs->nChannels == 1 );

  // Assign internal variables
  minR = minRad;
  maxR = maxRad;

  // Create base integrals image
  integrals = calculateIntegralHOG(img_gs);

  // Initialize default options
  bins = 8;
  add_ratio = 1.4;

  // Set output index
  output_index = index;

}

HoGFeatureGenerator::~HoGFeatureGenerator() {
  for (int k = 0; k < 9; k++) 
    cvReleaseImage(&integrals[k]);
}

void HoGFeatureGenerator::Generate( CandidatePtrVector& cds ) {
  for( unsigned int i=0; i<cds.size(); i++ ) {
    if( !GenerateSingle( cds[i] ) ) {
      cds[i]->isActive = false;
    }
  }
}

// Generates a HoG feature vector for the Candidate point
bool HoGFeatureGenerator::GenerateSingle( Candidate* cd ) {

  // Check if NULL
  if( cd == NULL )
    return false;

  // Generate statistics
  double window_radius = ceil( cd->major*add_ratio );
  int lower_r = cd->r - window_radius;
  int upper_r = ceil(cd->r + window_radius);
  int lower_c = cd->c - window_radius;
  int upper_c = ceil(cd->c + window_radius);
  int window_width = upper_c - lower_c;
  int window_height = upper_r - lower_r;

  // Check kp size
  if( window_width < HOG_PIXEL_WIDTH_REQUIRED )
    return false;

  //cout << endl;
  //cout << lower_r << " " << upper_r << " " << integrals[0]->height << endl;
  //cout << lower_c << " " << upper_c << " " << integrals[0]->width << endl;

  // Calculate HoG Windows
  cd->hogResults[output_index] = calculateHOG_window(integrals, cvRect(lower_c, lower_r, window_width, window_height), 
    HOG_NORMALIZATION_METHOD, bins );

  return true;
}

// Old Methods
/*void calculateRHoG( Candidate *cd, IplImage *base ) {
  if( !cd->stats->active )
    return;
  assert( cd->stats->imgAdj64->width == 64 && cd->stats->imgAdj64->height == 64 );
  IplImage **integrals = calculateIntegralHOG(cd->stats->imgAdj64);
  cd->stats->RHoG = calculateHOG_window(integrals, cvRect(0, 0, 64, 64), CV_L2, 8, 8);
  for (int k = 0; k < 9; k++) 
    cvReleaseImage(&integrals[k]);
}

void calculateRHoG16( Candidate *cd, IplImage *base ) {
  if( !cd->stats->active )
    return;
  assert( cd->stats->imgAdj64->width == 64 && cd->stats->imgAdj64->height == 64 );
  IplImage **integrals = calculateIntegralHOG(cd->stats->imgAdj64);
  cd->stats->RHoG16 = calculateHOG_window(integrals, cvRect(0, 0, 64, 64), CV_L2, 16, 16);
  for (int k = 0; k < 9; k++) 
    cvReleaseImage(&integrals[k]);
}

void calculateCHoG( Candidate *cd, IplImage *base ) {
  if( !cd->stats->active )
    return;
  assert( cd->stats->imgAdj64->width == 64 && cd->stats->imgAdj64->height == 64 );
  IplImage **integrals = calculateIntegralHOG(cd->stats->imgAdj64);
  cd->stats->CHoG = calculateHOG_window(integrals, cvRect(0, 0, 64, 64), -1, 8, 8);
  for (int k = 0; k < 9; k++) 
    cvReleaseImage(&integrals[k]);
}

void HoGTest( IplImage *gs, CandidatePtrVector cds ) {
  IplImage **integrals = calculateIntegralHOG(gs);
  for( int i=0; i<cds.size(); i++ ) {    
    calculateHOG_window(integrals, cvRect(-40, 0, 124, 64), -1, 8, 8);
  }
  for (int k = 0; k < 9; k++) 
    cvReleaseImage(&integrals[k]);
}*/

// --------------------------------------------------------------------------------------------
// NOTE: Below code written by:
// http://smsoftdev-solutions.blogspot.com/2009/08/integral-histogram-for-fast-calculation.html

/*Function to calculate the integral histogram*/
IplImage** calculateIntegralHOG(IplImage* in) {

  /*Normalize*/

  //IplImage* img_gray = cvCloneImage( in );
  //cvEqualizeHist(img_gray,img_gray);
  IplImage* img_gray = in;

  /* Calculate the derivates of the grayscale image in the x and y directions using a sobel operator and obtain 2 
  gradient images for the x and y directions*/

  IplImage *xsobel = cvCreateImage( cvGetSize( img_gray ), img_gray->depth, 1 );
  IplImage *ysobel = cvCreateImage( cvGetSize( img_gray ), img_gray->depth, 1 );
  cvSobel(img_gray, xsobel, 1, 0, 3);
  cvSobel(img_gray, ysobel, 0, 1, 3);
  //cvReleaseImage(&img_gray);

  /* Create an array of 9 images (9 because I assume bin size 20 degrees and unsigned gradient ( 180/20 = 9), 
  one for each bin which will have zeroes for all pixels, except for the pixels in the original image for which 
  the gradient values correspond to the particular bin. These will be referred to as bin images. These bin images 
  will be then used to calculate the integral histogram, which will quicken the calculation of HOG descriptors */

  IplImage** bins = (IplImage**) malloc(9 * sizeof(IplImage*));
  for (int i = 0; i < 9 ; i++) {
    bins[i] = cvCreateImage(cvGetSize(in), IPL_DEPTH_32F,1);
    cvSetZero(bins[i]);
  }


  /* Create an array of 9 images ( note the dimensions of the image, the cvIntegral() function requires the size 
  to be that), to store the integral images calculated from the above bin images. These 9 integral images together 
  constitute the integral histogram */

  IplImage** integrals = (IplImage**) malloc(9 * sizeof(IplImage*)); for (int i = 0; i < 9 ; i++) {
    integrals[i] = cvCreateImage(cvSize(in->width + 1, in->height + 1),
      IPL_DEPTH_64F,1);
  }

  /* Calculate the bin images. The magnitude and orientation of the gradient at each pixel is calculated using the 
  xsobel and ysobel images.{Magnitude = sqrt(sq(xsobel) + sq(ysobel) ), gradient = itan (ysobel/xsobel) }. Then 
  according to the orientation of the gradient, the value of the corresponding pixel in the corresponding image is set */


  int x, y;
  float temp_gradient, temp_magnitude;
  for (y = 0; y <in->height; y++) {

    /* ptr1 and ptr2 point to beginning of the current row in the xsobel and ysobel images respectively. ptrs[i] 
    point to the beginning of the current rows in the bin images */

    float* ptr1 = (float*) (xsobel->imageData + y * (xsobel->widthStep));
    float* ptr2 = (float*) (ysobel->imageData + y * (ysobel->widthStep));
    float** ptrs = (float**) malloc(9 * sizeof(float*));
    for (int i = 0; i < 9 ;i++){
      ptrs[i] = (float*) (bins[i]->imageData + y * (bins[i]->widthStep));
    }

    /*For every pixel in a row gradient orientation and magnitude are calculated and corresponding values set for 
    the bin images. */

    for (x = 0; x <in->width; x++) {

      /* if the xsobel derivative is zero for a pixel, a small value is added to it, to avoid division by zero. 
      atan returns values in radians, which on being converted to degrees, correspond to values between -90 and 90 degrees. 
      90 is added to each orientation, to shift the orientation values range from {-90-90} to {0-180}. This is just 
      a matter of convention. {-90-90} values can also be used for the calculation. */

      if (ptr1[x] == 0){
        temp_gradient = ((atan(ptr2[x] / (ptr1[x] + 0.00001))) * (180/ PI)) + 90;
      }
      else{
        temp_gradient = ((atan(ptr2[x] / ptr1[x])) * (180 / PI)) + 90;
      }
      temp_magnitude = sqrt((ptr1[x] * ptr1[x]) + (ptr2[x] * ptr2[x]));

      /*The bin image is selected according to the gradient values. The corresponding pixel value is made equal to 
      the gradient magnitude at that pixel in the corresponding bin image */

      if (temp_gradient <= 20) {
        ptrs[0][x] = temp_magnitude;
      }
      else if (temp_gradient <= 40) {
        ptrs[1][x] = temp_magnitude;
      }
      else if (temp_gradient <= 60) {
        ptrs[2][x] = temp_magnitude;
      }
      else if (temp_gradient <= 80) {
        ptrs[3][x] = temp_magnitude;
      }
      else if (temp_gradient <= 100) {
        ptrs[4][x] = temp_magnitude;
      }
      else if (temp_gradient <= 120) {
        ptrs[5][x] = temp_magnitude;
      }
      else if (temp_gradient <= 140) {
        ptrs[6][x] = temp_magnitude;
      }
      else if (temp_gradient <= 160) {
        ptrs[7][x] = temp_magnitude;
      }
      else {
        ptrs[8][x] = temp_magnitude;
      }
    }
  }

  cvReleaseImage(&xsobel);
  cvReleaseImage(&ysobel);

  /*Integral images for each of the bin images are calculated*/

  for (int i = 0; i <9 ; i++){
    cvIntegral(bins[i], integrals[i]);
  }

  for (int i = 0; i <9 ; i++){
    cvReleaseImage(&bins[i]);
  }

  /*The function returns an array of 9 images which consitute the integral histogram*/

  return (integrals);

}

/* The following function takes as input the rectangular cell for which the histogram of oriented gradients has to be 
calculated, a matrix hog_cell of dimensions 1x9 to store the bin values for the histogram, the integral histogram, 
and the normalization scheme to be used. No normalization is done if normalization = -1 */

void calculateHOG_rect(CvRect cell, CvMat* hog_cell,
  IplImage** integrals, int normalization) {


    /* Calculate the bin values for each of the bin of the histogram one by one */

    for (int i = 0; i < 9 ; i++){

      float a =((double*)(integrals[i]->imageData + (cell.y)
        * (integrals[i]->widthStep)))[cell.x];
      float b = ((double*) (integrals[i]->imageData + (cell.y + cell.height)
        * (integrals[i]->widthStep)))[cell.x + cell.width];
      float c = ((double*) (integrals[i]->imageData + (cell.y)
        * (integrals[i]->widthStep)))[cell.x + cell.width];
      float d = ((double*) (integrals[i]->imageData + (cell.y + cell.height)
        * (integrals[i]->widthStep)))[cell.x];

      ((float*) hog_cell->data.fl)[i] = (a + b) - (c + d);

    }
    /*Normalize the matrix*/
    //if (normalization != -1){
    //  cvNormalize(hog_cell, hog_cell, 1, 0, normalization);
    //}

}

/* This function takes in a block as a rectangle and
calculates the hog features for the block by dividing
it into cells of size cell(the supplied parameter),
calculating the hog features for each cell using the
function calculateHOG_rect(...), concatenating the so
obtained vectors for each cell and then normalizing over
the concatenated vector to obtain the hog features for a
block */

void calculateHOG_block(CvRect block, CvMat* hog_block, 
  IplImage** integrals, int normalization) {

  CvMat vector_cell;    
  
  int centerx = block.x + block.width / 2;
  int centery = block.y + block.height / 2;
  int width1 = centerx - block.x;
  int width2 = block.width - width1;
  int height1 = centery - block.y;
  int height2 = block.height - height1;

  cvGetCols(hog_block, &vector_cell, 0, 9);
  calculateHOG_rect(cvRect(block.x, block.y, width1, height1), 
        &vector_cell, integrals, -1);
  cvGetCols(hog_block, &vector_cell, 9, 18);
  calculateHOG_rect(cvRect(centerx, block.y, width2, height1), 
        &vector_cell, integrals, -1);
  cvGetCols(hog_block, &vector_cell, 18, 27);
  calculateHOG_rect(cvRect(block.x, centery, width1, height2), 
        &vector_cell, integrals, -1);
  cvGetCols(hog_block, &vector_cell, 27, 36);
  calculateHOG_rect(cvRect(centerx, centery, width2, height2), 
        &vector_cell, integrals, -1);

  if (normalization != -1)
    cvNormalize(hog_block, hog_block, 1, 0, normalization);
}

/*void calculateHOG_block(CvRect block, CvMat* hog_block, 
  IplImage** integrals, int normalization) {

  CvMat vector_cell;
  int startcol = 0;
  double cellWidth = (double)block.width / 2;
  double cellHeight = (double)block.height / 2;
  double cell_start_y = block.y;
  for( int i=0; i<2; i++ ) {
    double cell_start_x = block.x;
    for( int j=0; j<2; j++ ) {
      
      cvGetCols(hog_block, &vector_cell, startcol, 
        startcol + 9);

      calculateHOG_rect(cvRect(round(cell_start_x),
        round(cell_start_y), cellWidth, cellHeight), 
        &vector_cell, integrals, -1);

      startcol += 9;
      cell_start_x += cellWidth;
    }
    cell_start_y += cellHeight;
  }

  if (normalization != -1)
    cvNormalize(hog_block, hog_block, 1, 0, 
    normalization);
}*/

/* This function takes in a window(64x128 pixels,
but can be easily modified for other window sizes)
and calculates the hog features for the window. It
can be used to calculate the feature vector for a 
64x128 pixel image as well. This window/image is the
training/detection window which is used for training
or on which the final detection is done. The hog
features are computed by dividing the window into
overlapping blocks, calculating the hog vectors for
each block using calculateHOG_block(...) and
concatenating the so obtained vectors to obtain the
hog feature vector for the window*/

CvMat* calculateHOG_window(IplImage** integrals,
  CvRect window, int normalization, int bins) 
{
  int imHeight = integrals[0]->height;
  int imWidth = integrals[0]->width;

  CvMat* window_feature_vector = cvCreateMat(1,(bins-1)*(bins-1)*36, CV_32FC1);

  double cell_height = (double)window.height / bins;
  double cell_width = (double)window.width / bins;
  
  CvMat vector_block;
  int startcol = 0;
  double block_start_y = window.y;
  for (int i=0; i<bins-1; i++) {
    double block_start_x = window.x;
    for (int j=0; j<bins-1; j++ ) { 

      // Reference position to insert this blocks features
      cvGetCols(window_feature_vector, &vector_block,
        startcol, startcol + 36);

      // Check if we have enough data to process this block
      if( block_start_x < 0 || block_start_y < 0 ||
        ceil( block_start_x + cell_width * 2 )+1 /*<--quickfix*/ >= imWidth ||
        ceil( block_start_y + cell_height * 2 )+1 /*<--quickfix*/ >= imHeight ) {

        // 0 set block
        for( int i=0; i<36; i++ ) {
          ((float*) vector_block.data.fl)[i] = 0.0f;
        }

      // Process block
      } else {
        
        calculateHOG_block(cvRect(dround(block_start_x),
          dround(block_start_y), dround(cell_width * 2), 
          dround(cell_height * 2)), &vector_block, integrals, 
          normalization);
      }

      // Increment Position
      startcol += 36;
      block_start_x += cell_width;
    }
    block_start_y += cell_height;
  }
  return (window_feature_vector);
}

#ifdef unused

CvMat* train_64x128(char *prefix, char *suffix, CvSize cell,
  CvSize window, int number_samples, int start_index,
  int end_index, char *savexml = NULL, int canny = 0,
  int block = 1, int normalization = 4) 
{

  char filename[50] = "\0", number[8];
  int prefix_length;
  prefix_length = strlen(prefix);
  int bins = 9;

  /* A default block size of 2x2 cells is considered */

  int block_width = 2, block_height = 2;

  /* Calculation of the length of a feature vector for
  an image (64x128 pixels)*/

  int feature_vector_length;
  feature_vector_length = (((window.width -
    cell.width * block_width)/ cell.width) + 1) *
    (((window.height - cell.height * block_height)
    / cell.height) + 1) * 36;

  /* Matrix to store the feature vectors for
  all(number_samples) the training samples */

  CvMat* training = cvCreateMat(number_samples,
    feature_vector_length, CV_32FC1);

  CvMat row;
  CvMat* img_feature_vector;
  IplImage** integrals;
  int i = 0, j = 0;

  //printf("Beginning to extract HoG features from
  //  positive images\n");

    strcat(filename, prefix);

  /* Loop to calculate hog features for each
  image one by one */

  for (i = start_index; i <= end_index; i++) 
  {
    cvtInt(number, i);
    strcat(filename, number);
    strcat(filename, suffix);
    IplImage* img = cvLoadImage(filename);

    /* Calculation of the integral histogram for
    fast calculation of hog features*/

    integrals = calculateIntegralHOG(img);
    cvGetRow(training, &row, j);
    img_feature_vector
      = calculateHOG_window(integrals, cvRect(0, 0,
      window.width, window.height), normalization);
    cvCopy(img_feature_vector, &row);
    j++;
    printf("%s\n", filename);
    filename[prefix_length] = '\0';
    for (int k = 0; k < 9; k++) 
    {
      cvReleaseImage(&integrals[k]);
    }
  }
  if (savexml != NULL) 
  {
    cvSave(savexml, training);
  }

  return training;
}

/* This function is almost the same as
train_64x128(...), except the fact that it can
take as input images of bigger sizes and
generate multiple samples out of a single
image.

It takes 2 more parameters than
train_64x128(...), horizontal_scans and
vertical_scans to determine how many samples
are to be generated from the image. It
generates horizontal_scans x vertical_scans
number of samples. The meaning of rest of the
parameters is same.

For example for a window size of
64x128 pixels, if a 320x240 pixel image is
given input with horizontal_scans = 5 and
vertical scans = 2, then it will generate to
samples by considering windows in the image
with (x,y,width,height) as (0,0,64,128),
(64,0,64,128), (128,0,64,128), .....,
(0,112,64,128), (64,112,64,128) .....
(256,112,64,128)

The function takes non-overlapping windows
from the image except the last row and last
column, which could overlap with the second
last row or second last column. So the values
of horizontal_scans and vertical_scans passed
should be such that it is possible to perform
that many scans in a non-overlapping fashion
on the given image. For example horizontal_scans
= 5 and vertical_scans = 3 cannot be passed for
a 320x240 pixel image as that many vertical scans
are not possible for an image of height 240
pixels and window of height 128 pixels. */

CvMat* train_large(char *prefix, char *suffix,
  CvSize cell, CvSize window, int number_images,
  int horizontal_scans, int vertical_scans,
  int start_index, int end_index,
  char *savexml = NULL, int normalization = 4)
{
  char filename[50] = "\0", number[8];
  int prefix_length;
  prefix_length = strlen(prefix);
  int bins = 9;

  /* A default block size of 2x2 cells is considered */

  int block_width = 2, block_height = 2;

  /* Calculation of the length of a feature vector for
  an image (64x128 pixels)*/

  int feature_vector_length;
  feature_vector_length = (((window.width -
    cell.width * block_width) / cell.width) + 1) *
    (((window.height - cell.height * block_height)
    / cell.height) + 1) * 36;

  /* Matrix to store the feature vectors for
  all(number_samples) the training samples */

  CvMat* training = cvCreateMat(number_images
    * horizontal_scans * vertical_scans,
    feature_vector_length, CV_32FC1);

  CvMat row;
  CvMat* img_feature_vector;
  IplImage** integrals;
  int i = 0, j = 0;
  strcat(filename, prefix);

  printf("Beginning to extract HoG features from negative images\n");

    /* Loop to calculate hog features for each
    image one by one */

    for (i = start_index; i <= end_index; i++) 
    {
      cvtInt(number, i);
      strcat(filename, number);
      strcat(filename, suffix);
      IplImage* img = cvLoadImage(filename);
      integrals = calculateIntegralHOG(img);
      for (int l = 0; l < vertical_scans - 1; l++)
      {
        for (int k = 0; k < horizontal_scans - 1; k++)
        {
          cvGetRow(training, &row, j);
          img_feature_vector = calculateHOG_window(
            integrals, cvRect(window.width * k,
            window.height * l, window.width,
            window.height), normalization);

          cvCopy(img_feature_vector, &row);
          j++;
        }

        cvGetRow(training, &row, j);

        img_feature_vector = calculateHOG_window(
          integrals, cvRect(img->width - window.width,
          window.height * l, window.width,
          window.height), normalization);

        cvCopy(img_feature_vector, &row);
        j++;
      }

      for (int k = 0; k < horizontal_scans - 1; k++)
      {
        cvGetRow(training, &row, j);

        img_feature_vector = calculateHOG_window(
          integrals, cvRect(window.width * k,
          img->height - window.height, window.width,
          window.height), normalization);

        cvCopy(img_feature_vector, &row);
        j++;
      }
      cvGetRow(training, &row, j);

      img_feature_vector = calculateHOG_window(integrals,
        cvRect(img->width - window.width, img->height -
        window.height, window.width, window.height),
        normalization);

      cvCopy(img_feature_vector, &row);
      j++;

      printf("%s\n", filename);
      filename[prefix_length] = '\0';
      for (int k = 0; k < 9; k++)
      {
        cvReleaseImage(&integrals[k]);
      }

      cvReleaseImage(&img);

    }

    printf("%d negative samples created \n",
      training->rows);

    if (savexml != NULL)
    {
      cvSave(savexml, training);
      printf("Negative samples saved as %s\n",
        savexml);
    }

    return training;

}


/* This function trains a linear support vector
machine for object classification. The synopsis is
as follows :

pos_mat : pointer to CvMat containing hog feature
vectors for positive samples. This may be
NULL if the feature vectors are to be read
from an xml file

neg_mat : pointer to CvMat containing hog feature
vectors for negative samples. This may be
NULL if the feature vectors are to be read
from an xml file

savexml : The name of the xml file to which the learnt
svm model should be saved

pos_file: The name of the xml file from which feature
vectors for positive samples are to be read.
It may be NULL if feature vectors are passed
as pos_mat

neg_file: The name of the xml file from which feature
vectors for negative samples are to be read.
It may be NULL if feature vectors are passed
as neg_mat*/


void trainSVM(CvMat* pos_mat, CvMat* neg_mat, char *savexml,
  char *pos_file = NULL, char *neg_file = NULL) 
{


  /* Read the feature vectors for positive samples */
  if (pos_file != NULL) 
  {
    printf("positive loading...\n");
    pos_mat = (CvMat*) cvLoad(pos_file);
    printf("positive loaded\n");
  }

  /* Read the feature vectors for negative samples */
  if (neg_file != NULL)
  {
    neg_mat = (CvMat*) cvLoad(neg_file);
    printf("negative loaded\n");
  }

  int n_positive, n_negative;
  n_positive = pos_mat->rows;
  n_negative = neg_mat->rows;
  int feature_vector_length = pos_mat->cols;
  int total_samples;
  total_samples = n_positive + n_negative;

  CvMat* trainData = cvCreateMat(total_samples,
    feature_vector_length, CV_32FC1);

  CvMat* trainClasses = cvCreateMat(total_samples,
    1, CV_32FC1 );

  CvMat trainData1, trainData2, trainClasses1,
    trainClasses2;

  printf("Number of positive Samples : %d\n",
    pos_mat->rows);

  /*Copy the positive feature vectors to training
  data*/

  cvGetRows(trainData, &trainData1, 0, n_positive);
  cvCopy(pos_mat, &trainData1);
  cvReleaseMat(&pos_mat);

  /*Copy the negative feature vectors to training
  data*/

  cvGetRows(trainData, &trainData2, n_positive,
    total_samples);

  cvCopy(neg_mat, &trainData2);
  cvReleaseMat(&neg_mat);

  printf("Number of negative Samples : %d\n",
    trainData2.rows);

  /*Form the training classes for positive and
  negative samples. Positive samples belong to class
  1 and negative samples belong to class 2 */

  cvGetRows(trainClasses, &trainClasses1, 0, n_positive);
  cvSet(&trainClasses1, cvScalar(1));

  cvGetRows(trainClasses, &trainClasses2, n_positive,
    total_samples);

  cvSet(&trainClasses2, cvScalar(2));


  /* Train a linear support vector machine to learn from
  the training data. The parameters may played and
  experimented with to see their effects*/

  CvSVM svm(trainData, trainClasses, 0, 0,
    CvSVMParams(CvSVM::C_SVC, CvSVM::LINEAR, 0, 0, 0, 2,
    0, 0, 0, cvTermCriteria(CV_TERMCRIT_EPS,0, 0.01)));

  printf("SVM Training Complete!!\n");

  /*Save the learnt model*/

  if (savexml != NULL) {
    svm.save(savexml);
  }
  cvReleaseMat(&trainClasses);
  cvReleaseMat(&trainData);

}

#endif

}
