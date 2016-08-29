//------------------------------------------------------------------------------
//                               Include Files
//------------------------------------------------------------------------------

//Scallop Includes
#include "ScallopTK/Classifiers/Classifier.h"
#include "ScallopTK/Classifiers/AdaClassifier.h"
#include "ScallopTK/Utilities/Threads.h"

#ifdef USE_CAFFE
#include "ScallopTK/Classifiers/CNNClassifier.h"
#endif

//Standard C/C++
#include <vector>

//OpenCV
#include <cv.h>

//------------------------------------------------------------------------------
//                          Assorted Helper Functions
//------------------------------------------------------------------------------

namespace ScallopTK
{

using namespace std;

// Load a new classifier
Classifier* loadClassifiers(
  const SystemParameters& sysParams,
  const ClassifierParameters& clsParams )
{
  Classifier* output;

  if( !clsParams.UseCNNClassifier )
  {
    output = new AdaClassifier();
  }
#ifdef USE_CAFFE
  else
  {
    output = new CNNClassifier();
  }
#else
  else
  {
    cerr << "ERROR: A build with caffe is required to use CNN classifier" << endl;
    return NULL;
  }
#endif

  if( !output->loadClassifiers( sysParams, clsParams ) )
  {
    delete output;
    return NULL;
  }

  return output;
}

// Sorting function 1 - based on major axis of ip
bool compareCandidateSize( Candidate* cd1, Candidate* cd2  ) {
  float avg1 = /*cd1->minor +*/ cd1->major;
  float avg2 = /*cd2->minor +*/ cd2->major;
  if( avg1 > avg2 ) 
    return true;
  return false;
}

// Sorting function 2 - sort based on classification magnitude
bool sortByMag( Candidate* cd1, Candidate* cd2 ) {
  if( cd1->classMagnitudes[cd1->classification] > cd2->classMagnitudes[cd2->classification] )
    return true;
  return false;
}

// Returns 0 - no intersection, 1 - overlap, 2 - 2 submerged in 1, 3 - 1 submerged in 2

// Simple Approximation
int ellipseIntersectStatus( Candidate* cd1, Candidate *cd2 ) {

  // Cirlce approx for simplicity for now (doesn't matter in this environment)
  float xdif = (cd1->r - cd2->r);
  float ydif = (cd1->c - cd2->c);
  float dist = xdif*xdif + ydif*ydif;
  dist = sqrt( dist );
  if( dist < cd1->major*2 && dist < cd2->major*2 )
    return 1;
  return 0;
}

float ellipseIntersectStatus2( Candidate* cd1, Candidate *cd2 ) {

  // Calculate area of overlap (assumes circles)
  /*float xdif = (cd1->r - cd2->r);
  float ydif = (cd1->c - cd2->c);
  float distsq = xdif*xdif + ydif*ydif;
  float d = sqrt( distsq );

  // Continue
  float r = cd1->major;
  float R = cd2->major;
  if( r > R ) {
    swap( r, R );
  }

  // Check to make sure don't have same center
  if( d == 0.0 )
    return 1.0;
  if( d > 2*r && d > 2*R )
    return 0.0;

  float rsq = cd1->major * cd1->major;
  float Rsq = cd2->major * cd2->major;
  float term1 = (distsq + rsq - Rsq) / (2 * d * r);
  float term2 = (distsq - rsq + Rsq) / (2 * d * R);
  float term3 = (-d+r+R)*(d+r-R)*(d-r+R)*(d+r+R);
  float area = rsq * acos( term1 ) + Rsq * acos( term2 ) - 0.5 * sqrt( term3 );

  cout << "DEBUG: " << r << " " << R << " " << rsq << " " << term1 << " " << term2 << " " << term3 << " " << d << endl;

  // Calc min %
  if( r < R ) {
    return area / ( PI * rsq );
  }
  return area / ( PI * Rsq );*/

  float r = cd1->major;
  float R = cd2->major;
  float xdif = (cd1->r - cd2->r);
  float ydif = (cd1->c - cd2->c);
  float distsq = xdif*xdif + ydif*ydif;
  float d = sqrt( distsq );

  if( R < r ) {
    float temp = r;
    r = R;
    R = temp;
  }

  if( d < R - r )
    return 1.0;
  if( d > r + R  )
    return 0.0;
  return ((R+r)-d)/(2*r);
}

// Clean up results for display
void scallopCleanUp( CandidatePtrVector& input, CandidatePtrVector& output, int method ) {

  // Adjust elliptical ips
  for( unsigned int i=0; i<input.size(); i++ ) {
    input[i]->minor = (input[i]->major - input[i]->minor)*0.5 + input[i]->minor;
  }

  // Sort input vector by Candidate size
  sort( input.begin(), input.end(), compareCandidateSize);

  // Create linked grouped structure
  vector< CandidatePtrVector > ol;
  for( unsigned int i=0; i<input.size(); i++ ) {
    bool standalone = true;
    for( unsigned int j=0; j<ol.size(); j++ ) {
      if( ellipseIntersectStatus( ol[j][0], input[i] ) != 0 ) {
        ol[j].push_back( input[i] );
        standalone = false;
        break;
      }
    }
    if( standalone ) {
      CandidatePtrVector temp;
      temp.push_back( input[i] );
      ol.push_back( temp );
    }
  }

  // Average group (method 0)
  if( method == 0 ) {
    for( unsigned int i=0; i<ol.size(); i++ ) {
      output.push_back( ol[i][0] );
    }
  }

  // Take local min overlapping maximas (method 1)
  if( method == 1 ) { 
    for( unsigned int i=0; i<ol.size(); i++ ) {

      // Sort entries by magnitude
      sort( ol[i].begin(), ol[i].end(), sortByMag ); 
      CandidatePtrVector toadd;

      // Take max non-overlapping entries
      for( unsigned int j=0; j<ol[i].size(); j++ ) {
        
        // Compare entries to all of those already being added
        bool add_entry = true;
        for( unsigned int k=0; k<toadd.size(); k++ ) {
          if( ellipseIntersectStatus( ol[i][j], ol[i][k] ) != 0 ) { 
            add_entry = false;
            break;
          }
        }
        if( add_entry )
          toadd.push_back( ol[i][j] );
      }

      // Add entries
      for( int j=0; j<toadd.size(); j++ ) 
        output.push_back( toadd[j] );
    }
  }
}

/*void removeInsidePoints( CandidatePtrVector& input, CandidatePtrVector& output ) {
  
  // Adjust elliptical ips
  for( int i=0; i<input.size(); i++ ) {
    input[i]->minor = (input[i]->major - input[i]->minor)*0.5 + input[i]->minor;
  }

  // Sort input vector by Candidate size
  sort( input.begin(), input.end(), compareCandidateSize);

  // Create linked grouped structure
  vector< vector< Candidate* > > ol;
  for( int i=0; i<input.size(); i++ ) {
    bool standalone = true;
    for( int j=0; j<ol.size(); j++ ) {
      if( ellipseIntersectStatus( ol[j][0], input[i] ) != 0 ) {
        ol[j].push_back( input[i] );
        standalone = false;
        break;
      }
    }
    if( standalone ) {
      vector< Candidate* > temp;
      temp.push_back( input[i] );
      ol.push_back( temp );
    }
  }

  // Take local min overlapping maximas
  for( int i=0; i<ol.size(); i++ ) {

    // Sort entries by magnitude
    //sort( ol[i].begin(), ol[i].end(), sortByFocus ); 
    sort( ol[i].begin(), ol[i].end(), sortByMag ); 
    CandidatePtrVector toadd;

    // Take max non-overlapping entries
    for( int j=0; j<ol[i].size(); j++ ) {

      // Compare entries to all of those already being added
      bool add_entry = true;
      for( int k=0; k<toadd.size(); k++ ) {
        float perc_overlap = ellipseIntersectStatus2( ol[i][j], ol[i][k] );
        if( perc_overlap > 0.25 ) { 
          add_entry = false;
          break;
        }
      }
      if( add_entry )
        toadd.push_back( ol[i][j] );
    }

    // Add entries
    for( int j=0; j<toadd.size(); j++ ) {
      output.push_back( toadd[j] );
    }
  }
}*/

void removeInsidePoints( CandidatePtrVector& input, CandidatePtrVector& output ) {
  
  // Adjust elliptical ips
  for( int i=0; i<input.size(); i++ ) {
    input[i]->minor = (input[i]->major - input[i]->minor)*0.5 + input[i]->minor;
  }

  // Sort input vector by Candidate size
  sort( input.begin(), input.end(), sortByMag );

  // Take local min overlapping maximas
  for( int i=0; i<input.size(); i++ ) {

    // Compare entries to all of those already being added
    bool add_entry = true;
    for( int j=0; j<output.size(); j++ ) {
      float perc_overlap = ellipseIntersectStatus2( output[j], input[i] );
      if( perc_overlap > 0.25 ) { 
        add_entry = false;
        break;
      }
    }
    if( add_entry )
      output.push_back( input[i] );
  }
}

void getSortedIDs( vector<int>& indices, vector<double> values )
{
  for( int j = 0; j < values.size(); j++ )
  {
    double max = -10000;
    int maxind = -1;
    for( int i = 0; i < values.size(); i++ ) 
    {
      if( values[i] > max )
      {
        max = values[i];
        maxind = i;
      }
    }
    indices.push_back( maxind );
    values[maxind] = -10000;
  }
}

bool appendInfoToFile( DetectionPtrVector& cds, const string& ListFilename, const string& this_fn, float resize_factor ) {
  
  // Get lock on file
  getListLock();
  
  // Open file and output
  ofstream fout( ListFilename.c_str(), ios::app );
  if( !fout.is_open() ) {
    cout << "ERROR: Could not open output list for writing!\n";
    return false;
  }
  
  // print out all results for every Candidate and every possible classification
  for( unsigned int i=0; i<cds.size(); i++ ) {
    
    // Sort possible classifications in descending value based on classification values
    vector<int> sorted_ind;
    getSortedIDs( sorted_ind, cds[i]->classProbabilities );
    
    // Output all possible classifications if enabled    
    for( unsigned int j=0; j<cds[i]->classIDs.size(); j++ )
    {
      int cind = sorted_ind[j];
      float r = cds[i]->r / resize_factor;
      float c = cds[i]->c / resize_factor;
      float maj = cds[i]->major / resize_factor;
      float minor = cds[i]->minor / resize_factor;
      fout << this_fn << "," << r << "," << c << "," << maj << "," << minor << "," << cds[i]->angle << ",";
      fout << cds[i]->classIDs[cind] << "," << cds[i]->classProbabilities[cind] << "," << int(j+1) << endl;
    }
  }
  // so the full output string is (imagename y x major_axis minor_axis angle class class-confidence rank) 

  // Close output file
  fout.close();
  
  // Remove lock on file
  unlockList();
  return true;
}

void cullSimilarObjects( CandidatePtrVector& input, bool clams, bool dollars, bool urchins, bool sacs, bool misc ) {

  // Suppress clam responses
  if( clams ) {
    for( int ind = input.size()-1; ind >= 0; ind-- ) {
      double scallopClassMag = input[ind]->classMagnitudes[ input[ind]->classification ];
      double clamClassMag = input[ind]->classMagnitudes[ CLAM ];
      if( scallopClassMag < clamClassMag ) {
        input.erase( input.begin() + ind );
      }
    }
  }

  // Suppress dollar responses
  if( dollars ) {
    for( int ind = input.size()-1; ind >= 0; ind-- ) {
      double scallopClassMag = input[ind]->classMagnitudes[ input[ind]->classification ];
      double dollarClassMag = input[ind]->classMagnitudes[ DOLLAR ];
      if( scallopClassMag < dollarClassMag ) {
        input.erase( input.begin() + ind );
      }
    }
  }

  // Suppress urchin responses
  if( urchins ) {
    for( int ind = input.size()-1; ind >= 0; ind-- ) {
      double scallopClassMag = input[ind]->classMagnitudes[ input[ind]->classification ];
      double urchinClassMag = input[ind]->classMagnitudes[ URCHIN ];
      if( scallopClassMag < urchinClassMag ) {
        input.erase( input.begin() + ind );
      }
    }
  }

  // Suppress sac responses
  if( sacs ) {
    for( int ind = input.size()-1; ind >= 0; ind-- ) {
      double scallopClassMag = input[ind]->classMagnitudes[ input[ind]->classification ];
      double sacClassMag = input[ind]->classMagnitudes[ SAC ];
      if( scallopClassMag < sacClassMag ) {
        input.erase( input.begin() + ind );
      }
    }
  }

  // Suppress misc other responses
  if( misc ) {
    for( int ind = input.size()-1; ind >= 0; ind-- ) {
      double scallopClassMag = input[ind]->classMagnitudes[ input[ind]->classification ];
      double miscClassMag = input[ind]->classMagnitudes[ UNCLASSIFIED ];
      if( scallopClassMag < miscClassMag ) {
        input.erase( input.begin() + ind );
      }
    }
  }
}

void sandDollarSuppression( CandidatePtrVector& input, bool& scallopMode ) {

  int scallop_count = 0;
  int sand_dollar_count = 0;

  // perform suppression
  for( unsigned int ind = 0; ind < input.size(); ind++ ) {

    double scallopClassMag = input[ind]->classMagnitudes[ input[ind]->classification ];
    double dollarClassMag = input[ind]->classMagnitudes[ DOLLAR ];

    if( scallopMode ) {

      if( scallopClassMag < dollarClassMag / SDSS_DIFFERENCE_FACTOR ) {
        input.erase( input.begin() + ind );
        sand_dollar_count++;
      } else {
        scallop_count++;
      }

    } else {

      if( dollarClassMag > scallopClassMag / SDSS_DIFFERENCE_FACTOR ) {
        input.erase( input.begin() + ind );
        sand_dollar_count++;
      } else {
        scallop_count++;
      }
    }
  }

  // adjust mode based on Detections
  if( scallop_count >= sand_dollar_count ) {
    scallopMode = true;
  } else {
    scallopMode = false;
  }
}

void deallocateDetections( DetectionPtrVector& vec )
{
  for( int i = 0; i < vec.size(); i++ )
  {
    delete vec[i];
  }
}

DetectionPtrVector interpolateResults( CandidatePtrVector& input,
  Classifier* Classifiers, string Filename )
{
  DetectionPtrVector output;
  
  int MainSize = Classifiers->outputClassCount();
  int SuppSize = Classifiers->suppressionClassCount();

  for( int i = 0; i < input.size(); i++ )
  {
    Detection* obj = new Detection;
    
    obj->r = input[i]->r;
    obj->c = input[i]->c;
    obj->angle = input[i]->angle;
    obj->major = input[i]->major;
    obj->minor = input[i]->minor;
    obj->cntr = NULL; // todo: check for and display
    
    ClassifierIDLabel* labelInfo;
    int best_class = input[i]->classification;

    if( best_class < MainSize )
      labelInfo = Classifiers->getLabel( best_class );
    else
      labelInfo = Classifiers->getSuppressionLabel( best_class-MainSize );

    obj->isBrownScallop = labelInfo->isScallop || labelInfo->isBrown;
    obj->isWhiteScallop = labelInfo->isWhite;
    obj->isSandDollar = labelInfo->isSandDollar;
    obj->isBuriedScallop = labelInfo->isBuried;

    double best_main_class_val = -10.0;
    for( int j = 0; j < MainSize; j++ ) {
      if( input[i]->classMagnitudes[j] >= 0 ) {
        obj->classIDs.push_back( Classifiers->getLabel( j )->id );
        obj->classProbabilities.push_back( input[i]->classMagnitudes[j] );
        if( input[i]->classMagnitudes[j] >= best_main_class_val )
          best_main_class_val = input[i]->classMagnitudes[j];
      }
    }
    
    for( int j = MainSize; j < MainSize + SuppSize; j++ ) {
      if( input[i]->classMagnitudes[j] >= 0 ) {
        obj->classIDs.push_back(
          Classifiers->getSuppressionLabel( j )->id );
        obj->classProbabilities.push_back(
          best_main_class_val * ( 1.0 + input[i]->classMagnitudes[j] ) );
      }
    }    
    output.push_back( obj );
  }

  return output;
}

}
