//------------------------------------------------------------------------------
// Title: 
// Author: 
// Description: 
//------------------------------------------------------------------------------

#include "Consolidator.h"

//------------------------------------------------------------------------------
//                                 Constants
//------------------------------------------------------------------------------

const float radius_scaling_template = 0.10f;
const float radius_scaling_adaptive = 0.10f;
const float radius_scaling_colorblob = 0.10f;
const float radius_scaling_canny = 0.10f;

//------------------------------------------------------------------------------
//                            Function Prototypes
//------------------------------------------------------------------------------

bool insertTemplateIP( candidate* cd, struct kdtree* kd_tree );
bool insertAdaptiveIP( candidate* cd, struct kdtree* kd_tree );
bool insertColorBlobIP( candidate* cd, struct kdtree* kd_tree );
bool insertCannyIP( candidate* cd, struct kdtree* kd_tree );
bool compareAndMergeIPTemplate( candidate* cd1, candidate* cd2 );
bool compareAndMergeIPDoG( candidate* cd1, candidate* cd2 );
bool compareAndMergeIPAdaptive( candidate* cd1, candidate* cd2 );
bool compareAndMergeIPCanny ( candidate* cd1, candidate* cd2 );
int addStatus( const int& s1, const int& s2 );
void AddDFS( struct kdnode *ptr, CandidateQueue& Ordered );

//------------------------------------------------------------------------------
//                            Function Definitions
//------------------------------------------------------------------------------

void prioritizeCandidates( CandidateVector& Blob, 
						   CandidateVector& Adaptive,
						   CandidateVector& Template,
						   CandidateVector& Canny,
						   CandidateVector& Unordered,
						   CandidateQueue& Ordered,
						   ThreadStatistics *GS ) {

	// Create necessary structures
	struct kdtree* kd = kd_create(2);
	int c[4] = {0,0,0,0};

	// Sorts Candidate Vectors and Assigns Rankings for Prioritization
	std::sort( Blob.begin(), Blob.end(), CompareCandidates() );
	std::sort( Adaptive.begin(), Adaptive.end(), CompareCandidates() );
	std::sort( Template.begin(), Template.end(), CompareCandidates() );
	std::sort( Canny.begin(), Canny.end(), CompareCandidates() );
	for( unsigned int i = 0; i < Blob.size(); i++ )
		Blob[i]->method_rank = i;
	for( unsigned int i = 0; i < Adaptive.size(); i++ )
		Adaptive[i]->method_rank = i;
	for( unsigned int i = 0; i < Template.size(); i++ )
		Template[i]->method_rank = i;
	for( unsigned int i = 0; i < Canny.size(); i++ )
		Canny[i]->method_rank = i;

	// Insert Templated IP into kd-tree
	for( unsigned int i=0; i < Template.size(); i++ ) {
		if( insertTemplateIP( Template[i], kd ) ) {
			Unordered.push_back( Template[i] );
		} else {
			delete Template[i];
			Template[i] = NULL;
			c[0]++;
		}
	}

	// Insert Color IP into kd-tree
	for( unsigned int i=0; i < Blob.size(); i++ ) {
		if( insertColorBlobIP( Blob[i], kd ) ) {
			Unordered.push_back( Blob[i] );
		} else {
			delete Blob[i];
			Blob[i] = NULL;
			c[1]++;
		}
	}

	// Insert Adaptive IP into kd-tree
	for( unsigned int i=0; i < Adaptive.size(); i++ ) {
		if( insertAdaptiveIP( Adaptive[i], kd ) ) {
			Unordered.push_back( Adaptive[i] );
		} else {
			delete Adaptive[i];
			Adaptive[i] = NULL;
			c[2]++;
		}
	}

	// Insert Canny IP into kd-tree
	for( unsigned int i=0; i < Canny.size(); i++ ) {
		if( insertCannyIP( Canny[i], kd ) ) {
			Unordered.push_back( Canny[i] );
		} else {
			delete Canny[i];
			Canny[i] = NULL;
			c[3]++;
		}
	}

	//cout << c[0] << " " << c[1] << " " << c[2] << " " << c[3] << endl;

	// Formulate priority queue
	struct kdnode *ptr = kd->root;
	AddDFS( ptr, Ordered );

	// Deallocations
	kd_free( kd );
}

