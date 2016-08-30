/* simple utility to print ground rect of camera
hardwired to 2/3" CCD

arguments

heading degrees
pitch degrees
roll  degrees
altitude meters
imageWidth pixels
imageHeight pixels
FocalLength meters

output:

PP = Point of PinHole
CCD CornerPositions   Ground Intersection Points

./scott_camera 0 0 0 2.4 1320 1024 0.017
0.0000 heading
0.0000 pitch
0.0000 roll
2.4000 altitude
0.0169 width
0.0131 height
0.0170 focalLength
PP =   0.0000   0.0000   2.4000
-0.0085   0.0066   2.4170  =>    1.1953  -0.9273   0.0000
-0.0085  -0.0066   2.4170  =>    1.1953   0.9273   0.0000
0.0085   0.0066   2.4170  =>   -1.1953  -0.9273   0.0000
0.0085  -0.0066   2.4170  =>   -1.1953   0.9273   0.0000
Area is   4.4334

Norman Vine nhv@cape.com  March 11 2010

*/

#include "ScottCamera.h"


inline SGfloat sgSin ( SGfloat s )
{ return (SGfloat) sin ( s * SG_DEGREES_TO_RADIANS ) ; }

inline SGfloat sgCos ( SGfloat s )
{ return (SGfloat) cos ( s * SG_DEGREES_TO_RADIANS ) ; }

inline SGfloat sgSqrt   ( const SGfloat x ) { return (SGfloat) sqrt ( x ) ; }

inline SGfloat sgAbs    ( const SGfloat a ) { return (a<SG_ZERO) ? -a : a ; }

inline void sgZeroVec3 ( sgVec3 dst ) { dst[0]=dst[1]=dst[2]=SG_ZERO ; }

inline void sgSetVec3 ( sgVec3 dst, const SGfloat x, const SGfloat y, const SGfloat z )
{
	dst [ 0 ] = x ;
	dst [ 1 ] = y ;
	dst [ 2 ] = z ;
}

inline void sgScaleVec3 ( sgVec3 dst, const SGfloat s )
{
	dst [ 0 ] *= s ;
	dst [ 1 ] *= s ;
	dst [ 2 ] *= s ;
}

inline void sgScaleVec3 ( sgVec3 dst, const sgVec3 src, const SGfloat s )
{
	dst [ 0 ] = src [ 0 ] * s ;
	dst [ 1 ] = src [ 1 ] * s ;
	dst [ 2 ] = src [ 2 ] * s ;
}

inline void sgSubVec3 ( sgVec3 dst, const sgVec3 src1, const sgVec3 src2 )
{
	dst [ 0 ] = src1 [ 0 ] - src2 [ 0 ] ;
	dst [ 1 ] = src1 [ 1 ] - src2 [ 1 ] ;
	dst [ 2 ] = src1 [ 2 ] - src2 [ 2 ] ;
}

inline void sgAddVec3 ( sgVec3 dst, const sgVec3 src1, const sgVec3 src2 )
{
	dst [ 0 ] = src1 [ 0 ] + src2 [ 0 ] ;
	dst [ 1 ] = src1 [ 1 ] + src2 [ 1 ] ;
	dst [ 2 ] = src1 [ 2 ] + src2 [ 2 ] ;
}

inline SGfloat sgScalarProductVec3 ( const sgVec3 a, const sgVec3 b )
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] ;
}

void sgVectorProductVec3 ( sgVec3 dst, const sgVec3 a, const sgVec3 b )
{
	dst[0] = a[1] * b[2] - a[2] * b[1] ;
	dst[1] = a[2] * b[0] - a[0] * b[2] ;
	dst[2] = a[0] * b[1] - a[1] * b[0] ;
}

inline SGfloat sgLengthVec3 ( const sgVec3 src )
{
	return sgSqrt ( sgScalarProductVec3 ( src, src ) ) ;
}

inline void sgNormaliseVec3 ( sgVec3 dst )
{
	sgScaleVec3 ( dst, SG_ONE / sgLengthVec3 ( dst ) ) ;
}


