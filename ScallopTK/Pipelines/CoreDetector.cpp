//------------------------------------------------------------------------------
// CoreDetector.cpp
// author: Matt Dawkins
// description: Core detector pipeline of this mini toolkit
//------------------------------------------------------------------------------

#include "CoreDetector.h"

// Standard C/C++
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// OpenCV
#include <highgui.h>

// Internal Scallop Includes
#include "ScallopTK/Utilities/ConfigParsing.h"
#include "ScallopTK/Utilities/Display.h"
#include "ScallopTK/Utilities/Benchmarking.h"
#include "ScallopTK/Utilities/Threads.h"
#include "ScallopTK/Utilities/Filesystem.h"

#include "ScallopTK/ScaleDetection/ImageProperties.h"

#include "ScallopTK/ObjectProposals/HistogramFiltering.h"
#include "ScallopTK/ObjectProposals/PriorStatistics.h"
#include "ScallopTK/ObjectProposals/AdaptiveThresholding.h"
#include "ScallopTK/ObjectProposals/TemplateApproximator.h"
#include "ScallopTK/ObjectProposals/CannyPoints.h"
#include "ScallopTK/ObjectProposals/Consolidator.h"

#include "ScallopTK/EdgeDetection/WatershedEdges.h"
#include "ScallopTK/EdgeDetection/StableSearch.h"
#include "ScallopTK/EdgeDetection/ExpensiveSearch.h"

#include "ScallopTK/FeatureExtraction/ColorID.h"
#include "ScallopTK/FeatureExtraction/HoG.h"
#include "ScallopTK/FeatureExtraction/ShapeID.h"
#include "ScallopTK/FeatureExtraction/Gabor.h"

#include "ScallopTK/Classifiers/TrainingUtils.h"
#include "ScallopTK/Classifiers/AdaClassifier.h"
#include "ScallopTK/Classifiers/CNNClassifier.h"

// Number of worker threads
int THREADS;

namespace ScallopTK
{

// Variables for benchmarking tests
#ifdef ENABLE_BENCHMARKING
  const string BenchmarkingFilename = "BenchmarkingResults.dat";
  vector<double> ExecutionTimes;
  ofstream benchmarkingOutput;
#endif

// Struct to hold inputs to the single image algorithm (1 per thread is created)
struct AlgorithmArgs {
  
  // ID for this thread
  int ThreadID;

  // Input image
  cv::Mat InputImage;

  // Input filename for input image, full path, if available
  string InputFilename;

  // Input filename for image, without directory
  string InputFilenameNoDir;

  // Output filename for Scallop List or Extracted Training Data
  string ListFilename;

  // Output filename for image result if enabled
  string OutputFilename;

  // Will have the algorithm use metadata if it is available
  bool UseMetadata;

  // Output options
  bool ShowVideoDisplay;
  bool EnableListOutput;
  bool OutputMultiEntries;
  bool EnableImageOutput;

  // Search radii (used in meters if metadata is available, else pixels)
  float MinSearchRadiusMeters;
  float MaxSearchRadiusMeters;
  float MinSearchRadiusPixels;
  float MaxSearchRadiusPixels;

  // True if metadata is provided externally from a list
  bool MetadataProvided;
  float Pitch;
  float Roll;
  float Altitude;
  float FocalLength;

  // Container for color filters
  ColorClassifier *CC;

  // Container for external statistics collected so far (densities, etc)
  ThreadStatistics *Stats;

  // Container for loaded classifier system to use on this image
  ClassifierSystem *ClassifierGroup;

  // Did we last see a scallop or sand dollar cluster?
  bool ScallopMode;
  
  // Processing mode
  bool IsTrainingMode;
  
  // Training style (GUI or GT) if in training mode
  bool UseGTData;

  // GT Training keep factor
  float TrainingPercentKeep;

  // Process border interest points
  bool ProcessBorderPoints;

