//------------------------------------------------------------------------------
// Title: Scallop Detector
// Author: Matthew Dawkins
// Description: Run the core detection pipeline in an executable
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

// Standard C/C++
#include <iostream>
#include <string>
#include <algorithm>

// Scallop Includes
#include "ScallopTK/Pipelines/CoreDetector.h"
#include "ScallopTK/Utilities/ConfigParsing.h"
#include "ScallopTK/Utilities/Threads.h"

//------------------------------------------------------------------------------
//                               Configurations
//------------------------------------------------------------------------------

// Namespaces
using namespace std;
using namespace ScallopTK;

//------------------------------------------------------------------------------
//                              Helper Functions
//------------------------------------------------------------------------------

void runCNNTrainingHelper()
{
  string trainLoc, caffeBinLoc, chipLoc, response;
  unsigned categories, bgdsRate;
  float perVal;
  vector< vector< string > > ids;
  vector< pair< string, int > > data;

  // Get user inputs
  cout << "Enter output directory to store all training files: ";
  getline( cin, trainLoc );
  cout << "Enter directory containing all caffe binaries: ";
  getline( cin, caffeBinLoc );
  cout << "Enter extracted image chip input directory: ";
  getline( cin, chipLoc );
  cout << "Enter total number of model categories (excluding background): ";
  cin >> categories;
  cout << "Enter background category chip downsample rate: ";
  cin >> bgdsRate;
  cout << "Enter percent of dataset to use as validation [0.0,1.0]: ";
  cin >> perVal;

  ids.push_back( vector< string >( 1, "background" ) );

  for( unsigned i = 0; i < categories; i++ )
  {
    ids.push_back( vector< string >() );

    cout << "Enter comma-seperate species IDs for model category " << INT_2_STR( i+1 ) << ": ";
    getline( cin, response );

    ids[i+1] = convertCSVLine( response, true );
  }

  cout << "Enter comma-seperate species IDs to include in background: ";
  getline( cin, response );

  vector< string > additions = convertCSVLine( response, true );
  ids[0].insert( ids[0].begin(), additions.begin(), additions.end() );

  // Read in image filename and label for each category
  for( unsigned i = 0; i < categories; i++ )
  {
    vector< string > categoryDirs;
    vector< string > backgroundDirs;

    for( unsigned j = 0; j < ids[i].size(); j++ )
    {
      if( ids[i][j] == "background" )
      {
        categoryDirs.push_back( chipLoc + "/background" );
      }
      else
      {
        backgroundDirs.push_back( chipLoc + "/" + ids[i][j] + "_0.10/" );

        categoryDirs.push_back( chipLoc + "/" + ids[i][j] + "_0.70/" );
        categoryDirs.push_back( chipLoc + "/" + ids[i][j] + "_0.80/" );
        categoryDirs.push_back( chipLoc + "/" + ids[i][j] + "_0.90/" );
      }
    }

    for( unsigned j = 0; j < categoryDirs.size(); j++ )
    {
      vector< string > files, dirs;
      listAllFile( categoryDirs[i], files, dirs );

      for( unsigned k = 0; k < files.size(); k++ )
      {
        data.push_back( make_pair( files[k], i ) );
      }
    }

    for( unsigned j = 0; j < backgroundDirs.size(); j++ )
    {
      vector< string > files, dirs;
      listAllFile( backgroundDirs[i], files, dirs );

      for( unsigned k = 0; k < files.size(); k++ )
      {
        data.push_back( make_pair( files[k], 0 ) );
      }
    }
  }

  // Shuffle list, divide into val and training
  random_shuffle( data.begin(), data.end() );

  // Output lists
  createDir( trainLoc );

  string trainFilelist = trainLoc + "/training.txt";
  string validationFilelist = trainLoc + "/validation.txt";

  ofstream train( trainFilelist.c_str() );
  ofstream val( validationFilelist.c_str() );

  unsigned center = ( 1.0 - perVal ) * data.size();

  for( unsigned i = 0; i < center; i++ )
  {
    train << data[i].first << " " << data[i].second << endl;
  }

  for( unsigned i = center; i < data.size(); i++ )
  {
    val << data[i].first << " " << data[i].second << endl;
  }

  train.close();
  val.close();

  // Call caffe database generator
  // []

  // Inject category count into CNN file
  // []

  // Call caffe training utility
  // []
}

//------------------------------------------------------------------------------
//                                Main Function
//------------------------------------------------------------------------------