void sgMakeNormal(sgVec3 dst, const sgVec3 a, const sgVec3 b, const sgVec3 c )
{
	sgVec3 ab ; sgSubVec3 ( ab, b, a ) ;
	sgVec3 ac ; sgSubVec3 ( ac, c, a ) ;
	sgVectorProductVec3 ( dst, ab,ac ) ; sgNormaliseVec3 ( dst ) ;
}


inline void sgMakePlane ( sgVec4 dst, const sgVec3 a, const sgVec3 b, const sgVec3 c )
{
	/*
	Ax + By + Cz + D == 0 ;
	D = - ( Ax + By + Cz )
	= - ( A*a[0] + B*a[1] + C*a[2] )
	= - sgScalarProductVec3 ( normal, a ) ;
	*/

	sgMakeNormal ( dst, a, b, c ) ;

	dst [ 3 ] = - sgScalarProductVec3 ( dst, a ) ;
}


SGfloat sgIsectLinesegPlane ( sgVec3 dst, sgVec3 v1, sgVec3 v2, sgVec4 plane )
{
	sgVec3 delta ;

	sgSubVec3 ( delta, v2, v1 ) ;

	SGfloat p = sgScalarProductVec3 ( plane, delta ) ;

	if ( p == SG_ZERO )
	{
		dst [ 0 ] = dst [ 1 ] = dst [ 2 ] = FLT_MAX ;
		return FLT_MAX ;
	}

	float s = - ( sgScalarProductVec3 ( plane, v1 ) + plane[3] ) / p ;

	sgScaleVec3 ( dst, delta, s ) ;
	sgAddVec3   ( dst, dst, v1 ) ;

	return s ;
}


void sgMakeCoordMat4 ( sgMat4 m, const SGfloat x, const SGfloat y,
  const SGfloat z, const SGfloat h, const SGfloat p, const SGfloat r )
{
	SGfloat ch, sh, cp, sp, cr, sr, srsp, crsp, srcp ;

	if ( h == SG_ZERO )
	{
		ch = SG_ONE ;
		sh = SG_ZERO ;
	}
	else
	{
		sh = sgSin ( h ) ;
		ch = sgCos ( h ) ;
	}

	if ( p == SG_ZERO )
	{
		cp = SG_ONE ;
		sp = SG_ZERO ;
	}
	else
	{
		sp = sgSin ( p ) ;
		cp = sgCos ( p ) ;
	}

	if ( r == SG_ZERO )
	{
		cr   = SG_ONE ;
		sr   = SG_ZERO ;
		srsp = SG_ZERO ;
		srcp = SG_ZERO ;
		crsp = sp ;
	}
	else
	{
		sr   = sgSin ( r ) ;
		cr   = sgCos ( r ) ;
		srsp = sr * sp ;
		crsp = cr * sp ;
		srcp = sr * cp ;
	}

	m[0][0] = (SGfloat)(  ch * cr - sh * srsp ) ;
	m[1][0] = (SGfloat)( -sh * cp ) ;
	m[2][0] = (SGfloat)(  sr * ch + sh * crsp ) ;
	m[3][0] =  x ;

	m[0][1] = (SGfloat)( cr * sh + srsp * ch ) ;
	m[1][1] = (SGfloat)( ch * cp ) ;
	m[2][1] = (SGfloat)( sr * sh - crsp * ch ) ;
	m[3][1] =  y ;

	m[0][2] = (SGfloat)( -srcp ) ;
	m[1][2] = (SGfloat)(  sp ) ;
	m[2][2] = (SGfloat)(  cr * cp ) ;
	m[3][2] =  z ;

	m[0][3] =  SG_ZERO ;
	m[1][3] =  SG_ZERO ;
	m[2][3] =  SG_ZERO ;
	m[3][3] =  SG_ONE ;
}

