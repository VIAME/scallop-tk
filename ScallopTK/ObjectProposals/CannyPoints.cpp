//------------------------------------------------------------------------------
// Title:
// Author:
// Description: 
//------------------------------------------------------------------------------

#include "CannyPoints.h"

//------------------------------------------------------------------------------
//                                Structures
//------------------------------------------------------------------------------

struct cp_circle {

	// Circle properties
	double r, c;
	double rad;

	// Contour props which identified circle
	int csize;
	double normMSE;	
	double coverage;
};

struct cp_point {
	cp_point( const int& _r, const int& _c ) : r(_r), c(_c) {}
	int r, c;
};

//------------------------------------------------------------------------------
//                            Function Definitions
//------------------------------------------------------------------------------

// (a,b) (c,d) (e,f) -- (r,c)
bool circleFrom3Points( const float &a, const float &b,  
						const float &c, const float &d,
						const float &e, const float &f, 
						cp_circle& cir ) {

	float sqr[6];
	sqr[0] = a * a;
	sqr[1] = b * b;
	sqr[2] = c * c;
	sqr[3] = d * d;
	sqr[4] = e * e;
	sqr[5] = f * f;

	float addA = sqr[0] + sqr[1];
	float addB = sqr[2] + sqr[3];
	float addC = sqr[4] + sqr[5];
	float lower1 = ( a * (f-d ) + c * ( b - f ) + e * ( d - b ) );
	float lower2 = ( b * (e-c ) + d * ( a - e ) + f * ( c - a ) );

	if( lower1 == 0 || lower2 == 0 ) // straight line
		return false;

	cir.r = (addA)*(f-d)+(addB)*(b-f)+(addC)*(d-b);
	cir.r = 0.5 * cir.r / lower1;

	cir.c = (addA)*(e-c)+(addB)*(a-e)+(addC)*(c-a);
	cir.c = 0.5 * cir.c / lower2;

	cir.rad = sqrt( (a-cir.r)*(a-cir.r) + (b-cir.c)*(b-cir.c) );
	return true;
}

bool circleFrom3PointsInd( vector<cp_point>& cntr, int i1, int i2, int i3, cp_circle& cir ) {
	return circleFrom3Points( cntr[i1].r, cntr[i1].c, cntr[i2].r, cntr[i2].c, cntr[i3].r, cntr[i3].c, cir );
}

cp_circle circleFromNPoints( float *p, int n ) {

	cp_circle cir;

	//Magic

	return cir;
}

/*bool compareCircles( const cp_circle& cd1, const cp_circle& cd2  ) {
	if( (double)cd1.csize/cd1.normMSE > (double)cd2.csize/cd2.normMSE ) 
		return true;
	return false;
}*/

bool compareCircles( const cp_circle& cd1, const cp_circle& cd2  ) {
	if( cd1.coverage/cd1.normMSE > cd2.coverage/cd2.normMSE ) 
		return true;
	return false;
}

void calcMSE( cp_circle& cir, vector<cp_point>& pts ) {
	double MSE = 0.0;
	for( unsigned int i=0; i<pts.size(); i++ ) {
		double r = (pts[i].r - cir.r)/cir.rad;
		double c = (pts[i].c - cir.c)/cir.rad;
		r = r*r;
		c = c*c;
		r = sqrt( r+c );
		c = r-1;
		r = c*c;
		MSE += r;
	}
	cir.normMSE = MSE / pts.size();
}

void calcCoverage( cp_circle& cir, vector<cp_point>& pts ) {
	cir.csize = pts.size();
	cir.coverage = (double)cir.csize*cir.csize / cir.rad;
}