void AddDFS( struct kdnode *ptr, CandidateQueue& Ordered ) {

	if( ptr == NULL )
		return;

	Ordered.push( (candidate*) ptr->data );
	AddDFS( ptr->left, Ordered );
	AddDFS( ptr->right, Ordered );
}

// Inserts a template IP into the KD-tree
bool insertTemplateIP( candidate* cd, struct kdtree* kd_tree ) {
	// Check if there are any nearby neighbors
	float range = radius_scaling_template * cd->major;
	struct kdres *set = kd_nearest_range2f(kd_tree, cd->r, cd->c, range);
	// If not add ip
	if( set->size == 0 ) {
		kd_insert2f( kd_tree, cd->r, cd->c, (void*)cd );
		kd_res_free(set);
		return true;
	} 
	// Check nearest neighbors in set for conflicts
	struct res_node *ptr = set->riter;
	while( ptr != NULL ) {
		candidate *nearby = (candidate*)ptr->item->data;
		if( compareAndMergeIPTemplate( cd, nearby ) ) {
			kd_res_free(set);
			return false;
		}
		ptr = ptr->next;
	}
	kd_insert2f( kd_tree, cd->r, cd->c, (void*)cd );
	kd_res_free(set);
	return true;
}

// Inserts a Blob IP into the KD-tree
bool insertColorBlobIP( candidate* cd, struct kdtree* kd_tree ) {
	float range = radius_scaling_colorblob * cd->major;
	struct kdres *set = kd_nearest_range2f(kd_tree, cd->r, cd->c, range);
	// If not add ip
	if( set->size == 0 ) {
		kd_insert2f( kd_tree, cd->r, cd->c, (void*)cd );
		kd_res_free(set);
		return true;
	} 
	// Check nearest neighbors in set for conflicts
	struct res_node *ptr = set->riter;
	while( ptr != NULL ) {
		candidate *nearby = (candidate*)ptr->item->data;
		if( compareAndMergeIPDoG( cd, nearby ) ) {
			kd_res_free(set);
			return false;
		}
		ptr = ptr->next;
	}
	kd_insert2f( kd_tree, cd->r, cd->c, (void*)cd );
	kd_res_free(set);
	return true;
}

// Inserts a Color IP into the KD-tree
bool insertAdaptiveIP( candidate* cd, struct kdtree* kd_tree ) {
	float range = radius_scaling_adaptive * cd->major;
	struct kdres *set = kd_nearest_range2f(kd_tree, cd->r, cd->c, range);
	// If not add ip
	if( set->size == 0 ) {
		kd_insert2f( kd_tree, cd->r, cd->c, (void*)cd );
		kd_res_free(set);
		return true;
	} 
	// Check nearest neighbors in set for conflicts
	struct res_node *ptr = set->riter;
	while( ptr != NULL ) {
		candidate *nearby = (candidate*)ptr->item->data;
		if( compareAndMergeIPAdaptive( cd, nearby ) ) {
			kd_res_free(set);
			return false;
		}
		ptr = ptr->next;
	}
	kd_insert2f( kd_tree, cd->r, cd->c, (void*)cd );
	kd_res_free(set);
	return true;
}

// Inserts a Canny IP into the KD-tree
bool insertCannyIP( candidate* cd, struct kdtree* kd_tree ) {
	float range = radius_scaling_canny * cd->major;
	struct kdres *set = kd_nearest_range2f(kd_tree, cd->r, cd->c, range);
	// If not add ip
	if( set->size == 0 ) {
		kd_insert2f( kd_tree, cd->r, cd->c, (void*)cd );
		kd_res_free(set);
		return true;
	} 
	// Check nearest neighbors in set for conflicts
	struct res_node *ptr = set->riter;
	while( ptr != NULL ) {
		candidate *nearby = (candidate*)ptr->item->data;
		if( compareAndMergeIPCanny( cd, nearby ) ) {
			kd_res_free(set);
			return false;
		}
		ptr = ptr->next;
	}
	kd_insert2f( kd_tree, cd->r, cd->c, (void*)cd );
	kd_res_free(set);
	return true;
}

