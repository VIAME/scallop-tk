
#include "TopClassifier.h"

// td; Use a binary file next time
int classifyCandidate( candidate *cd, ClassifierSystem *Classifiers ) {

	// Exit if inactive flag set
	if( !cd->is_active )
		return 0;

	// Build input to classifier
	double input[3900];
	int pos = 0;

	// Print Size features
	for( int i=0; i<SIZE_FEATURES; i++ ) {
		input[pos++] = cd->SizeFeatures[i];
	}

	// Print color features
	for( int i=0; i<COLOR_FEATURES; i++ )
		input[pos++] = cd->ColorFeatures[i];

	// Print edge features
	for( int i=0; i<EDGE_FEATURES; i++ )
		input[pos++] = cd->EdgeFeatures[i];

	// Print HoG1
	CvMat* mat = cd->HoGResult[0];
	for( int i=0; i<1764; i++ ) {
		float value = ((float*)(mat->data.ptr))[i];
		input[pos++] = value;
	}

	// Print HoG2
	mat = cd->HoGResult[1];
	for( int i=0; i<1764; i++ ) {
		float value = ((float*)(mat->data.ptr))[i];
		input[pos++] = value;
	}

	// Print Gabor features
	for( int i=0; i<GABOR_FEATURES; i++ )
		input[pos++] = cd->GaborFeatures[i];

	// Classify our interest point based on the above features
	std::vector< Classifier >& MainClassifiers = Classifiers->MainClassifiers;
	std::vector< Classifier >& SuppressionClassifiers = Classifiers->SuppressionClassifiers;
	int idx = 0;
	pos = 0;
	double max = -1.0;
	for( int i = 0; i < MainClassifiers.size(); i++ )
	{
		cd->class_magnitudes[pos] = MainClassifiers[i].Clsfr.Predict( input );
		if( cd->class_magnitudes[pos] > max )
		{
			max = cd->class_magnitudes[pos];
			idx = pos;
		}
		pos++;
	}

	// If we passed any of the above, compute secondary classifiers
	if( max >= Classifiers->Threshold ) {

		for( int i = 0; i < SuppressionClassifiers.size(); i++ )
		{
			cd->class_magnitudes[pos] = SuppressionClassifiers[i].Clsfr.Predict( input );
			if( cd->class_magnitudes[pos] > max )
			{
				max = cd->class_magnitudes[pos];
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

// Sorting function 1 - based on major axis of ip
bool compareCandidateSize( candidate* cd1, candidate* cd2  ) {
	float avg1 = /*cd1->minor +*/ cd1->major;
	float avg2 = /*cd2->minor +*/ cd2->major;
	if( avg1 > avg2 ) 
		return true;
	return false;
}

// Sorting function 2 - sort based on classification magnitude
bool sortByMag( candidate* cd1, candidate* cd2 ) {
	if( cd1->class_magnitudes[cd1->classification] > cd2->class_magnitudes[cd2->classification] )
		return true;
	return false;
}

// Returns 0 - no intersection, 1 - overlap, 2 - 2 submerged in 1, 3 - 1 submerged in 2

// Simple Approximation
int ellipseIntersectStatus( candidate* cd1, candidate *cd2 ) {

	// Cirlce approx for simplicity for now (doesn't matter in this environment)
	float xdif = (cd1->r - cd2->r);
	float ydif = (cd1->c - cd2->c);
	float dist = xdif*xdif + ydif*ydif;
	dist = sqrt( dist );
	if( dist < cd1->major*2 && dist < cd2->major*2 )
		return 1;
	return 0;
}

float ellipseIntersectStatus2( candidate* cd1, candidate *cd2 ) {

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
void scallopCleanUp( vector<candidate*>& input, vector<candidate*>& output, int method ) {

	// Adjust elliptical ips
	for( unsigned int i=0; i<input.size(); i++ ) {
		input[i]->minor = (input[i]->major - input[i]->minor)*0.5 + input[i]->minor;
	}

	// Sort input vector by candidate size
	sort( input.begin(), input.end(), compareCandidateSize);

	// Create linked grouped structure
	vector< vector< candidate* > > ol;
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
			vector< candidate* > temp;
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
			vector<candidate*> toadd;

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

/*void removeInsidePoints( vector<candidate*>& input, vector<candidate*>& output ) {
	
	// Adjust elliptical ips
	for( int i=0; i<input.size(); i++ ) {
		input[i]->minor = (input[i]->major - input[i]->minor)*0.5 + input[i]->minor;
	}

	// Sort input vector by candidate size
	sort( input.begin(), input.end(), compareCandidateSize);

	// Create linked grouped structure
	vector< vector< candidate* > > ol;
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
			vector< candidate* > temp;
			temp.push_back( input[i] );
			ol.push_back( temp );
		}
	}

	// Take local min overlapping maximas
	for( int i=0; i<ol.size(); i++ ) {

		// Sort entries by magnitude
		//sort( ol[i].begin(), ol[i].end(), sortByFocus ); 
		sort( ol[i].begin(), ol[i].end(), sortByMag ); 
		vector<candidate*> toadd;

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

void removeInsidePoints( vector<candidate*>& input, vector<candidate*>& output ) {
	
	// Adjust elliptical ips
	for( int i=0; i<input.size(); i++ ) {
		input[i]->minor = (input[i]->major - input[i]->minor)*0.5 + input[i]->minor;
	}

	// Sort input vector by candidate size
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

// Loads classifiers from given folder
ClassifierSystem* loadClassifiers( SystemSettings& sparams, ClassifierConfigParameters& cparams )
{
	ClassifierSystem* output = new ClassifierSystem;
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
		MainClass.Clsfr.LoadFromFile(file_rdr);
		fclose(file_rdr);

		// Set special conditions
		MainClass.IsDollar = ( cparams.L1SpecTypes[i] == SAND_DOLLAR );
		MainClass.IsScallop = ( cparams.L1SpecTypes[i] == ALL_SCALLOP );
		MainClass.IsWhite = ( cparams.L1SpecTypes[i] == WHITE_SCALLOP );
		MainClass.IsBrown = ( cparams.L1SpecTypes[i] == BROWN_SCALLOP );
		MainClass.IsBuried = ( cparams.L1SpecTypes[i] == BURIED_SCALLOP );
		
		// Set is scallop classifier flag
		if( MainClass.IsWhite || MainClass.IsBrown || MainClass.IsBuried || MainClass.IsScallop )
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
		SuppClass.Clsfr.LoadFromFile(file_rdr);
		fclose(file_rdr);

		// Set special conditions
		SuppClass.IsDollar = ( cparams.L2SpecTypes[i] == SAND_DOLLAR );
		SuppClass.IsScallop = ( cparams.L2SpecTypes[i] == ALL_SCALLOP );
		SuppClass.IsWhite = ( cparams.L2SpecTypes[i] == WHITE_SCALLOP );
		SuppClass.IsBrown = ( cparams.L2SpecTypes[i] == BROWN_SCALLOP );
		SuppClass.IsBuried = ( cparams.L2SpecTypes[i] == BURIED_SCALLOP );

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

// quick badly designed "hey its 1am and I had too much wine" inline helper func
inline void getSortedIDs( vector<int>& indices, vector<double> values )
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

bool appendInfoToFile( vector<detection*>& cds, const string& ListFilename, const string& this_fn, float resize_factor ) {
	
	// Get lock on file
	get_ListLock();
	
	// Open file and output
	ofstream fout( ListFilename.c_str(), ios::app );
	if( !fout.is_open() ) {
		cout << "ERROR: Could not open output list for writing!\n";
		return false;
	}
	
	// print out all results for every candidate and every possible classification
	for( unsigned int i=0; i<cds.size(); i++ ) {
		
		// Sort possible classifications in descending value based on classification values
		vector<int> sorted_ind;
		getSortedIDs( sorted_ind, cds[i]->ClassificationValues );
		
		// Output all possible classifications if enabled		
		for( unsigned int j=0; j<cds[i]->IDs.size(); j++ )
		{
			int cind = sorted_ind[j];
			float r = cds[i]->r / resize_factor;
			float c = cds[i]->c / resize_factor;
			float maj = cds[i]->major / resize_factor;
			float minor = cds[i]->minor / resize_factor;
			fout << this_fn << "," << r << "," << c << "," << maj << "," << minor << "," << cds[i]->angle << ",";
			fout << cds[i]->IDs[cind] << "," << cds[i]->ClassificationValues[cind] << "," << int(j+1) << endl;
		}
	}
	// so the full output string is (imagename y x major_axis minor_axis angle class class-confidence rank) 

	// Close output file
	fout.close();
	
	// Remove lock on file
	unlock_list();
	return true;
}

void cullSimilarObjects( vector<candidate*>& input, bool clams, bool dollars, bool urchins, bool sacs, bool misc ) {

	// Suppress clam responses
	if( clams ) {
		for( int ind = input.size()-1; ind >= 0; ind-- ) {
			double scallopClassMag = input[ind]->class_magnitudes[ input[ind]->classification ];
			double clamClassMag = input[ind]->class_magnitudes[ CLAM ];
			if( scallopClassMag < clamClassMag ) {
				input.erase( input.begin() + ind );
			}
		}
	}

	// Suppress dollar responses
	if( dollars ) {
		for( int ind = input.size()-1; ind >= 0; ind-- ) {
			double scallopClassMag = input[ind]->class_magnitudes[ input[ind]->classification ];
			double dollarClassMag = input[ind]->class_magnitudes[ DOLLAR ];
			if( scallopClassMag < dollarClassMag ) {
				input.erase( input.begin() + ind );
			}
		}
	}

	// Suppress urchin responses
	if( urchins ) {
		for( int ind = input.size()-1; ind >= 0; ind-- ) {
			double scallopClassMag = input[ind]->class_magnitudes[ input[ind]->classification ];
			double urchinClassMag = input[ind]->class_magnitudes[ URCHIN ];
			if( scallopClassMag < urchinClassMag ) {
				input.erase( input.begin() + ind );
			}
		}
	}

	// Suppress sac responses
	if( sacs ) {
		for( int ind = input.size()-1; ind >= 0; ind-- ) {
			double scallopClassMag = input[ind]->class_magnitudes[ input[ind]->classification ];
			double sacClassMag = input[ind]->class_magnitudes[ SAC ];
			if( scallopClassMag < sacClassMag ) {
				input.erase( input.begin() + ind );
			}
		}
	}

	// Suppress misc other responses
	if( misc ) {
		for( int ind = input.size()-1; ind >= 0; ind-- ) {
			double scallopClassMag = input[ind]->class_magnitudes[ input[ind]->classification ];
			double miscClassMag = input[ind]->class_magnitudes[ UNCLASSIFIED ];
			if( scallopClassMag < miscClassMag ) {
				input.erase( input.begin() + ind );
			}
		}
	}
}

void sandDollarSuppression( vector<candidate*>& input, bool& scallopMode ) {

	int scallop_count = 0;
	int sand_dollar_count = 0;

	// perform suppression
	for( unsigned int ind = 0; ind < input.size(); ind++ ) {

		double scallopClassMag = input[ind]->class_magnitudes[ input[ind]->classification ];
		double dollarClassMag = input[ind]->class_magnitudes[ DOLLAR ];

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

	// adjust mode based on detections
	if( scallop_count >= sand_dollar_count ) {
		scallopMode = true;
	} else {
		scallopMode = false;
	}
}

void deallocateDetections( vector<detection*>& vec )
{
	for( int i = 0; i < vec.size(); i++ )
	{
		delete vec[i];
	}
}

vector<detection*> interpolateResults( vector<candidate*>& input, ClassifierSystem* Classifiers, std::string Filename )
{
	vector<detection*> output;
	
	int MainSize = Classifiers->MainClassifiers.size();
	int SuppSize = Classifiers->SuppressionClassifiers.size();

	for( int i = 0; i < input.size(); i++ )
	{
		detection* obj = new detection;
		
		obj->r = input[i]->r;
		obj->c = input[i]->c;
		obj->angle = input[i]->angle;
		obj->major = input[i]->major;
		obj->minor = input[i]->minor;
		obj->cntr = NULL; // todo: check for and display
		
		Classifier* clfr;
		int best_class = input[i]->classification;

		if( best_class < MainSize )
			clfr = &(Classifiers->MainClassifiers[best_class]);
		else
			clfr = &(Classifiers->SuppressionClassifiers[best_class-MainSize]);

		obj->IsBrownScallop = clfr->IsScallop || clfr->IsBrown;
		obj->IsWhiteScallop = clfr->IsWhite;
		obj->IsDollar = clfr->IsDollar;
		obj->IsBuriedScallop = clfr->IsBuried;

		double best_main_class_val = -10.0;
		for( int j = 0; j < MainSize; j++ ) {
			if( input[i]->class_magnitudes[j] >= 0 ) {
				obj->IDs.push_back( Classifiers->MainClassifiers[j].ID );
				obj->ClassificationValues.push_back( input[i]->class_magnitudes[j] );
				if( input[i]->class_magnitudes[j] >= best_main_class_val )
					best_main_class_val = input[i]->class_magnitudes[j];
			}
		}
		
		for( int j = MainSize; j < MainSize + SuppSize; j++ ) {
			if( input[i]->class_magnitudes[j] >= 0 ) {
				obj->IDs.push_back( Classifiers->SuppressionClassifiers[j].ID );
				if( Classifiers->SuppressionClassifiers[j].Type != DESIRED_VS_OBJ )
					obj->ClassificationValues.push_back( input[i]->class_magnitudes[j] );
				else
					obj->ClassificationValues.push_back( best_main_class_val * (1.0 + input[i]->class_magnitudes[j] ) );
			}
		}		
		output.push_back( obj );
	}

	return output;
}
