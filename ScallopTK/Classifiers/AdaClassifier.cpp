
#include "AdaClassifier.h"

#include "ScallopTK/Utilities/HelperFunctions.h"

// Loads classifiers from given folder
bool AdaClassifier::loadClassifiers(
  const SystemParameters& sysParams,
  const ClassifierParameters& clsParams )
{
  string dir = sysParams.RootClassifierDIR + clsParams.ClassifierSubdir;
  isScallopDirected = false;
  threshold = clsParams.Threshold;

  // Load Main Classifiers
  for( int i = 0; i < clsParams.L1Files.size(); i++ )
  {
    // Declare new classifier
    SingleAdaClassifier MainClass;
    MainClass.id = clsParams.L1Keys[i];
    MainClass.type = MAIN_CLASS;
    string path_to_clfr = clsParams.L1Files[i];
    FILE *file_rdr = fopen(path_to_clfr.c_str(),"r");

    // Check to make sure file is open
    if( !file_rdr )
    {
      std::cout << std::endl << std::endl;
      std::cout << "CRITICAL ERROR: Could not load classifier " << path_to_clfr << std::endl;
      return false;
    }

    // Actually load classifier
    MainClass.adaTree.LoadFromFile(file_rdr);
    fclose(file_rdr);

    // Set special conditions
    MainClass.isSandDollar = ( clsParams.L1SpecTypes[i] == SAND_DOLLAR );
    MainClass.isScallop = ( clsParams.L1SpecTypes[i] == ALL_SCALLOP );
    MainClass.isWhite = ( clsParams.L1SpecTypes[i] == WHITE_SCALLOP );
    MainClass.isBrown = ( clsParams.L1SpecTypes[i] == BROWN_SCALLOP );
    MainClass.isBuried = ( clsParams.L1SpecTypes[i] == BURIED_SCALLOP );
    
    // Set is scallop classifier flag
    if( MainClass.isWhite || MainClass.isBrown || MainClass.isBuried || MainClass.isScallop )
    {
      isScallopDirected = true;
    }

    // Add classifier to system
    mainClassifiers.push_back( MainClass );    
  }

  // Load suppression classifiers
  for( int i = 0; i < clsParams.L2Files.size(); i++ )
  {
    // Declare new classifier
    SingleAdaClassifier SuppClass;
    SuppClass.id = clsParams.L2Keys[i];
    SuppClass.type = MIXED_CLASS;
    if( clsParams.L2SuppTypes[i] == WORLD_VS_OBJ_STR )
      SuppClass.type = WORLD_VS_OBJ;
    else if( clsParams.L2SuppTypes[i] == DESIRED_VS_OBJ_STR )
      SuppClass.type = DESIRED_VS_OBJ;
    string path_to_clfr = clsParams.L2Files[i];
    FILE *file_rdr = fopen(path_to_clfr.c_str(),"r");

    // Check to make sure file is open
    if( !file_rdr )
    {
      std::cout << "CRITICAL ERROR: Could not load classifier " << path_to_clfr << std::endl;
      return false;
    }

    // Actually load classifier
    SuppClass.adaTree.LoadFromFile(file_rdr);
    fclose(file_rdr);

    // Set special conditions
    SuppClass.isSandDollar = ( clsParams.L2SpecTypes[i] == SAND_DOLLAR );
    SuppClass.isScallop = ( clsParams.L2SpecTypes[i] == ALL_SCALLOP );
    SuppClass.isWhite = ( clsParams.L2SpecTypes[i] == WHITE_SCALLOP );
    SuppClass.isBrown = ( clsParams.L2SpecTypes[i] == BROWN_SCALLOP );
    SuppClass.isBuried = ( clsParams.L2SpecTypes[i] == BURIED_SCALLOP );

    // Add classifier to system
    suppressionClassifiers.push_back( SuppClass );  
  }
  
  // Check results
  if( suppressionClassifiers.size() + mainClassifiers.size() >= MAX_CLASSIFIERS )
  {
    std::cout << "CRITICAL ERROR: Increase max allowed number of classifiers!" << endl;
    return false;
  }

  return true;
}


// td; Use a binary file next time
int AdaClassifier::classifyCandidate( cv::Mat image, Candidate* cd )
{
  // Exit if inactive flag set
  if( !cd->isActive )
    return 0;

  // Build input to classifier
  double input[3900];
  int pos = 0;

  // Print Size features
  for( int i=0; i<SIZE_FEATURES; i++ ) {
    input[pos++] = cd->sizeFeatures[i];
  }

  // Print color features
  for( int i=0; i<COLOR_FEATURES; i++ )
    input[pos++] = cd->colorFeatures[i];

  // Print edge features
  for( int i=0; i<EDGE_FEATURES; i++ )
    input[pos++] = cd->edgeFeatures[i];

  // Print HoG1
  CvMat* mat = cd->hogResults[0];
  for( int i=0; i<HOG_FEATURES; i++ ) {
    float value = ((float*)(mat->data.ptr))[i];
    input[pos++] = value;
  }

  // Print HoG2
  mat = cd->hogResults[1];
  for( int i=0; i<HOG_FEATURES; i++ ) {
    float value = ((float*)(mat->data.ptr))[i];
    input[pos++] = value;
  }

  // Print Gabor features
  for( int i=0; i<GABOR_FEATURES; i++ )
    input[pos++] = cd->gaborFeatures[i];

  // Classify our interest point based on the above features
  int idx = 0;
  pos = 0;
  double max = -1.0;
  for( int i = 0; i < mainClassifiers.size(); i++ )
  {
    cd->classMagnitudes[pos] = mainClassifiers[i].adaTree.Predict( input );
    if( cd->classMagnitudes[pos] > max )
    {
      max = cd->classMagnitudes[pos];
      idx = pos;
    }
    pos++;
  }

  // If we passed any of the above, compute secondary classifiers
  if( max >= threshold ) {

    for( int i = 0; i < suppressionClassifiers.size(); i++ )
    {
      cd->classMagnitudes[pos] = suppressionClassifiers[i].adaTree.Predict( input );
      if( cd->classMagnitudes[pos] > max )
      {
        max = cd->classMagnitudes[pos];
        idx = pos;
      }
      pos++;
    }
    cd->classification = idx;
    return idx+1;
  }
  cd->classification = UNCLASSIFIED;
  return UNCLASSIFIED;
}

void AdaClassifier::classifyCandidates(
  cv::Mat image,
  CandidatePtrVector& candidates,
  CandidatePtrVector& positive )
{
  positive.clear();

  for( unsigned int i=0; i<candidates.size(); i++ ) {

    if( classifyCandidate( image, candidates[i] ) > 0 )
      positive.push_back( candidates[i] );
  }
}