// Returns true if candidates mergerd, false if they are different
bool compareAndMergeIPTemplate( candidate* cd1, candidate* cd2 ) {

	// Calculate difference in angle
	float angle_dif = abs( cd1->angle - cd2->angle );
	angle_dif = min( angle_dif, 360.0f - angle_dif );
	float maj_scl_dif = cd1->major / cd2->major;
	
	// Calculate differences in major axis scale
	bool cd1_maj_larger = true;
	if( maj_scl_dif < 1.0f ) {
		maj_scl_dif = 1 / maj_scl_dif;
		cd1_maj_larger = false;
	}

	// Calculate differences in minor axis scale
	bool cd1_min_larger = true;
	float min_scl_dif = cd1->minor / cd2->minor;
	if( min_scl_dif < 1.0f ) {
		min_scl_dif = 1 / min_scl_dif;
		cd1_min_larger = false;
	}

	// Calculate differences in location
	float rdif = abs(cd1->r-cd2->r);
	float cdif = abs(cd1->c-cd2->c);
	float distsq = rdif*rdif + cdif*cdif;
	float ndist = sqrt( distsq );

	// Immediate Rejection Parameters
	if( maj_scl_dif > 2 || min_scl_dif > 2 )
		return false;

	if( angle_dif > 25 )
		return false;

	// Calculate Simularity Factor
	/*float dif_factor = avg_scl_dif - 1.0f;
	if( ndist > 0.15 ) 
		dif_factor = dif_factor + 1;
	
	// Merge if necessary, return true if merged
	if( dif_factor < 0.15 ) {
		float cd2_favor = 0.5;
		cd2->angle = ((1-cd2_favor)*cd1->angle + cd2_favor*cd2->angle)/2;
		cd2->c = ((1-cd2_favor)*cd1->c + cd2_favor*cd2->c)/2;
		cd2->r = ((1-cd2_favor)*cd1->r + cd2_favor*cd2->r)/2;
		cd2->major = ((1-cd2_favor)*cd1->major + cd2_favor*cd2->major)/2;
		cd2->minor = ((1-cd2_favor)*cd1->minor + cd2_favor*cd2->minor)/2;
		cd2->magnitude = max( cd1->magnitude, cd2->magnitude );
		cd2->method = addStatus( cd1->method, cd2->method );
		return true;
	}*/

	// For now, just simple thresholding on radial difference
	if( maj_scl_dif < 1.11 && min_scl_dif < 1.20 ) {
		float cd2_favor = 0.5;
		cd2->angle = ((1-cd2_favor)*cd1->angle + cd2_favor*cd2->angle);
		cd2->c = ((1-cd2_favor)*cd1->c + cd2_favor*cd2->c);
		cd2->r = ((1-cd2_favor)*cd1->r + cd2_favor*cd2->r);
		cd2->major = ((1-cd2_favor)*cd1->major + cd2_favor*cd2->major);
		cd2->minor = ((1-cd2_favor)*cd1->minor + cd2_favor*cd2->minor);
		cd2->magnitude = max( cd1->magnitude, cd2->magnitude );
		cd2->method = addStatus( cd1->method, cd2->method );
		return true;
	}
	
	return false;
}