  // Pointer to GT input data if in training mode
  GTEntryList *GTData;
};

// Our Core Detection Algorithm - performs classification for a single image
//   inputs - shown above
//   outputs - returns NULL
void *ProcessImage( void *InputArgs ) {
    
//--------------------Get Pointers to Main Inputs---------------------

  // Read input arguments (pthread requires void* as argument type)
  AlgorithmArgs *Options = (AlgorithmArgs*) InputArgs;
  ColorClassifier *CC = Options->CC;
  ThreadStatistics *Stats = Options->Stats;

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.clear();
  startTimer();
#endif  

  // Declare input image in assorted formats for later operations
  cv::Mat inputImgMat = Options->InputImage;

  if( inputImgMat.cols == 0 || inputImgMat.rows == 0 )
  {
    throw std::runtime_error( "Invalid input image" );
  }

//----------------------Calculate Object Size-------------------------

  // Declare Image Properties reader (for metadata read, size calc, etc)
  ImageProperties inputProp;

  if( Options->UseMetadata )
  {
    // Automatically loads metadata from input file if necessary
    if( !Options->MetadataProvided )
    {
      inputProp.calculateImageProperties( Options->InputFilename, inputImgMat.cols,
        inputImgMat.rows, Options->FocalLength );
    }
    else
    {
      inputProp.calculateImageProperties( inputImgMat.cols, inputImgMat.rows,
         Options->Altitude, Options->Pitch, Options->Roll, Options->FocalLength );
    }
  
    if( !inputProp.hasMetadata() )
    {
      cerr << "ERROR: Failure to read image metadata for file ";
      cerr << Options->InputFilenameNoDir << endl;
      threadExit();
      return NULL;
    }
  }
  else
  {
    inputProp.calculateImageProperties( inputImgMat.cols, inputImgMat.rows);
  }
  
  // Get the min and max Scallop size from combined image properties and input parameters
  float minRadPixels = ( Options->UseMetadata ? Options->MinSearchRadiusMeters
    : Options->MinSearchRadiusPixels ) / inputProp.getAvgPixelSizeMeters();
  float maxRadPixels = ( Options->UseMetadata ? Options->MaxSearchRadiusMeters
    : Options->MaxSearchRadiusPixels ) / inputProp.getAvgPixelSizeMeters();

  // Threshold size scanning range
  if( maxRadPixels < 1.0 )
  {
    cerr << "WARN: Scallop scanning size range is less than 1 pixel for image ";
    cerr << Options->InputFilenameNoDir << ", skipping." << endl;
    threadExit();
    return NULL;
  }

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

//-------------------------Format Base Images--------------------------

  // Resize image to maximum size required for all operations
  //  Stats->getMaxMinRequiredRad returns the maximum required image size
  //  in terms of how many pixels the min scallop radius should be. We only
  //  resize the image if this results in a downscale.
  float resizeFactor = Stats->returnMaxMinRadRequired() / minRadPixels;

  if( resizeFactor < 1.0f ) {
    cv::Mat resizedImgMat;

    cv::resize( inputImgMat, resizedImgMat,
      cv::Size( (int)( resizeFactor*inputImgMat.cols ),
                (int)( resizeFactor*inputImgMat.rows ) ) );

    inputImgMat = resizedImgMat;
    minRadPixels = minRadPixels * resizeFactor;
    maxRadPixels = maxRadPixels * resizeFactor;
  } else {
    resizeFactor = 1.0f;
  }

  // The remaining code uses legacy OpenCV API (IplImage)
  IplImage inputImgIplWrapper = inputImgMat;
  IplImage *inputImg = &inputImgIplWrapper;

  // Create processed mask - records which pixels belong to what
  IplImage *mask = cvCreateImage( cvGetSize( inputImg ), IPL_DEPTH_8U, 1 );
  cvSet( mask, cvScalar( 255 ) );

  // Records how many detections of each classification category we have
  // within the current image
  int detections[TOTAL_DESIG];
  for( unsigned int i=0; i<TOTAL_DESIG; i++ )
  {
    detections[i] = 0;
  }

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Convert input image to other formats required for later operations
  //  - 32f = Std Floating Point, Range [0,1]
  //  - 8u = unsigned bytes, Range [0,255]
  //  - lab = CIELab color space
  //  - gs = Grayscale
  //  - rgb = sRGB (although beware OpenCV may load this as BGR in mem)
  IplImage *imgRGB32f = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_32F, inputImg->nChannels );
  IplImage *imgLab32f = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_32F, inputImg->nChannels );
  IplImage *imgGrey32f = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_32F, 1 );  
  IplImage *imgGrey8u = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_8U, 1 );
  IplImage *imgRGB8u = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_8U, 3 );

  float scalingFactor = 1 / ( pow( 2.0f, inputImg->depth ) - 1 );
  cvConvertScale( inputImg, imgRGB32f, scalingFactor );
  cvCvtColor( imgRGB32f, imgLab32f, CV_RGB2Lab );
  cvCvtColor( imgRGB32f, imgGrey32f, CV_RGB2GRAY );
  cvScale( imgGrey32f, imgGrey8u, 255. );
  cvScale( imgRGB32f, imgRGB8u, 255. );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Perform color classifications on base image
  //   Puts results in hfResults struct
  //   Contains classification results for different organisms, and sal maps
  hfResults *color = CC->performColorClassification( imgRGB32f,
    minRadPixels, maxRadPixels );
    
