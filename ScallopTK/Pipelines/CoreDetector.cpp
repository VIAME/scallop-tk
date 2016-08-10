
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
#include "highgui.h"

// Internal Scallop Includes
#include "ScallopTK/Utilities/ConfigParsing.h"
#include "ScallopTK/Utilities/Display.h"
#include "ScallopTK/Utilities/Benchmarking.h"
#include "ScallopTK/Utilities/Threads.h"
#include "ScallopTK/Utilities/Filesystem.h"

#include "ScallopTK/ScaleDetection/ImageProperties.h"

#include "ScallopTK/ObjectProposals/Consolidator.h"
#include "ScallopTK/ObjectProposals/HistogramFiltering.h"
#include "ScallopTK/ObjectProposals/PriorStatistics.h"
#include "ScallopTK/ObjectProposals/AdaptiveThresholding.h"
#include "ScallopTK/ObjectProposals/TemplateApproximator.h"
#include "ScallopTK/ObjectProposals/CannyPoints.h"

#include "ScallopTK/EdgeDetection/WatershedEdges.h"
#include "ScallopTK/EdgeDetection/StableSearch.h"
#include "ScallopTK/EdgeDetection/ExpensiveSearch.h"

#include "ScallopTK/FeatureExtraction/ColorID.h"
#include "ScallopTK/FeatureExtraction/HoG.h"
#include "ScallopTK/FeatureExtraction/ShapeID.h"
#include "ScallopTK/FeatureExtraction/Gabor.h"

#include "ScallopTK/Classifiers/TrainingUtils.h"
#include "ScallopTK/Classifiers/AdaClassifier.h"

// Number of worker threads
int THREADS;

namespace ScallopTK
{

// Variables for benchmarking tests
#ifdef ENABLE_BENCHMARKING
  const string BenchmarkingFilename = "BenchmarkingResults.dat";
  vector<double> ExecutionTimes;
  ofstream BenchmarkingOutput;
#endif

// Struct to hold inputs to the single image algorithm (1 per thread is created)
struct AlgorithmArgs {
  
  // ID for this thread
  int ThreadID;

  // input filename for input image, full path
  string InputFilename;

  // input filename for image, without directory
  string InputFilenameNoDir;

  // Output filename for Scallop List or Extracted Training Data
  string ListFilename;
  
  // Output filename for image result if enabled
  string OutputFilename;

  // Output options
  bool ShowVideoDisplay;
  bool EnableListOutput;
  bool OutputMultiEntries;
  bool EnableImageOutput;

  // True if metadata is provided externally from the image file
  bool MetadataProvided;
  float Pitch;
  float Roll;
  float Altitude;
  float FocalLength;

  // Search radii (in meters if metadata is available, else pixels)
  float MinSearchRadius;
  float MaxSearchRadius;

  // container for color filters
  ColorClassifier *CC;

  // container for external statistics collected so far (densities, etc)
  ThreadStatistics *Stats;

  // container for loaded classifier system to use on this image
  ClassifierSystem *ClassifierGroup;

  // Did we last see a scallop or sand dollar cluster?
  bool ScallopMode;
  
  // processing mode
  bool IsTrainingMode;
  
  // training style (GUI or GT) if in training mode
  bool UseGTData;

  // GT Training keep factor
  float TrainingPercentKeep;

  // Process border interest points
  bool ProcessBorderPoints;