bool compareAndMergeIPDoG( candidate* cd1, candidate* cd2 ) {

	// Calculate difference in angle
	float angle_dif = abs( cd1->angle - cd2->angle );
	angle_dif = min( angle_dif, 360.0f - angle_dif );
	float maj_scl_dif = cd1->major / cd2->major;
	
	// Calculate differences in major axis scale
	bool cd1_maj_larger = true;
	if( maj_scl_dif < 1.0f ) {
		maj_scl_dif = 1 / maj_scl_dif;
		cd1_maj_larger = false;
	}

	// Calculate differences in minor axis scale
	bool cd1_min_larger = true;
	float min_scl_dif = cd1->minor / cd2->minor;
	if( min_scl_dif < 1.0f ) {
		min_scl_dif = 1 / min_scl_dif;
		cd1_min_larger = false;
	}

	// Calculate differences in location
	float rdif = abs(cd1->r-cd2->r);
	float cdif = abs(cd1->c-cd2->c);
	float distsq = rdif*rdif + cdif*cdif;
	float ndist = sqrt( distsq );

	// Immediate Rejection Parameters
	if( maj_scl_dif > 2 || min_scl_dif > 2 )
		return false;

	if( angle_dif > 25 )
		return false;

	// For now, just simple thresholding on radial difference
	if( maj_scl_dif < 1.11 && min_scl_dif < 1.20 ) {
		float cd2_favor = 0.5;
		cd2->angle = ((1-cd2_favor)*cd1->angle + cd2_favor*cd2->angle);
		cd2->c = ((1-cd2_favor)*cd1->c + cd2_favor*cd2->c);
		cd2->r = ((1-cd2_favor)*cd1->r + cd2_favor*cd2->r);
		cd2->major = ((1-cd2_favor)*cd1->major + cd2_favor*cd2->major);
		cd2->minor = ((1-cd2_favor)*cd1->minor + cd2_favor*cd2->minor);
		cd2->magnitude = max( cd1->magnitude, cd2->magnitude );
		cd2->method = addStatus( cd1->method, cd2->method );
		return true;
	}
	
	return false;
}

bool compareAndMergeIPAdaptive( candidate* cd1, candidate* cd2 ) {

	// Calculate difference in angle
	float angle_dif = abs( cd1->angle - cd2->angle );
	angle_dif = min( angle_dif, 360.0f - angle_dif );
	float maj_scl_dif = cd1->major / cd2->major;
	
	// Calculate differences in major axis scale
	bool cd1_maj_larger = true;
	if( maj_scl_dif < 1.0f ) {
		maj_scl_dif = 1 / maj_scl_dif;
		cd1_maj_larger = false;
	}

	// Calculate differences in minor axis scale
	bool cd1_min_larger = true;
	float min_scl_dif = cd1->minor / cd2->minor;
	if( min_scl_dif < 1.0f ) {
		min_scl_dif = 1 / min_scl_dif;
		cd1_min_larger = false;
	}

	// Calculate differences in location
	float rdif = abs(cd1->r-cd2->r);
	float cdif = abs(cd1->c-cd2->c);
	float distsq = rdif*rdif + cdif*cdif;
	float ndist = sqrt( distsq );

	// Immediate Rejection Parameters
	if( maj_scl_dif > 2 || min_scl_dif > 2 )
		return false;

	if( angle_dif > 25 )
		return false;

	// For now, just simple thresholding on radial difference
	if( maj_scl_dif < 1.11 && min_scl_dif < 1.20 ) {
		float cd2_favor = 0.5;
		cd2->angle = ((1-cd2_favor)*cd1->angle + cd2_favor*cd2->angle);
		cd2->c = ((1-cd2_favor)*cd1->c + cd2_favor*cd2->c);
		cd2->r = ((1-cd2_favor)*cd1->r + cd2_favor*cd2->r);
		cd2->major = ((1-cd2_favor)*cd1->major + cd2_favor*cd2->major);
		cd2->minor = ((1-cd2_favor)*cd1->minor + cd2_favor*cd2->minor);
		cd2->magnitude = max( cd1->magnitude, cd2->magnitude );
		cd2->method = addStatus( cd1->method, cd2->method );
		return true;
	}
	
	return false;
}

