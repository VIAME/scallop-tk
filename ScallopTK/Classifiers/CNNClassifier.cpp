
#include "CNNClassifier.h"

#include "ScallopTK/Utilities/HelperFunctions.h"
#include "ScallopTK/Utilities/Filesystem.h"

namespace ScallopTK
{

using caffe::Blob;
using caffe::Net;
using caffe::Caffe;

CNNClassifier::CNNClassifier()
{
  initialClfr = NULL;
  suppressionClfr = NULL;
  isScallopDirected = false;
}

CNNClassifier::~CNNClassifier()
{
  deallocCNNs();
}

void CNNClassifier::deallocCNNs()
{
  if( initialClfr )
  {
    delete initialClfr;
    initialClfr = NULL;
  }

  if( suppressionClfr )
  {
    delete suppressionClfr;
    suppressionClfr = NULL;
  }
}

bool CNNClassifier::loadClassifiers(
  const SystemParameters& sysParams,
  const ClassifierParameters& clsParams )
{
  deallocCNNs();

  initialClfrLabels.clear();
  suppressionClfrLabels.clear();

  std::string dir = sysParams.RootClassifierDIR + clsParams.ClassifierSubdir;
  isScallopDirected = false;
  threshold = clsParams.Threshold;
  isTrainingMode = sysParams.IsTrainingMode;
  outputFolder = sysParams.OutputDirectory;
  trainingPercentKeep = sysParams.TrainingPercentKeep;

  // Auto-detect which GPU to use based on highest memory count
#ifdef CPU_ONLY
  deviceMode = Caffe::CPU;
#else
  int deviceCount = 0;
  cudaGetDeviceCount( &deviceCount );

  if( deviceCount <= 0 )
  {
    deviceMode = Caffe::CPU;
  }
  else
  {
    for( int i = 0; i < deviceCount; ++i )
    {
      cudaDeviceProp prop;
      cudaGetDeviceProperties( &prop, i );

      if( prop.totalGlobalMem > deviceMem )
      {
        deviceID = i;
        deviceMem = prop.totalGlobalMem;
        deviceMode = Caffe::GPU;
      }
    }
  }
#endif

  Caffe::set_mode( deviceMode );

  if( deviceMode == Caffe::GPU && deviceID >= 0 )
  {
    Caffe::SetDevice( deviceID );
  }

  // Load CNN Classifiers
  if( isTrainingMode )
  {
    if( clsParams.L1Files.size() == 0 )
    {
      std::cerr << "CNN model definition must be listed in classifier config file" << std::endl;
      return false;
    }

    initialClfr = new CNN( clsParams.L1Files[0], caffe::TEST );
  }
  else
  {
    if( clsParams.L1Files.size() != 2 )
    {
      std::cerr << "CNN config file contains invalid number of files" << std::endl;
      return false;
    }

    initialClfr = new CNN( clsParams.L1Files[0], caffe::TEST );
    initialClfr->CopyTrainedLayersFrom( clsParams.L1Files[1] );

    if( clsParams.L2Files.size() == 2 )
    {
      suppressionClfr = new CNN( clsParams.L2Files[0], caffe::TEST );
      suppressionClfr->CopyTrainedLayersFrom( clsParams.L2Files[1] );
    }
    else if( clsParams.L2Files.size() != 0 )
    {
      std::cerr << "CNN config file contains invalid number of files" << std::endl;
      return false;
    }
  }

  // Load Labels Vectors
  if( clsParams.L1Keys.size() != clsParams.L1SpecTypes.size() )
  {
    std::cerr << "CNN config key and type vectors not equal length" << std::endl;
    return false;
  }
  if( clsParams.L2Keys.size() != clsParams.L2SpecTypes.size() )
  {
    std::cerr << "CNN config key and type vectors not equal length" << std::endl;
    return false;
  }

  for( int i = 0; i < clsParams.L1Keys.size(); i++ )
  {
    // Declare new classifier
    IDLabel label;
    label.id = clsParams.L1Keys[i];

    // Set special conditions
    label.isBackground = ( clsParams.L1SpecTypes[i] == BACKGROUND );
    label.isSandDollar = ( clsParams.L1SpecTypes[i] == SAND_DOLLAR );
    label.isScallop = ( clsParams.L1SpecTypes[i] == ALL_SCALLOP );
    label.isWhite = ( clsParams.L1SpecTypes[i] == WHITE_SCALLOP );
    label.isBrown = ( clsParams.L1SpecTypes[i] == BROWN_SCALLOP );
    label.isBuried = ( clsParams.L1SpecTypes[i] == BURIED_SCALLOP );
    
    // Set is scallop classifier flag
    if( label.isWhite || label.isBrown || label.isBuried || label.isScallop )
    {
      isScallopDirected = true;
    }

    // Add classifier to system
    initialClfrLabels.push_back( label );    
  }

  // Load Labels Vectors
  for( int i = 0; i < clsParams.L2Keys.size(); i++ )
  {
    // Declare new classifier
    IDLabel label;
    label.id = clsParams.L2Keys[i];

    // Set special conditions
    label.isBackground = ( clsParams.L2SpecTypes[i] == BACKGROUND );
    label.isSandDollar = ( clsParams.L2SpecTypes[i] == SAND_DOLLAR );
    label.isScallop = ( clsParams.L2SpecTypes[i] == ALL_SCALLOP );
    label.isWhite = ( clsParams.L2SpecTypes[i] == WHITE_SCALLOP );
    label.isBrown = ( clsParams.L2SpecTypes[i] == BROWN_SCALLOP );
    label.isBuried = ( clsParams.L2SpecTypes[i] == BURIED_SCALLOP );
    
    // Set is scallop classifier flag
    if( label.isWhite || label.isBrown || label.isBuried || label.isScallop )
    {
      isScallopDirected = true;
    }

    // Add classifier to system
    suppressionClfrLabels.push_back( label );    
  }

  return true;
}

void CNNClassifier::classifyCandidates(
  cv::Mat image,
  CandidatePtrVector& candidates,
  CandidatePtrVector& positive )
{
  // Perform initial classification
  if( image.cols > 0 && image.rows > 0 )
  {
    // Create pointer to input blob
    Blob< float >* inputBlob = initialClfr->input_blobs()[0];

    // Get batch size and input parameters from model
    const unsigned totalCandidates = candidates.size();
    const unsigned batchSize = inputBlob->num();
    const unsigned channels = inputBlob->channels();
    const unsigned chipHeight = inputBlob->height();
    const unsigned chipWidth = inputBlob->width();

    // Training mode subroutine
    if( isTrainingMode )
    {
      return;
    }
  
    // Iterate over all candidates, extracting chips and computing propabilities
    int entry = 0;
  
    while( entry < candidates.size() )
    {
      // Formate input data
      int batchPosition = 0;
      std::vector< int > batchIndices;
  
      for( ; entry < totalCandidates && batchPosition < batchSize;
           entry++, batchPosition++ )
      {
        // Extract image chip for candidate, if possible
        cv::Mat chip = getCandidateChip( image, candidates[entry], chipWidth, chipHeight );
  
        if( chip.rows == 0 || chip.cols == 0 )
        {
          batchPosition--;
        }
        else
        {
          // Add chip to batch
          batchIndices.push_back( entry );                  
        }
      }
  
      // Process latest compiled batch, starting with resetting operating mode
      Caffe::set_mode( deviceMode );
    
      if( deviceMode == Caffe::GPU && deviceID >= 0 )
      {
        Caffe::SetDevice( deviceID );
      }
  
      // Process CNN
      initialClfr->ForwardPrefilled();
  
      // Receive output from CNN, inject back in candidate and threshold
      Blob< float >* outputBlob = initialClfr->output_blobs()[0];

      for( int i = 0; i < batchPosition; i++ )
      {
        
      }
    }
  }
  else
  {
    std::cerr << "Classifier received invalid image" << std::endl;
    return;
  }

  // Perform secondary classification
  if( suppressionClfr )
  {

  }
}

cv::Rect getCandidateBox( Candidate* cd )
{
  if( !cd )
    return cv::Rect();

  int axis = std::max( cd->major, cd->minor );

  return cv::Rect( cd->c - axis, cd->r - axis, 2 * axis, 2 * axis );
}

cv::Rect sclCandidateBox( cv::Rect r, float expansion )
{
  float pixelsToAddW = ( expansion - 1.0 ) * r.width;
  float pixelsToAddH = ( expansion - 1.0 ) * r.height;

  return cv::Rect( r.x - pixelsToAddW/2, r.y - pixelsToAddH/2,
    r.width + pixelsToAddW, r.height + pixelsToAddH ); 
}

cv::Mat CNNClassifier::getCandidateChip( cv::Mat image, Candidate* cd, int width, int height )
{
  // Get bbox in input image coords
  cv::Rect box = sclCandidateBox( getCandidateBox( cd ), CNN_EXPANSION_RATIO );

  // Extract ROI contained in box to same scale input
  cv::Mat extractedROI( box.width, box.height, image.type() );
  extractedROI.setTo( 0 );

  cv::Rect inputROI = ( box & cv::Rect( 0, 0, image.cols, image.rows ) );

  cv::Rect outputROI = cv::Rect(
    ( box.x < 0 ? -box.x : 0 ),
    ( box.y < 0 ? -box.y : 0 ),
    inputROI.width, inputROI.height );

  if( outputROI.area() > 0 && inputROI.area() > 0 )
  {
    image( inputROI ).copyTo( extractedROI( outputROI ) );
  }

  // Resize extracted ROI to desired chip size
  cv::Size size( width, height );
  cv::Mat resizedROI;
  cv::resize( extractedROI, resizedROI, size );

  return resizedROI;
}

float boxIntersection( cv::Rect r1, cv::Rect r2 )
{
  float areaInt = ( r1 & r2 ).area();

  float areaR1 = r1.area();
  float areaR2 = r2.area();

  return std::min( ( areaR1 > 0.0f ? areaInt / areaR1 : 0.0f ),
                   ( areaR2 > 0.0f ? areaInt / areaR2 : 0.0f ) );
}

void CNNClassifier::extractSamples(
  cv::Mat image,
  CandidatePtrVector& candidates,
  CandidatePtrVector& groundTruth )
{
  // Get pointer to input blob simply for chip properties
  Blob< float >* inputBlob = initialClfr->input_blobs()[0];

  // Get batch size and input parameters from model
  const unsigned totalCandidates = candidates.size();
  const unsigned channels = inputBlob->channels();
  const unsigned chipHeight = inputBlob->height();
  const unsigned chipWidth = inputBlob->width();

#ifdef CNN_CLASSIFIER_OUTPUT_GT_IMAGE
  // Local debug image output
  IplImage w = image;
  static int counter = 0;
  counter++;
  std::stringstream ss;
  ss << counter;

  if( groundTruth.size() > 0 )
    saveCandidates( &w, groundTruth, "output" + ss.str() + ".png" );
#endif

  static unsigned sampleCounter = 0;

  for( unsigned i = 0; i < totalCandidates; i++ )
  {
    cv::Rect cbox = getCandidateBox( candidates[i] );

    if( ( cbox & cv::Rect( 0, 0, image.cols, image.rows ) ).area() < 5 )
      continue;

    // Get top label for this rectangle
    bool isBackground = false;
    std::string topLabel;
    float topIntersect = 0.10f;

    for( unsigned j = 0; j < groundTruth.size(); j++ )
    {
      cv::Rect gbox = getCandidateBox( groundTruth[j] );
      float intersect = boxIntersection( cbox, gbox );

      std::string postfix;

      if( intersect > 0.90f )
      {
        postfix = "0.90";
      }
      else if( intersect > 0.80f )
      {
        postfix = "0.80";
      }
      else if( intersect > 0.70f )
      {
        postfix = "0.70";
      }
      else if( intersect > 0.50f )
      {
        postfix = "0.50";
      }
      else if( intersect > 0.30f )
      {
        postfix = "0.30";
      }
      else if( intersect >= 0.10f )
      {
        postfix = "0.10";
      }

      if( intersect > topIntersect )
      {
        topLabel = INT_2_STR( groundTruth[j]->classification ) + "_" + postfix;
        topIntersect = intersect;
      }
    }

    // Perform downsampling
    bool isBGSample = topLabel.empty();

    if( isBGSample )
    {
      if( ( (double)rand() / (double)RAND_MAX ) < trainingPercentKeep )
      {
        continue;
      }

      topLabel = "background";
    }

    // Extract chip
    cv::Mat chip = getCandidateChip( image, candidates[i], chipWidth, chipHeight );

    if( isBGSample )
    {
      float sf = 0.5 + 1.0 * ( (double)rand() / (double)RAND_MAX );

      chip = sf * chip;
    }

    // Write chip to correct file
    std::string outputLoc = outputFolder + "/" + topLabel;

    // Create output dir for sample if it doesn't exist
    createDir( outputLoc );

    // Output formatted image
    std::string outputFile = outputLoc + "/" + INT_2_STR( sampleCounter++ ) + ".png";
    imwrite( outputFile, chip );

    // Augment image and re-output
    if( !isBGSample )
    {
      float sf2 = 0.5 + 1.0 * ( (double)rand() / (double)RAND_MAX );
      float sf3 = 0.6 + 0.8 * ( (double)rand() / (double)RAND_MAX );
      float sf4 = 0.7 + 0.6 * ( (double)rand() / (double)RAND_MAX );

      cv::Mat chip2, chip3, chip4;

      cv::transpose( chip, chip2 );  
      cv::flip( chip, chip3, 1 );
      cv::flip( chip, chip4, 1 );
      cv::transpose( chip4, chip4 );

      chip2 *= sf2;
      chip3 *= sf3;
      chip4 *= sf4;

      outputFile = outputLoc + "/" + INT_2_STR( sampleCounter++ ) + ".png";
      imwrite( outputFile, chip2 );
      outputFile = outputLoc + "/" + INT_2_STR( sampleCounter++ ) + ".png";
      imwrite( outputFile, chip3 );
      outputFile = outputLoc + "/" + INT_2_STR( sampleCounter++ ) + ".png";
      imwrite( outputFile, chip4 );
    }
  }
}

}