void sgXformPnt3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat )
{
	SGfloat t0 = src[ 0 ] ;
	SGfloat t1 = src[ 1 ] ;
	SGfloat t2 = src[ 2 ] ;

	dst[0] = t0 * mat[ 0 ][ 0 ] +
		t1 * mat[ 1 ][ 0 ] +
		t2 * mat[ 2 ][ 0 ] +
		mat[ 3 ][ 0 ] ;

	dst[1] = t0 * mat[ 0 ][ 1 ] +
		t1 * mat[ 1 ][ 1 ] +
		t2 * mat[ 2 ][ 1 ] +
		mat[ 3 ][ 1 ] ;

	dst[2] = t0 * mat[ 0 ][ 2 ] +
		t1 * mat[ 1 ][ 2 ] +
		t2 * mat[ 2 ][ 2 ] +
		mat[ 3 ][ 2 ] ;
}

void sgXformPnt4 ( sgVec4 dst, const sgVec4 src, const sgMat4 mat )
{
	SGfloat t0 = src[ 0 ] ;
	SGfloat t1 = src[ 1 ] ;
	SGfloat t2 = src[ 2 ] ;
	SGfloat t3 = src[ 3 ] ;

	dst[0] = t0 * mat[ 0 ][ 0 ] +
		t1 * mat[ 1 ][ 0 ] +
		t2 * mat[ 2 ][ 0 ] +
		t3 * mat[ 3 ][ 0 ] ;

	dst[1] = t0 * mat[ 0 ][ 1 ] +
		t1 * mat[ 1 ][ 1 ] +
		t2 * mat[ 2 ][ 1 ] +
		t3 * mat[ 3 ][ 1 ] ;

	dst[2] = t0 * mat[ 0 ][ 2 ] +
		t1 * mat[ 1 ][ 2 ] +
		t2 * mat[ 2 ][ 2 ] +
		t3 * mat[ 3 ][ 2 ] ;

	dst[3] = t0 * mat[ 0 ][ 3 ] +
		t1 * mat[ 1 ][ 3 ] +
		t2 * mat[ 2 ][ 3 ] +
		t3 * mat[ 3 ][ 3 ] ;
}



void sgFullXformPnt3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat )
{
	sgVec4 tmp ;

	tmp [ 0 ] = src [ 0 ] ;
	tmp [ 1 ] = src [ 1 ] ;
	tmp [ 2 ] = src [ 2 ] ;
	tmp [ 3 ] =   SG_ONE  ;

	sgXformPnt4 ( tmp, tmp, mat ) ;
	sgScaleVec3 ( dst, tmp, SG_ONE / tmp [ 3 ] ) ;
}

float  myIsectLinesegPlane ( sgVec3 dst, sgVec3 v1, sgVec3 v2, sgVec4 plane )
{
	float result;

#ifdef DISPLAY_CAMINFO
	fprintf(stdout,"     %8.4f %8.4f %8.4f",v1[0], v1[1],v1[2]);
#endif

	result = sgIsectLinesegPlane ( dst, v1, v2, plane );

#ifdef DISPLAY_CAMINFO
	if( result == FLT_MAX  )
		fprintf(stdout,"     IsectLinePlane  --  No Intersection Found\n");
	else
		fprintf(stdout,"  =>  %8.4f %8.4f %8.4f\n",dst[0], dst[1],dst[2]);
#endif

	return result;

}

float sgTriArea( sgVec3 p0, sgVec3 p1, sgVec3 p2 )
{
	sgVec3 sum;
	sgZeroVec3( sum );

	sgVec3 norm;
	sgMakeNormal( norm, p0, p1, p2 );

	float *vv[3];
	vv[0] = p0;
	vv[1] = p1;
	vv[2] = p2;

	for( int i=0; i<3; i++ )
	{
		int ii = (i+1) % 3;

		sum[0] += (vv[i][1] * vv[ii][2] - vv[i][2] * vv[ii][1]) ;
		sum[1] += (vv[i][2] * vv[ii][0] - vv[i][0] * vv[ii][2]) ;
		sum[2] += (vv[i][0] * vv[ii][1] - vv[i][1] * vv[ii][0]) ;
	}

	float area = sgAbs ( sgScalarProductVec3 ( norm, sum ) ) ;

	return area / 2.0f ;
}


