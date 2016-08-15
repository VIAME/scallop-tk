
#include "CNNClassifier.h"

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
        cv::Mat chip = getCandidateChip( image, candidates[entry] );
  
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
  if( !isTrainingMode && suppressionClfr )
  {

  }  
}

cv::Mat CNNClassifier::getCandidateChip( cv::Mat image, Candidate* cd )
{
  
}