bool compareAndMergeIPCanny( candidate* cd1, candidate* cd2 ) {

		// Calculate difference in angle
	float angle_dif = abs( cd1->angle - cd2->angle );
	angle_dif = min( angle_dif, 360.0f - angle_dif );
	float maj_scl_dif = cd1->major / cd2->major;
	
	// Calculate differences in major axis scale
	bool cd1_maj_larger = true;
	if( maj_scl_dif < 1.0f ) {
		maj_scl_dif = 1 / maj_scl_dif;
		cd1_maj_larger = false;
	}

	// Calculate differences in minor axis scale
	bool cd1_min_larger = true;
	float min_scl_dif = cd1->minor / cd2->minor;
	if( min_scl_dif < 1.0f ) {
		min_scl_dif = 1 / min_scl_dif;
		cd1_min_larger = false;
	}

	// Calculate differences in location
	float rdif = abs(cd1->r-cd2->r);
	float cdif = abs(cd1->c-cd2->c);
	float distsq = rdif*rdif + cdif*cdif;
	float ndist = sqrt( distsq );

	// Immediate Rejection Parameters
	if( maj_scl_dif > 2 || min_scl_dif > 2 )
		return false;

	if( angle_dif > 25 )
		return false;

	// For now, just simple thresholding on radial difference
	if( maj_scl_dif < 1.11 && min_scl_dif < 1.20 ) {
		float cd2_favor = 0.5;
		cd2->angle = ((1-cd2_favor)*cd1->angle + cd2_favor*cd2->angle);
		cd2->c = ((1-cd2_favor)*cd1->c + cd2_favor*cd2->c);
		cd2->r = ((1-cd2_favor)*cd1->r + cd2_favor*cd2->r);
		cd2->major = ((1-cd2_favor)*cd1->major + cd2_favor*cd2->major);
		cd2->minor = ((1-cd2_favor)*cd1->minor + cd2_favor*cd2->minor);
		cd2->magnitude = max( cd1->magnitude, cd2->magnitude );
		cd2->method = addStatus( cd1->method, cd2->method );
		return true;
	}
	
	return false;
}

int addStatus( const int& s1, const int& s2 ) {

/*
const unsigned int TEMPLATE = 0;
const unsigned int DOG = 1;
const unsigned int ADAPTIVE = 2;
const unsigned int CANNY = 3;
const unsigned int MULTIPLE1 = 4;
const unsigned int MULTIPLE2 = 5;
const unsigned int MULTIPLE3 = 6;
const unsigned int TOTAL_DM = 7;*/

	if( s1 == TEMPLATE ) {
		if( s2 == TEMPLATE ) 
			return TEMPLATE;
		else if( s2 == DOG )
			return MULTIPLE1;
		else if( s2 == ADAPTIVE )
			return MULTIPLE2;
		else if( s2 == CANNY )
			return MULTIPLE1;
		else if( s2 == MULTIPLE1 )
			return MULTIPLE2;
		else if( s2 == MULTIPLE2 ) 
			return MULTIPLE3;
		else
			return MULTIPLE3;
	} else if( s1 == DOG ) {
		if( s2 == TEMPLATE ) 
			return MULTIPLE2;
		else if( s2 == DOG )
			return DOG;
		else if( s2 == ADAPTIVE )
			return MULTIPLE1;
		else if( s2 == CANNY )
			return MULTIPLE2;
		else if( s2 == MULTIPLE1 )
			return MULTIPLE2;
		else if( s2 == MULTIPLE2 ) 
			return MULTIPLE3;
		else
			return MULTIPLE3;
	} else if( s1 == ADAPTIVE ) {
		if( s2 == TEMPLATE ) 
			return MULTIPLE2;
		else if( s2 == DOG )
			return MULTIPLE1;
		else if( s2 == ADAPTIVE )
			return ADAPTIVE;
		else if( s2 == CANNY )
			return MULTIPLE2;
		else if( s2 == MULTIPLE1 )
			return MULTIPLE2;
		else if( s2 == MULTIPLE2 ) 
			return MULTIPLE3;
		else
			return MULTIPLE3;
	} else if( s1 == CANNY ) {
		if( s2 == TEMPLATE ) 
			return MULTIPLE1;
		else if( s2 == DOG )
			return MULTIPLE2;
		else if( s2 == ADAPTIVE )
			return MULTIPLE2;
		else if( s2 == CANNY )
			return CANNY;
		else if( s2 == MULTIPLE1 )
			return MULTIPLE2;
		else if( s2 == MULTIPLE2 ) 
			return MULTIPLE3;
		else
			return MULTIPLE3;
	} else if( s1 == MULTIPLE1 ) {
		return MULTIPLE2;
	} else {
		return MULTIPLE3;
	}
	cerr << "ERROR! Undefined status used in Consolidator.h::addStatus\n";
	return s1;
}