void findCannyCandidates( GradientChain& grad, std::vector<Candidate*>& cds ) {

	// Read canny edge stats from gradient image
	IplImage *canny = grad.cannyEdges;
	int width = canny->width;
	int height = canny->height;
	int wstep = canny->widthStep;
	float minRad = grad.minRad;
	float maxRad = grad.maxRad;

	// Single pass through image (sort of) linking edges
	vector<cp_circle> iden;
	const char EDGE_MARKER = (char)(255);
	const char USED_MARKER = (char)(254);
	//cout << width << " " << height << endl;
	for( int r = 1; r < height-1; r++ ) {
		for( int c = 1; c < width-1; c++ ) {

			// Check if new contour exists
			if( (canny->imageData+wstep*r)[c] == EDGE_MARKER ) {

				stack<cp_point> sq;
				cp_point pt(r,c);
				vector<cp_point> seq;
				sq.push( pt );

				while( sq.size() != 0 ) {

					pt = sq.top();
					int ir = pt.r;
					int ic = pt.c;
					
					// Check boundary conditions
					if( ir < 1 || ic < 1 || ir > height-1 || ic > width-1 ) {
						sq.pop();
						continue;
					}

					// Check label
					char* pos = (canny->imageData + wstep*ir)+ic;
					if( *pos != EDGE_MARKER ) {
						sq.pop();
						continue;
					}
					
					*pos = USED_MARKER;
					seq.push_back( pt );
					sq.pop();

					// Check 8-connectedness
					sq.push( cp_point( ir+1, ic ) );
					sq.push( cp_point( ir, ic+1 ) );
					sq.push( cp_point( ir-1, ic ) );
					sq.push( cp_point( ir, ic-1 ) );
					sq.push( cp_point( ir-1, ic+1 ) );
					sq.push( cp_point( ir-1, ic-1 ) );
					sq.push( cp_point( ir+1, ic+1 ) );
					sq.push( cp_point( ir+1, ic-1 ) );
				}	

				// Quickly Analyze Contour
				int csize = seq.size();

				// Threshold size
				if( csize < 8 )
					continue;

				// If its a very small contour
				if( csize < 16 ) {
					int i1 = 1;
					int i2 = csize / 2;
					int i3 = csize - 2;
					cp_circle c1;
					bool status = circleFrom3PointsInd( seq, i1, i2, i3, c1 );
					if( status && c1.rad > (1.5*minRad) && c1.rad < maxRad ) {
						calcMSE( c1, seq );
						calcCoverage( c1, seq );
						iden.push_back( c1 );
					}
					continue;
				}

				// Sampling Indecies
				int i1 = 0;
				int i2 = 0.2*csize;
				int i3 = 0.4*csize;
				int i4 = 0.6*csize;
				int i5 = 0.8*csize;
				int i6 = csize - 1;
				
				// Sample circles
				cp_circle c1, c2, c3;
				bool s1 = circleFrom3PointsInd( seq, i1, i4, i6, c1 );
				bool s2 = circleFrom3PointsInd( seq, i2, i4, i6, c2 );
				bool s3 = circleFrom3PointsInd( seq, i1, i3, i5, c3 );

				// Filter
				if( s1 && c1.rad > minRad && c1.rad < maxRad ) {
					calcMSE( c1, seq );
					calcCoverage( c1, seq );
					iden.push_back( c1 );
				}
				if( s2 && c2.rad > minRad && c2.rad < maxRad ) {
					calcMSE( c2, seq );
					calcCoverage( c2, seq );
					iden.push_back( c2 );
				}
				if( s3 && c3.rad > minRad && c3.rad < maxRad ) {
					calcMSE( c3, seq );
					calcCoverage( c3, seq );
					iden.push_back( c3 );
				}	
			}
		}
	}

	// Insert/Adjust IP for scale
	const int MAX_CP_IP = 40;
	sort( iden.begin(), iden.end(), compareCircles );
	for( unsigned int i = 0; i < iden.size(); i++ ) {

		// Check max
		if( i >= MAX_CP_IP )
			break;

		// Insert new Candidate point
		Candidate* cd1 = new Candidate;
		cd1->r = iden[i].r * grad.scale;
		cd1->c = iden[i].c * grad.scale;
		cd1->major = iden[i].rad * grad.scale;
		cd1->minor = iden[i].rad * grad.scale;
		cd1->angle = 0.0;
		cd1->method = CANNY;
		cd1->magnitude = iden[i].normMSE*iden[i].csize;
		cds.push_back( cd1 );
	}
}