  // pointer to mip input data if in training mode
  GTEntryList *GTData;
};

// Our Core Detection Algorithm - performs classification for a single image
//   inputs - shown above
//   outputs - returns NULL
void *ProcessImage( void *InputArgs ) {
    
//---------------------------Load Image----------------------------

  // Read input (pthread requires void* as argument type)
  AlgorithmArgs *Options = (AlgorithmArgs*) InputArgs;
  ColorClassifier *CC = Options->CC;
  ThreadStatistics *Stats = Options->Stats;

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.clear();
  startTimer();
#endif  

  // Retrieve image type from extension (TIFF, JPEG, etc.)
  int imageType = getImageType( Options->InputFilename );

  // Load input image from file
  IplImage *inputImg;
  if( imageType != UNKNOWN ) {
    if( ( inputImg = cvLoadImage(Options->InputFilename.c_str(),
            CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR)) == 0 ) {
      cerr << "ERROR: Unable to load file.\n";
      threadExit();
      return NULL; 
    }
  } else {
    cerr << "ERROR: Unsupported file type.\n";
    threadExit();
    return NULL; 
  }

  // Confirm input has 3 channels
  if( inputImg->nChannels != 3 )
  {
    cerr << "ERROR: Unsupported file type.\n";
    cvReleaseImage( &inputImg );
    threadExit();
    return NULL; 
  }

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

//----------------------Calculate Scallop Size------------------------

  // Declare Image Properties reader (for metadata read, scallop size calc, etc)
#ifdef AUTO_READ_METADATA

  // Automatically loads metadata from input file if necessary
  ImageProperties inputProp;
  
  if( !Options->MetadataProvided )
  {
    inputProp.calculateImageProperties( Options->InputFilename, inputImg->width, inputImg->height, Options->FocalLength );
  }
  else
  {
    inputProp.calculateImageProperties( inputImg->width, inputImg->height,
       Options->Altitude, Options->Pitch, Options->Roll, Options->FocalLength );
  }

  if( !inputProp.hasMetadata() )
  {
    cerr << "ERROR: Failure to read image metadata for file ";
    cerr << Options->InputFilenameNoDir << endl;
    cvReleaseImage( &inputImg );
    threadExit();
    return NULL;
  }
#else
  // Queries user for min/max radii to scan for
  ImageProperties inputProp( inputImg->width, inputImg->height );
#endif
  
  // Get the min and max Scallop size from combined image properties and input parameters
  float minRadPixels = Options->MinSearchRadius / inputProp.getAvgPixelSizeMeters();
  float maxRadPixels = Options->MaxSearchRadius / inputProp.getAvgPixelSizeMeters();

  // Threshold size scanning range
  if( maxRadPixels < 1.0 )
  {
    cerr << "WARN: Scallop scanning size range is less than 1 pixel for image ";
    cerr << Options->InputFilenameNoDir << endl;
    cvReleaseImage( &inputImg );
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
    IplImage *temp = cvCreateImage( cvSize((int)(resizeFactor*inputImg->width),(int)(resizeFactor*inputImg->height)),
                                        inputImg->depth, inputImg->nChannels );
    cvResize(inputImg, temp, CV_INTER_LINEAR);
    cvReleaseImage(&inputImg);
    inputImg = temp;
    minRadPixels = minRadPixels * resizeFactor;
    maxRadPixels = maxRadPixels * resizeFactor;
  } else {
    resizeFactor = 1.0f;
  }

  // Create processed mask - records which pixels belong to what
  IplImage *mask = cvCreateImage( cvGetSize( inputImg ), IPL_DEPTH_8U, 1 );
  cvSet( mask, cvScalar(255) );

  // Records how many Detections of each classification category we have
  int Detections[TOTAL_DESIG];
  for( unsigned int i=0; i<TOTAL_DESIG; i++ )
  {
    Detections[i] = 0;
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
  IplImage *img_rgb_32f = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_32F, inputImg->nChannels );
  IplImage *ImgLab32f = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_32F, inputImg->nChannels );
  IplImage *img_gs_32f = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_32F, 1 );  
  IplImage *img_gs_8u = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_8U, 1 );
  IplImage *img_rgb_8u = cvCreateImage( cvGetSize(inputImg), IPL_DEPTH_8U, 3 );
  float scalingFactor = 1 / (pow(2.0f, inputImg->depth)-1);
  cvConvertScale(inputImg, img_rgb_32f, scalingFactor );
  cvCvtColor(img_rgb_32f, ImgLab32f, CV_RGB2Lab );
  cvCvtColor(img_rgb_32f, img_gs_32f, CV_RGB2GRAY );
  cvScale( img_gs_32f, img_gs_8u, 255. );
  cvScale( img_rgb_32f, img_rgb_8u, 255. );
  
#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Perform color classifications on base image
  //   Puts results in hfResults struct
  //   Contains classification results for different organisms, and sal maps
  hfResults *color = CC->performColorClassification( img_rgb_32f, minRadPixels, maxRadPixels );
    
#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Calculate all required image gradients for later operations
  GradientChain Gradients = createGradientChain( ImgLab32f, img_gs_32f, img_gs_8u, img_rgb_8u, color, minRadPixels, maxRadPixels );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif
        
//-----------------------Detect ROIs-----------------------------

  // Containers for initial interest points
  CandidateVector ColorBlob;
  CandidateVector Adaptive;
  CandidateVector Template;
  CandidateVector Canny;
  
  // Perform Difference of Gaussian blob Detection on our color classifications
  if( Stats->processed > 5 )
    detectColoredBlobs( color, ColorBlob ); //<-- Better for large # of images
  else 
    detectSalientBlobs( color, ColorBlob ); //<-- Better for small # of images

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif
  
  // Perform Adaptive Filtering
  performAdaptiveFiltering( color, Adaptive, minRadPixels, false );
    