int calculateDimensions(float heading,
	float pitch,
	float roll,
	float altitude,
	float width,
	float height,
	float focalLength,
	float &area,
	float &rHeight,
	float &rWidth)
{
	//sgVec3 hpr;
	sgVec4 plane;
	sgMat4 trans;

#define inch2meters 0.0254

	float chip_size = (2.0/3.0)*float(inch2meters);

	height = chip_size * height / width;
	width = chip_size;

#ifdef DISPLAY_CAMINFO
	fprintf(stdout,"\n     %8.4f heading\n",heading);
	fprintf(stdout,"     %8.4f pitch\n",pitch);
	fprintf(stdout,"     %8.4f roll\n",roll);
	fprintf(stdout,"     %8.4f altitude\n",altitude);
	fprintf(stdout,"     %8.4f width\n",width);
	fprintf(stdout,"     %8.4f height\n",height);
	fprintf(stdout,"     %8.4f focalLength\n",focalLength);
#endif

	//float pixel_size = chip_size/width;


	///  Principal Point and ImagePlane;
	sgVec3 PP,UL,LL,UR,LR;

	sgSetVec3(PP,0,0,0.0);

	sgSetVec3(UL, -width/2.0f, height/2.0f, focalLength);
	sgSetVec3(LL, -width/2.0f, -height/2.0f, focalLength );
	sgSetVec3(UR, width/2.0f, height/2.0f, focalLength);
	sgSetVec3(LR, width/2.0f, -height/2.0f, focalLength);

	sgVec3 A,B,C;
	sgSetVec3(A,1,0,0);
	sgSetVec3(B,1,1,0);
	sgSetVec3(C,0,1,0);

	sgMakePlane(plane,A,B,C);
	sgMakeCoordMat4(trans, 0.0,0.0,altitude,heading,pitch,roll);

#define myXformPnt3 sgXformPnt3

	//sgVec3 P0,P1,P2,P3,P4;
	sgVec3 P1,P2,P3,P4;
	myXformPnt3 ( PP, PP, trans );

#ifdef DISPLAY_CAMINFO
	fprintf(stdout,"     PP = %8.4f %8.4f %8.4f\n",PP[0],PP[1],PP[2]);
#endif

	myXformPnt3 ( UL, UL, trans );
	myXformPnt3 ( LL, LL, trans );
	myXformPnt3 ( UR, UR, trans );
	myXformPnt3 ( LR, LR, trans );

	myIsectLinesegPlane (  P1, UL, PP,  plane );
	myIsectLinesegPlane (  P2, LL, PP,  plane );
	myIsectLinesegPlane (  P3, UR, PP,  plane );
	myIsectLinesegPlane (  P4, LR, PP,  plane );

	area = sgTriArea(P1,P2,P3) + sgTriArea(P2,P3,P4);

	rHeight = sqrt( (P2[1]-P1[1])*(P2[1]-P1[1]) + (P2[0]-P1[0])*(P2[0]-P1[0]) );
	rHeight = rHeight + sqrt( (P4[1]-P3[1])*(P4[1]-P3[1]) + (P4[0]-P3[0])*(P4[0]-P3[0]) );
	rHeight = rHeight / 2;

	rWidth = sqrt( (P3[1]-P1[1])*(P3[1]-P1[1]) + (P3[0]-P1[0])*(P3[0]-P1[0]) );
	rWidth = rWidth + sqrt( (P4[1]-P2[1])*(P4[1]-P2[1]) + (P4[0]-P2[0])*(P4[0]-P2[0]) );
	rWidth = rWidth / 2;

#ifdef DISPLAY_CAMINFO
	fprintf(stdout,"     Avg Width is %8.4f \n", rWidth);
	fprintf(stdout,"     Avg Height is %8.4f \n", rHeight);
	fprintf(stdout,"     Area is %8.4f \n\n", area);
#endif

	return 0;
}