#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Calculate all required image gradients for later operations
  GradientChain gradients = createGradientChain( imgLab32f, imgGrey32f,
    imgGrey8u, imgRGB8u, color, minRadPixels, maxRadPixels );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif
        
//-----------------------Detect ROIs-----------------------------

  // Containers for initial interest points
  CandidateVector cdsColorBlob;
  CandidateVector cdsAdaptiveFilt;
  CandidateVector cdsTemplateAprx;
  CandidateVector cdsCannyEdge;
  
  // Perform Difference of Gaussian blob Detection on our color classifications
  if( Stats->processed > 5 )
    detectColoredBlobs( color, cdsColorBlob ); //<-- Better for large # of images
  else 
    detectSalientBlobs( color, cdsColorBlob ); //<-- Better for small # of images

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif
  
  // Perform cdsAdaptiveFilt Filtering
  performAdaptiveFiltering( color, cdsAdaptiveFilt, minRadPixels, false );
    
#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif
  
  // cdsTemplateAprx Matching Approx
  findTemplateCandidates( gradients, cdsTemplateAprx, inputProp, mask );
    
#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Stable cdsCannyEdge Edge Candidates
  findCannyCandidates( gradients, cdsCannyEdge );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

//---------------------Consolidate ROIs--------------------------

  // Containers for sorted IPs
  CandidateVector UnorderedCandidates;
  CandidateQueue OrderedCandidates;

  // Consolidate interest points
  prioritizeCandidates( cdsColorBlob, cdsAdaptiveFilt, cdsTemplateAprx, cdsCannyEdge,
    UnorderedCandidates, OrderedCandidates, Stats );
    
#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

//------------------GT Merging Procedure------------------------

  if( Options->IsTrainingMode && Options->UseGTData )
  {
    // Mark which interest points are in this image
    CandidateVector GTDetections;
    for( int i = 0; i < Options->GTData->size(); i++ )
    {
      GTEntry& Pt = (*Options->GTData)[i];
      if( Pt.Name == Options->InputFilenameNoDir )
      {
        Candidate *cd1 = ConvertGTToCandidate( Pt, resizeFactor );
        GTDetections.push_back( cd1 );

        if( true /*todo: paramatize double_import*/ )
        {
          Candidate *cd2 = ConvertGTToCandidate( Pt, resizeFactor );
          GTDetections.push_back( cd2 );           
        }
      }
    }
    
    // Remove any detected Candidates which conflict with markups
    RemoveOverlapAndMerge( UnorderedCandidates, GTDetections, Options->TrainingPercentKeep );
  }

  if( !Options->ProcessBorderPoints )
  {
    removeBorderCandidates( UnorderedCandidates, imgRGB32f );
  }

  if( Options->ShowVideoDisplay )
  {
    displayInterestPointImage( imgRGB32f, UnorderedCandidates );
  }

//--------------------Extract Features---------------------------

  // Initializes Candidate stats used for classification
  initalizeCandidateStats( UnorderedCandidates, inputImg->height, inputImg->width );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Identifies edges around each IP
  edgeSearch( gradients, color, imgLab32f, UnorderedCandidates, imgRGB32f );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Creates an unoriented gs HoG descriptor around each IP
  HoGFeatureGenerator gsHoG( imgGrey32f, minRadPixels, maxRadPixels, 0 );
  gsHoG.Generate( UnorderedCandidates );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Creates an unoriented sal HoG descriptor around each IP
  HoGFeatureGenerator salHoG( color->SaliencyMap, minRadPixels, maxRadPixels, 1 );
  salHoG.Generate( UnorderedCandidates );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Calculates size based features around each IP
  for( int i=0; i<UnorderedCandidates.size(); i++ ) {
    calculateSizeFeatures( UnorderedCandidates[i], inputProp, resizeFactor);
  }

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Calculates color based features around each IP
  createColorQuadrants( imgGrey32f, UnorderedCandidates );
  for( int i=0; i<UnorderedCandidates.size(); i++ ) {
    calculateColorFeatures( imgRGB32f, color, UnorderedCandidates[i] );
  }

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Calculates gabor based features around each IP
  calculateGaborFeatures( imgGrey32f, UnorderedCandidates );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif
  
