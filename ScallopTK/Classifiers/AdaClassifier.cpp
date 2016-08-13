
#include "AdaClassifier.h"

#include "ScallopTK/Utilities/HelperFunctions.h"

// Loads classifiers from given folder
/*Classifier* loadClassifiers( const SystemParameters& sparams, ClassifierParameters& cparams )
{
  Classifier* output = new Classifier;
  string dir = sparams.RootClassifierDIR + cparams.ClassifierSubdir;
  output->IsScallopDirected = false;
  output->SDSS = cparams.EnabledSDSS;
  output->Threshold = cparams.Threshold;

  // Load Main Classifiers
  for( int i = 0; i < cparams.L1Files.size(); i++ )
  {
    // Declare new classifier
    Classifier MainClass;
    MainClass.ID = cparams.L1Keys[i];
    MainClass.Type = MAIN_CLASS;
    string path_to_clfr = cparams.L1Files[i];
    FILE *file_rdr = fopen(path_to_clfr.c_str(),"r");

    // Check to make sure file is open
    if( !file_rdr )
    {
      std::cout << std::endl << std::endl;
      std::cout << "CRITICAL ERROR: Could not load classifier " << path_to_clfr << std::endl;
      return NULL;
    }

    // Actually load classifier
    MainClass.adaTree.LoadFromFile(file_rdr);
    fclose(file_rdr);

    // Set special conditions
    MainClass.isSandDollar = ( cparams.L1SpecTypes[i] == SAND_DOLLAR );
    MainClass.isScallop = ( cparams.L1SpecTypes[i] == ALL_SCALLOP );
    MainClass.isWhite = ( cparams.L1SpecTypes[i] == WHITE_SCALLOP );
    MainClass.isBrown = ( cparams.L1SpecTypes[i] == BROWN_SCALLOP );
    MainClass.isBuried = ( cparams.L1SpecTypes[i] == BURIED_SCALLOP );
    
    // Set is scallop classifier flag
    if( MainClass.isWhite || MainClass.isBrown || MainClass.isBuried || MainClass.isScallop )
    {
      output->IsScallopDirected = true;
    }

    // Add classifier to system
    output->MainClassifiers.push_back( MainClass );    
  }

  // Load suppression classifiers
  for( int i = 0; i < cparams.L2Files.size(); i++ )
  {
    // Declare new classifier
    Classifier SuppClass;
    SuppClass.ID = cparams.L2Keys[i];
    SuppClass.Type = MIXED_CLASS;
    if( cparams.L2SuppTypes[i] == WORLD_VS_OBJ_STR )
      SuppClass.Type = WORLD_VS_OBJ;
    else if( cparams.L2SuppTypes[i] == DESIRED_VS_OBJ_STR )
      SuppClass.Type = DESIRED_VS_OBJ;
    string path_to_clfr = cparams.L2Files[i];
    FILE *file_rdr = fopen(path_to_clfr.c_str(),"r");

    // Check to make sure file is open
    if( !file_rdr )
    {
      std::cout << "CRITICAL ERROR: Could not load classifier " << path_to_clfr << std::endl;
      return NULL;
    }

    // Actually load classifier
    SuppClass.adaTree.LoadFromFile(file_rdr);
    fclose(file_rdr);

    // Set special conditions
    SuppClass.isSandDollar = ( cparams.L2SpecTypes[i] == SAND_DOLLAR );
    SuppClass.isScallop = ( cparams.L2SpecTypes[i] == ALL_SCALLOP );
    SuppClass.isWhite = ( cparams.L2SpecTypes[i] == WHITE_SCALLOP );
    SuppClass.isBrown = ( cparams.L2SpecTypes[i] == BROWN_SCALLOP );
    SuppClass.isBuried = ( cparams.L2SpecTypes[i] == BURIED_SCALLOP );

    // Add classifier to system
    output->SuppressionClassifiers.push_back( SuppClass );  
  }
  
  // Check results
  if( output->SuppressionClassifiers.size() + output->MainClassifiers.size() >= MAX_CLASSIFIERS )
  {
    std::cout << "CRITICAL ERROR: Increase max allowed number of classifiers!" << endl;
    return NULL;
  }

  return output;
}


// td; Use a binary file next time
int classifyCandidate( Candidate *cd, Classifier* Classifiers ) {

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
  for( int i=0; i<1764; i++ ) {
    float value = ((float*)(mat->data.ptr))[i];
    input[pos++] = value;
  }

  // Print HoG2
  mat = cd->hogResults[1];
  for( int i=0; i<1764; i++ ) {
    float value = ((float*)(mat->data.ptr))[i];
    input[pos++] = value;
  }

  // Print Gabor features
  for( int i=0; i<GABOR_FEATURES; i++ )
    input[pos++] = cd->gaborFeatures[i];

  // Classify our interest point based on the above features
  std::vector< Classifier >& MainClassifiers = Classifiers->MainClassifiers;
  std::vector< Classifier >& SuppressionClassifiers = Classifiers->SuppressionClassifiers;
  int idx = 0;
  pos = 0;
  double max = -1.0;
  for( int i = 0; i < MainClassifiers.size(); i++ )
  {
    cd->classMagnitudes[pos] = MainClassifiers[i].adaTree.Predict( input );
    if( cd->classMagnitudes[pos] > max )
    {
      max = cd->classMagnitudes[pos];
      idx = pos;
    }
    pos++;
  }

  // If we passed any of the above, compute secondary classifiers
  if( max >= Classifiers->Threshold ) {

    for( int i = 0; i < SuppressionClassifiers.size(); i++ )
    {
      cd->classMagnitudes[pos] = SuppressionClassifiers[i].adaTree.Predict( input );
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
}*/