#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif
  
  // Template Matching Approx
  findTemplateCandidates( Gradients, Template, inputProp, mask );
    
#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Stable Canny Edge Candidates
  findCannyCandidates( Gradients, Canny );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

//---------------------Consolidate ROIs--------------------------

  // Containers for sorted IPs
  CandidateVector UnorderedCandidates;
  CandidateQueue OrderedCandidates;

  // Consolidate interest points
  prioritizeCandidates( ColorBlob, Adaptive, Template, Canny,
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
    RemoveBorderCandidates( UnorderedCandidates, img_rgb_32f );
  }

  /*if( Options.ShowVideoDisplay )
  {
    //I took this out to speed things and not complicate the display- smg 11/5/11
    displayInterestPointImage( img_rgb_32f, UnorderedCandidates );
  }*/

//--------------------Extract Features---------------------------

  // Initializes Candidate stats used for classification
  initalizeCandidateStats( UnorderedCandidates, inputImg->height, inputImg->width );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Identifies edges around each IP
  edgeSearch( Gradients, color, ImgLab32f, UnorderedCandidates, img_rgb_32f );

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Creates an unoriented gs HoG descriptor around each IP
  HoGFeatureGenerator gsHoG( img_gs_32f, minRadPixels, maxRadPixels, 0 );
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
  createColorQuadrants( img_gs_32f, UnorderedCandidates );
  for( int i=0; i<UnorderedCandidates.size(); i++ ) {
    calculateColorFeatures( img_rgb_32f, color, UnorderedCandidates[i] );
  }

#ifdef ENABLE_BENCHMARKING
  ExecutionTimes.push_back( getTimeSinceLastCall() );
#endif

  // Calculates gabor based features around each IP
  calculateGaborFeatures( img_gs_32f, UnorderedCandidates );

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
    if( !getDesignationsFromUser( OrderedCandidates, img_rgb_32f, mask, Detections,
           minRadPixels, maxRadPixels, Options->InputFilenameNoDir ) )
    {
      training_exit_flag = true;
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
    expensiveEdgeSearch( Gradients, color, ImgLab32f, img_rgb_32f, Interesting );
  
    // Perform cleanup by removing interest points which are part of another interest point
    removeInsidePoints( Interesting, LikelyObjects );
  
    // Interpolate correct object categories
    Objects = interpolateResults( LikelyObjects, Options->ClassifierGroup, Options->InputFilename );

    // Display Detections
    if( Options->ShowVideoDisplay ) {
      getDisplayLock();
      displayResultsImage( img_rgb_32f, Objects, Options->InputFilenameNoDir );
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
        Detections[SCALLOP_BROWN]++;
        updateMask( mask, cur->r, cur->c, cur->angle, cur->major, cur->minor, SCALLOP_BROWN );
      } else if( cur->IsWhiteScallop ) {
        Detections[SCALLOP_WHITE]++;
        updateMask( mask, cur->r, cur->c, cur->angle, cur->major, cur->minor, SCALLOP_WHITE );
      } else if( cur->IsBuriedScallop ) {
        Detections[SCALLOP_BURIED]++;
        updateMaskRing( mask, cur->r, cur->c, cur->angle, cur->major*0.8, cur->minor*0.8, cur->major, SCALLOP_BROWN );
      }
    }
  }
  else
  {
    // Quick hack: If we're not trying to detect scallops, reuse scallop histogram for better ip Detections
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
        Detections[SCALLOP_WHITE]++;
        updateMask( mask, cur->r, cur->c, cur->angle, cur->major, cur->minor, SCALLOP_WHITE );
      }
      else if( id >= 18004 && id <= 18035 || id == 1 )
      {
        Detections[SCALLOP_BROWN]++;
        updateMask( mask, cur->r, cur->c, cur->angle, cur->major, cur->minor, SCALLOP_BROWN );
      }
      else if( id == 3 )
      {
        Detections[SCALLOP_BURIED]++;
        updateMaskRing( mask, cur->r, cur->c, cur->angle, cur->major*0.8, cur->minor*0.8, cur->major, SCALLOP_BROWN );
      }
    }
  }

  // Update color classifiers from mask and Detections matrix
  CC->Update( img_rgb_32f, mask, Detections );

  // Update statistics
  Stats->Update( Detections, inputProp.getImgHeightMeters()*inputProp.getImgWidthMeters() );

  // Output results to file(s)
  if( Options->EnableImageOutput )
  {
    saveScallops( img_rgb_32f, Objects, Options->OutputFilename + ".Detections.jpg" );
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
  deallocateGradientChain( Gradients );
  hfDeallocResults( color );
  cvReleaseImage(&inputImg);
  cvReleaseImage(&img_rgb_32f);
  cvReleaseImage(&img_rgb_8u);
  cvReleaseImage(&img_gs_32f);
  cvReleaseImage(&ImgLab32f);
  cvReleaseImage(&img_gs_8u);
  cvReleaseImage(&mask);
  
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
    CullNonImages( inputFilenames );
    
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
    cerr << "\nERROR: Input invalid or contains no valid images.\n";
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
  string ListFilename = outputDir + outputFile;

  // Check to make sure we can open the output file (and flush contents)
  if( settings.OutputList ) {
    ofstream fout( ListFilename.c_str() );
    if( !fout.is_open() ) {
      cout << "ERROR: Could not open output list for writing!\n";
      return false;
    }
    fout.close();
  }

#ifdef ENABLE_BENCHMARKING
  // Initialize Timing Statistics
  initializeTimer();
  BenchmarkingOutput.open( BenchmarkingFilename.c_str() );
  if( !BenchmarkingOutput.is_open() ) {
    cout << "ERROR: Could not write to benchmarking file!\n";
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
      cerr << "ERROR: Could not load colour filters!\n";
      return 0;
    }
  }
  cout << "FINISHED\n";

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
    inputArgs[i].ListFilename = ListFilename;
  }

  // Initiate display window for output
  if( settings.EnableOutputDisplay )
  {
    initOutputDisplay();
  }

  // Initialize training mode if in gui mode
  if( settings.IsTrainingMode && !settings.UseFileForTraining ) {
    if( !initializeTrainingMode( outputDir, outputFile ) ) {
      cerr << "ERROR: Could not initiate training mode!\n";
    }
  }