//----------------------Classify ROIs----------------------------

  CandidateVector Interesting;
  CandidateVector LikelyObjects;
  DetectionVector Objects;

  if( Options->IsTrainingMode && !Options->UseGTData )
  {
    // If in training mode, have user enter Candidate classifications
    if( !getDesignationsFromUser( OrderedCandidates, imgRGB32f, mask, detections,
           minRadPixels, maxRadPixels, Options->InputFilenameNoDir ) )
    {
      trainingExitFlag = true;
    }
  }
  else if( Options->IsTrainingMode )
  {
    // Append all extracted features to file
    dumpCandidateFeatures( Options->ListFilename, UnorderedCandidates );
  }
  else
  {
    // Standard mode, use loaded classifiers to perform initial classifications
    for( unsigned int i=0; i<UnorderedCandidates.size(); i++ ) {
  
      // Classify ip
      Candidate *cur = UnorderedCandidates[i];
      int interest = classifyCandidate( cur, Options->ClassifierGroup );
      
      // Update Scallop List
      if( interest > 0 )
        Interesting.push_back( cur );
    }
  
    // Calculate expensive edges around each interesting point
    expensiveEdgeSearch( gradients, color, imgLab32f, imgRGB32f, Interesting );
  
    // Perform cleanup by removing interest points which are part of another interest point
    removeInsidePoints( Interesting, LikelyObjects );
  
    // Interpolate correct object categories
    Objects = interpolateResults( LikelyObjects, Options->ClassifierGroup, Options->InputFilename );

    // Display Detections
    if( Options->ShowVideoDisplay ) {
      getDisplayLock();
      displayResultsImage( imgRGB32f, Objects, Options->InputFilenameNoDir );
      unlockDisplay();
    }
  }

//-----------------------Update Stats----------------------------

  // Update Detection variables and mask
  if( !Options->IsTrainingMode && Options->ClassifierGroup->IsScallopDirected )
  {
    for( unsigned int i=0; i<Objects.size(); i++ ) {
      Detection *cur = Objects[i];
      if( cur->IsBrownScallop ) {
        detections[SCALLOP_BROWN]++;
        updateMask( mask, cur->r, cur->c, cur->angle, cur->major, cur->minor, SCALLOP_BROWN );
      } else if( cur->IsWhiteScallop ) {
        detections[SCALLOP_WHITE]++;
        updateMask( mask, cur->r, cur->c, cur->angle, cur->major, cur->minor, SCALLOP_WHITE );
      } else if( cur->IsBuriedScallop ) {
        detections[SCALLOP_BURIED]++;
        updateMaskRing( mask, cur->r, cur->c, cur->angle,
          cur->major*0.8, cur->minor*0.8, cur->major, SCALLOP_BROWN );
      }
    }
  }
  else
  {
    // Quick hack: If we're not trying to detect scallops, reuse scallop histogram for better ip detections
    for( unsigned int i=0; i<Objects.size(); i++ ) {
      Detection *cur = Objects[i];
      updateMask( mask, cur->r, cur->c, cur->angle, cur->major, cur->minor, SCALLOP_BROWN );
    }
  }

  // If in training mode use designations to distinguish
  if( Options->IsTrainingMode )
  {
    for( unsigned int i=0; i<UnorderedCandidates.size(); i++ ) {
      Candidate *cur = UnorderedCandidates[i];
      int id = cur->classification;
      if( id == 18028 || id == 18034 || id == 2 )
      {
        detections[SCALLOP_WHITE]++;
        updateMask( mask, cur->r, cur->c, cur->angle, cur->major, cur->minor, SCALLOP_WHITE );
      }
      else if( id >= 18004 && id <= 18035 || id == 1 )
      {
        detections[SCALLOP_BROWN]++;
        updateMask( mask, cur->r, cur->c, cur->angle, cur->major, cur->minor, SCALLOP_BROWN );
      }
      else if( id == 3 )
      {
        detections[SCALLOP_BURIED]++;
        updateMaskRing( mask, cur->r, cur->c, cur->angle,
          cur->major*0.8, cur->minor*0.8, cur->major, SCALLOP_BROWN );
      }
    }
  }

  // Update color classifiers from mask and detections matrix
  CC->Update( imgRGB32f, mask, detections );

  // Update statistics
  Stats->Update( detections, inputProp.getImgHeightMeters()*inputProp.getImgWidthMeters() );

  // Output results to file(s)
  if( Options->EnableImageOutput )
  {
    saveScallops( imgRGB32f, Objects, Options->OutputFilename + ".detections.jpg" );
  }
  if( Options->EnableListOutput && !Options->IsTrainingMode )
  {
    if( !appendInfoToFile( Objects, Options->ListFilename, Options->InputFilenameNoDir, resizeFactor ) )
    {
      cerr << "CRITICAL ERROR: Could not write to output list!" << endl;
    }
  }