int main( int argc, char** argv )
{
  // Special case for command line training helper utility
  if( argc == 2 && argv[1] == "CNN_TRAINING_UTIL" )
  {
    runCNNTrainingHelper();
  }

  // Variables as defined in definitions.h
  SystemParameters settings;
  string mode, input, output, config, classifier;

  // Output Welcome Message
  // ASCII art from:
  // http://www.chris.com/ascii/index.php?art=sports%20and%20activities%2Fscuba
  cout << endl;
  cout << "~~~~~~~~~~~SCALLOP DETECTOR~~~~~~~~~~~~" << endl;
  cout << endl;
  cout << "           _.-''|''-._ " << endl;
  cout << "        .-'     |     `-.       __\\" << endl;
  cout << "      .'\\       |       /`. _  (..)\\" << endl;
  cout << "    .'   \\      |      /   `.\\ \\ ,_/ " << endl;
  cout << "    \\     \\     |     /     / \\ )(_ " << endl;
  cout << "     `\\    \\    |    /    /'   `|| \\" << endl;
  cout << "       `\\   \\   |   /   /'      || /" << endl;
  cout << "         `\\  \\  |  /  /'        |\\ `" << endl;
  cout << "        _.-`\\ \\ | / /'-._       | \\" << endl;
  cout << "       {_____`\\\\|//'_____}  ____| /" << endl;
  cout << "               `-'          \\_.-' \\`." << endl;
  cout << "                                   ~~~ " << endl;
  cout << " Software Author: MDD " << endl;
  cout << " ASCII Art Author: JGS " << endl;
  cout << endl;

  // If we're running via standard command line usage...
  if( argc == 4 || argc == 5 ) {

    mode = argv[1];
    input = argv[2];
    output = argv[3];

    if( !parseSystemConfig( settings ) )
    {
      return false;
    }
    if( !parseModeConfig( mode, settings ) )
    {
      return false;
    }
    if( settings.IsInputDirectory && !(settings.IsTrainingMode && settings.UseFileForTraining) )
    {
      settings.InputDirectory = input;
      settings.InputFilename = "";

      if( input.size() > 0 && input[ input.size()-1 ] != '\\' && input[ input.size()-1 ] != '/' )
      {
        settings.InputDirectory = settings.InputDirectory + "/";
      }
      splitPathAndFile( output, settings.OutputDirectory, settings.OutputFilename );
    }
    else
    {
      splitPathAndFile( input, settings.InputDirectory, settings.InputFilename );
      splitPathAndFile( output, settings.OutputDirectory, settings.OutputFilename );
    }

    if( argc == 5 )
    {
      settings.ClassifierToUse = argv[4];
    }
  }
  // If we haven't specified any arguments...
  else if( argc == 1 ) 
  {
    cout << "Enter input foldername: ";
    cin >> input;
    cout << "Enter output list filename: ";
    cin >> output;
    cout << "Enter classifier to use (enter DEFAULT for default): ";
    cin >> classifier;

    if( !parseSystemConfig( settings ) )
    {
      cerr << "ERROR: Unable to parse system config!" << endl;
      return false;
    }
    if( !parseModeConfig( "PROCESS_DIR", settings ) )
    {
      cerr << "ERROR: Unable to parse process_dir config!" << endl;
      return false;
    }
    
    settings.ClassifierToUse = classifier;

    splitPathAndFile( output, settings.OutputDirectory, settings.OutputFilename );

    settings.InputDirectory = input;
    settings.InputFilename = "";
  }
  // If we specified too many arguments...
  else
  {
    cout << endl;
    cout << "Scallop Detector Usage: " << endl;
    cout << "./ScallopDetector [MODE] [INPUT_LIST/DIR] [OUTPUT_LIST/DATA]" << endl;
    cout << endl;
    return 0;
  }

  // Validate number of specified threads
  if( settings.NumThreads < 1 || settings.NumThreads > MAX_THREADS ) {
    cerr << "WARNING: Invalid number of threads. Defaulting to 1." << endl;
    settings.NumThreads = 1;
  }

#ifdef WIN32
  // If on windows, default to 1 thread
  if( settings.NumThreads != 1 ) {
    cerr << endl;
    cerr << "WARNING: Windows version does not support threading at this time. ";
    cerr << "WARNING: Defaulting to 1 thread." << endl;
    settings.NumThreads = 1;
  }
#endif

  // If in training mode...
  if( settings.IsTrainingMode ) 
  {
    cout << endl << "TRAINING MODE INITIALIZING" << endl;

    if( settings.NumThreads != 1 ) {
      cerr << endl;
      cerr << "WARNING: Threading serves no purpose in training mode. ";
      cerr << "WARNING: Defaulting to 1 thread." << endl;
      settings.NumThreads = 1;
    }
  }

  // Count scallops!
  int count = runCoreDetector( settings );

  // Output completion statement
  cout << endl << "Processing Complete." << endl;
  return true;
}