#ifdef USE_PTHREADS

  // Support removed at the moment!

  /*initializeThreads();
  setThreadsAsAvailable();
  pthread_t pid[ MAX_THREADS ];
  pthread_t earliest;
  
  // Cycle through all input files
  cout << "Processing Files: \n\n";
  for( unsigned int i=0; i<inputFilenames.size(); i++ ) {
    
    // Go through all threads
    bool launched = false;
    while( !launched ) {

      // Cycle through threads checking for availability
      for( unsigned int idx = 0; idx < THREADS; idx++ ) {
        if( !isThreadRunning(idx) ) {
          int dirLength = inputDir.size() + 1;
          int fileLength = inputFilenames[i].size() - dirLength;
          string filenameNoDir = inputFilenames[i].substr(dirLength, fileLength);
          cout << "Processing " << filenameNoDir << "...  \n";
          inputArgs[idx].InputFilename = inputFilenames[i];
          inputArgs[idx].OutputFilename = outputFilenames[i];
          inputArgs[idx].ThreadID = idx;
          inputArgs[idx].InputFilenameNoDir = filenameNoDir;
          markThreadAsRunning( idx );
          pthread_create( &pid[idx], NULL, ProcessImage, &inputArgs[idx] );
          launched = true;
          break;
        }
      }

      // [Replace this with a conditioned wait!]
      sleep(0.01);
    }
  }
  
  // Halt execution until all worker threads are finished
  for( unsigned int idx = 0; idx < THREADS; idx++ ) {
    pthread_join( pid[idx], NULL );
  }

  // Deallocate threading variables
  killThreads();*/

#else

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

    // Execute processing
    ProcessImage( inputArgs );

#ifdef ENABLE_BENCHMARKING
    // Output benchmarking results to file
    for( unsigned int i=0; i<ExecutionTimes.size(); i++ )
      BenchmarkingOutput << ExecutionTimes[i] << " ";
    BenchmarkingOutput << endl;
#endif

    // Checks if user entered EXIT command in training mode
    if( settings.IsTrainingMode && training_exit_flag )
    {
      break;
    }
  }  
#endif

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
  BenchmarkingOutput.close();
#endif

  // Close gui-training mode
  if( settings.IsTrainingMode && !settings.UseFileForTraining )
  {
    exitTrainingMode();
  }

  // Deallocate GT info if in training mode
  if( !GTs )
  {
    delete GTs;
  }

  return 0;
}

// Streaming class definition, for use by external libraries
class CoreDetector::Priv
{
public:

  int lol;
};

CoreDetector::CoreDetector( std::string configFile )
{
  
}

CoreDetector::CoreDetector( const SystemParameters& settings )
{
  
}

CoreDetector::~CoreDetector()
{
  
}

std::vector< Detection >
CoreDetector::processFrame( const cv::Mat& image )
{
  
}

std::vector< Detection >
CoreDetector::processFrame( std::string filename )
{
  
}

}