//-------------------------Clean Up------------------------------
  
  // Deallocate memory used by thread
  deallocateCandidates( UnorderedCandidates );
  deallocateDetections( Objects );
  deallocateGradientChain( gradients );
  hfDeallocResults( color );
  cvReleaseImage( &inputImg );
  cvReleaseImage( &imgRGB32f );
  cvReleaseImage( &imgRGB8u );
  cvReleaseImage( &imgGrey32f );
  cvReleaseImage( &imgLab32f );
  cvReleaseImage( &imgGrey8u );
  cvReleaseImage( &mask );
  
  // Update thread status
  markThreadAsFinished( Options->ThreadID );
  threadExit();
  return NULL; 
}

//--------------File system manager / algorithm caller------------------

int runCoreDetector( const SystemParameters& settings )
{
  // Retrieve some contents from input
  string inputDir = settings.InputDirectory;
  string inputFile = settings.InputFilename;
  string outputDir = settings.OutputDirectory;
  string outputFile = settings.OutputFilename;

  // Compile a vector of all files to process with related info
  vector<string> inputFilenames; // filenames (full or relative path)
  vector<string> inputClassifiers; // corresponding classifier keys
  vector<float> inputPitch; // input pitches, if used
  vector<float> inputAltitudes; // input altitudes, if used
  vector<float> inputRoll; // input rolls, if used
  vector<string> subdirsToCreate; // subdirs we may be going to create
  vector<string> outputFilenames; // corresponding output filenames if enabled

  // GT file contents if required
  string GTfilename = inputDir + inputFile;
  GTEntryList* GTs = NULL;

  // Process directory mode
  if( settings.IsInputDirectory )
  {
    // Get a list of all files and sub directories in the input dir
    ListAllFiles( inputDir, inputFilenames, subdirsToCreate );
    
    // Remove files that don't have an image extension (jpg/tif)
    cullNonImages( inputFilenames );
    
    // Initialize classifier array
    inputClassifiers.resize( inputFilenames.size(), settings.ClassifierToUse );
  }
  // If we're in process list mode
  else if( !settings.IsTrainingMode )
  {
    // Read input list
    string list_fn = inputDir + inputFile;
    ifstream input( list_fn.c_str() );

    // Check to make sure list is open
    if( !input )
    {
      cerr << endl << "CRITICAL ERROR: " << "Unable to open input list!" << endl;
      return 0;
    }

    // Iterate through list
    while( !input.eof() )
    {
      string input_fn, classifier_key;
      float alt, pitch, roll;

      // Metadata is in the input files
      if( settings.IsMetadataInImage )
      {
        input >> input_fn >> classifier_key;
        RemoveSpaces( input_fn );
        RemoveSpaces( classifier_key );
        if( input_fn.size() > 0 && classifier_key.size() > 0 )
        {
          inputFilenames.push_back( input_fn );
          inputClassifiers.push_back( classifier_key );
        }
      }
      // Metadata is in the list
      else
      {
        input >> input_fn >> alt >> pitch >> roll >> classifier_key;
        RemoveSpaces( input_fn );
        RemoveSpaces( classifier_key );
        if( input_fn.size() > 0 && classifier_key.size() > 0 )
        {
          inputFilenames.push_back( input_fn );
          inputClassifiers.push_back( classifier_key );
          inputAltitudes.push_back( alt );
          inputRoll.push_back( roll );
          inputPitch.push_back( pitch );
        }
      }
    }
    input.close();

    // Set input dir to 0 because the list should have the full or relative path
    inputDir = "";
  }
  // We're in list training mode
  else if( settings.IsTrainingMode && !settings.IsInputDirectory )
  {
    // Read input list
    string list_fn = inputDir + inputFile;
    ifstream input( list_fn.c_str() );

    // Check to make sure list is open
    if( !input )
    {
      cerr << endl << "CRITICAL ERROR: " << "Unable to open input list!" << endl;
      return 0;
    }

    // Read location of GT annotations
    char buffer[2048];
    input.getline(buffer,2048);
    GTfilename = buffer;
    RemoveSpaces( GTfilename );

    // Iterate through list
    while( !input.eof() )
    {
      string input_fn, classifier_key;
      float alt, pitch, roll;

      // Metadata is in the input files
      if( settings.IsMetadataInImage )
      {
        input >> input_fn;
        RemoveSpaces( input_fn );
        if( input_fn.size() > 0  )
        {
          inputFilenames.push_back( input_fn );
          inputClassifiers.push_back( classifier_key );
        }
      }
      // Metadata is in the list
      else
      {
        input >> input_fn >> alt >> pitch >> roll;
        RemoveSpaces( input_fn );
        if( input_fn.size() > 0 )
        {
          inputFilenames.push_back( input_fn );
          inputClassifiers.push_back( classifier_key );
          inputAltitudes.push_back( alt );
          inputRoll.push_back( roll );
          inputPitch.push_back( pitch );
        }
      }
    }
    input.close();

    // Set input dir to 0 because the list should have the full or relative path
    inputDir = "";
  }

  // Read GTs file if necessary
  if( settings.IsTrainingMode && settings.UseFileForTraining )
  {
    // srand for random adjustments
    srand(time(NULL));

    // Load csv file
    GTs = new GTEntryList;
    cout << GTfilename << endl;
    ParseGTFile( GTfilename, *GTs );
  }

  // Check to make sure image list is not empty
  if( inputFilenames.size() == 0 )
  {
    cerr << "\nERROR: Input invalid or contains no valid images." << std::endl;
    return 0;
  }

  // Copy the folder structure in the input dir to the output dir
  if( settings.OutputDetectionImages && !settings.IsTrainingMode )
  {
    CopyDirTree( subdirsToCreate, inputDir, outputDir );
  }

  // Format the output name vector for each input image
  FormatOutputNames( inputFilenames, outputFilenames, inputDir, outputDir );

  // Set global thread count
  THREADS = settings.NumThreads;

  // Create output list filename
  string listFilename = outputDir + outputFile;

  // Check to make sure we can open the output file (and flush contents)
  if( settings.OutputList ) {
    ofstream fout( listFilename.c_str() );
    if( !fout.is_open() ) {
      cout << "ERROR: Could not open output list for writing!" << std::endl;
      return false;
    }
    fout.close();
  }

#ifdef ENABLE_BENCHMARKING
  // Initialize Timing Statistics
  initializeTimer();
  benchmarkingOutput.open( BenchmarkingFilename.c_str() );

  if( !benchmarkingOutput.is_open() ) {
    cout << "ERROR: Could not write to benchmarking file!" << std::endl;
    return false;
  }
#endif

  // Storage map for loaded classifier styles
  map< string, ClassifierSystem* > classifiers;

  // Preload all required classifiers just in case there's a mistake
  // in a config file (so it doesn't die midstream)
  if( !settings.IsTrainingMode )
  {
    cout << "Loading Classifier Systems... ";
    for( int i = 0; i < inputClassifiers.size(); i++ )
    {
      // If classifier key does not exist in our list
      if( classifiers.find( inputClassifiers[i] ) == classifiers.end() )
      {
        // Load classifier config
        ClassifierParameters cparams;
        if( !ParseClassifierConfig( inputClassifiers[i], settings, cparams ) )
        {
          return 0;
        }

        // Load classifier system based on config settings
        ClassifierSystem* LoadedSystem = loadClassifiers( settings, cparams );

        if( LoadedSystem == NULL )
        {
          cout << "ERROR: Unabled to load classifier " << inputClassifiers[i] << endl;
          return 0;
        }
      
        classifiers[ inputClassifiers[i] ] = LoadedSystem;      
      }
    }
    cout << "FINISHED" << endl;
  }

  // Load Statistics/Color filters
  cout << "Loading Colour Filters... ";
  AlgorithmArgs *inputArgs = new AlgorithmArgs[THREADS];

  for( int i=0; i < THREADS; i++ )
  {
    inputArgs[i].CC = new ColorClassifier;
    inputArgs[i].Stats = new ThreadStatistics;
    if( !inputArgs[i].CC->loadFilters( settings.RootColorDIR, "_32f_rgb_v1.cfilt" ) ) {
      cerr << "ERROR: Could not load colour filters!" << std::endl;
      return 0;
    }
  }
  cout << "FINISHED" << std::endl;

  // Configure algorithm input based on settings
  for( int i=0; i<THREADS; i++ )
  {
    // Set thread output options
    inputArgs[i].IsTrainingMode = settings.IsTrainingMode;
    inputArgs[i].UseGTData = settings.UseFileForTraining;
    inputArgs[i].GTData = GTs;
    inputArgs[i].EnableImageOutput = settings.OutputDetectionImages;
    inputArgs[i].EnableListOutput = settings.OutputList;
    inputArgs[i].OutputMultiEntries = settings.OutputDuplicateClass;
    inputArgs[i].ShowVideoDisplay = settings.EnableOutputDisplay;
    inputArgs[i].ScallopMode = true;
    inputArgs[i].MetadataProvided = !settings.IsMetadataInImage && !settings.IsInputDirectory;
    inputArgs[i].ListFilename = listFilename;
  }

  // Initiate display window for output
  if( settings.EnableOutputDisplay )
  {
    initOutputDisplay();
  }

  // Initialize training mode if in gui mode
  if( settings.IsTrainingMode && !settings.UseFileForTraining ) {
    if( !initializeTrainingMode( outputDir, outputFile ) ) {
      cerr << "ERROR: Could not initiate training mode!" << std::endl;
    }
  }

  // Cycle through all input files
  cout << endl << "Processing Files: " << endl << endl;
  cout << "Directory: " << inputDir << endl << endl;

  // For every file...
  for( unsigned int i=0; i<inputFilenames.size(); i++ ) {

    // Set classifier for entry
    if( !settings.IsTrainingMode )
    {
      inputArgs[0].ClassifierGroup = classifiers[ inputClassifiers[i] ];
    }

    // Always set the focal length
    inputArgs[0].FocalLength = settings.FocalLength;
    inputArgs[0].TrainingPercentKeep = settings.TrainingPercentKeep;
    inputArgs[0].ProcessBorderPoints = settings.LookAtBorderPoints;

    // Set file/dir arguments
    string Dir, filenameNoDir;
    SplitPathAndFile( inputFilenames[i], Dir, filenameNoDir );
    cout << filenameNoDir << "..." << endl;
    inputArgs[0].InputFilename = inputFilenames[i];
    inputArgs[0].OutputFilename = outputFilenames[i];
    inputArgs[0].InputFilenameNoDir = filenameNoDir;

    // Set metadata if required
    if( !settings.IsMetadataInImage && !settings.IsInputDirectory )
    {
      inputArgs[0].Altitude = inputAltitudes[i];
      inputArgs[0].Pitch = inputPitch[i];
      inputArgs[0].Roll = inputRoll[i];
    }

    // Load image from file
    cv::Mat image;
    image = imread( inputFilenames[i], CV_LOAD_IMAGE_COLOR );
    inputArgs[0].InputImage = image;

    // Execute processing
    ProcessImage( inputArgs );

#ifdef ENABLE_BENCHMARKING
    // Output benchmarking results to file
    for( unsigned int i=0; i<ExecutionTimes.size(); i++ )
      benchmarkingOutput << ExecutionTimes[i] << " ";
    benchmarkingOutput << endl;
#endif

    // Checks if user entered EXIT command in training mode
    if( settings.IsTrainingMode && trainingExitFlag )
    {
      break;
    }
  }

  // Deallocate algorithm inputs
  for( int i=0; i < THREADS; i++ ) {
    delete inputArgs[i].Stats;
    delete inputArgs[i].CC;
  }
  delete[] inputArgs;

  // Deallocate loaded classifier systems
  map< string, ClassifierSystem* >::iterator p = classifiers.begin();
  while( p != classifiers.end() )
  {
    delete p->second;
    p++;
  }

  // Remove output display window
  if( settings.EnableOutputDisplay )
  {
    killOuputDisplay();
  }

#ifdef ENABLE_BENCHMARKING
  // Close benchmarking output file
  benchmarkingOutput.close();
#endif

  // Close gui-training mode
  if( settings.IsTrainingMode && !settings.UseFileForTraining )
  {
    exitTrainingMode();
  }

  // Deallocate GT info if in training mode
  if( GTs )
  {
    delete GTs;
  }

  return 0;
}

// Streaming class definition, for use by external libraries
class CoreDetector::Priv
{
public:

  explicit Priv( const SystemParameters& settings );
  ~Priv();

  ClassifierSystem* classifier;
  AlgorithmArgs *inputArgs;
  SystemParameters settings;
  unsigned counter;
};

CoreDetector::Priv::Priv( const SystemParameters& sets )
{
  counter = 0;

  // Retrieve some contents from input
  settings = sets;
  string outputDir = settings.OutputDirectory;
  string outputFile = settings.OutputFilename;
  string listFilename = outputDir + outputFile;
  THREADS = settings.NumThreads;

  // Check to make sure we can open the output file (and flush contents)
  if( settings.OutputList ) {
    ofstream fout( listFilename.c_str() );
    if( !fout.is_open() ) {
      throw std::runtime_error( "Could not open output list for writing" );
    }
    fout.close();
  }

#ifdef ENABLE_BENCHMARKING
  // Initialize Timing Statistics
  initializeTimer();
  benchmarkingOutput.open( BenchmarkingFilename.c_str() );

  if( !benchmarkingOutput.is_open() ) {
    throw std::runtime_error( "Could not write to benchmarking file" );
  }
#endif

  // Load classifier config
  cout << "Loading Classifier System";

  ClassifierParameters cparams;

  if( !ParseClassifierConfig( settings.ClassifierToUse, settings, cparams ) ) {
    throw std::runtime_error( "Unabled to read config for "+ settings.ClassifierToUse );
  }

  // Load classifier system based on config settings
  classifier = loadClassifiers( settings, cparams );

  if( classifier == NULL ) {
    throw std::runtime_error( "Unabled to load classifier " + settings.ClassifierToUse );
  }

  // Load Statistics/Color filters
  cout << "Loading Colour Filters... ";
  inputArgs = new AlgorithmArgs[THREADS];

  for( int i=0; i < THREADS; i++ )
  {
    inputArgs[i].CC = new ColorClassifier;
    inputArgs[i].Stats = new ThreadStatistics;

    if( !inputArgs[i].CC->loadFilters( settings.RootColorDIR, DEFAULT_COLORBANK_EXT ) ) {
      throw std::runtime_error( "Could not load colour filters" );
    }
  }

  cout << "FINISHED" << std::endl;

  // Configure algorithm input based on settings
  for( int i=0; i<THREADS; i++ )
  {
    // Set thread output options
    inputArgs[i].IsTrainingMode = settings.IsTrainingMode;
    inputArgs[i].UseGTData = settings.UseFileForTraining;
    inputArgs[i].GTData = NULL;
    inputArgs[i].EnableImageOutput = settings.OutputDetectionImages;
    inputArgs[i].EnableListOutput = settings.OutputList;
    inputArgs[i].OutputMultiEntries = settings.OutputDuplicateClass;
    inputArgs[i].ShowVideoDisplay = settings.EnableOutputDisplay;
    inputArgs[i].ScallopMode = true;
    inputArgs[i].MetadataProvided = !settings.IsMetadataInImage && !settings.IsInputDirectory;
    inputArgs[i].ListFilename = listFilename;
    inputArgs[i].FocalLength = settings.FocalLength;
  }

  // Initiate display window for output
  if( settings.EnableOutputDisplay )
  {
    initOutputDisplay();
  }

  // Cycle through all input files
  cout << endl << "Ready to Process Files" << endl;
}

CoreDetector::Priv::~Priv()
{
  // Deallocate algorithm inputs
  for( int i=0; i < THREADS; i++ ) {
    delete inputArgs[i].Stats;
    delete inputArgs[i].CC;
  }

  delete[] inputArgs;

  // Deallocate loaded classifier systems
  if( classifier )
  {
    delete classifier;
  }

  // Remove output display window
  if( settings.EnableOutputDisplay )
  {
    killOuputDisplay();
  }

#ifdef ENABLE_BENCHMARKING
  // Close benchmarking output file
  benchmarkingOutput.close();
#endif
}

CoreDetector::CoreDetector( const std::string& configFile )
{
  SystemParameters settings;

  if( !ParseSystemConfig( settings, configFile ) )
  {
    throw std::runtime_error( "Unable to read system parameters file" );
  }

  data = new Priv( settings );
}

CoreDetector::CoreDetector( const SystemParameters& settings )
{
  data = new Priv( settings );
}

CoreDetector::~CoreDetector()
{
  if( data )
  {
    delete data;
  }
}

std::vector< Detection >
CoreDetector::processFrame( const cv::Mat& image,
 float pitch, float roll, float altitude )
{
  // Always set the focal length
  data->counter++;
  std::string frameID = "streaming_frame" + INT_2_STR( data->counter );

  data->inputArgs[0].InputImage = image;
  data->inputArgs[0].InputFilename = frameID;
  data->inputArgs[0].OutputFilename = frameID;
  data->inputArgs[0].InputFilenameNoDir = frameID;

  if( pitch != 0.0f || roll != 0.0f || altitude != 0.0f )
  {
    data->inputArgs[0].MetadataProvided = true;
    data->inputArgs[0].Pitch = pitch;
    data->inputArgs[0].Roll = roll;
    data->inputArgs[0].Altitude = altitude;
  }
  else
  {
    data->inputArgs[0].MetadataProvided = false;
  }

  // Execute processing
  ProcessImage( data->inputArgs );

#ifdef ENABLE_BENCHMARKING
  // Output benchmarking results to file
  for( unsigned int i=0; i<ExecutionTimes.size(); i++ )
    benchmarkingOutput << ExecutionTimes[i] << " ";
  benchmarkingOutput << endl;
#endif
}

std::vector< Detection >
CoreDetector::processFrame( std::string filename,
 float pitch, float roll, float altitude )
{
  cv::Mat image;
  image = imread( filename, CV_LOAD_IMAGE_COLOR );
  processFrame( image, pitch, roll, altitude );
}

}
