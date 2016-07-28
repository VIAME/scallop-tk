
#ifndef SCOTT_CAMERA_H_
#define SCOTT_CAMERA_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <float.h>

#include "../Common/definitions.h"

#define sgFloat float
#define SGfloat float

typedef SGfloat sgVec3 [ 3 ] ;
typedef SGfloat sgVec4 [ 4 ] ;
typedef SGfloat sgMat4  [4][4] ;

#define SG_MAX   FLT_MAX
#define SG_ZERO  0.0f
#define SG_HALF  0.5f
#define SG_ONE   1.0f
#define SG_180   180.0f

#ifndef M_PI
#define SG_PI  3.1415926535f
#else
#define SG_PI  ((SGfloat) M_PI)
#endif

#define SG_DEGREES_TO_RADIANS  (SG_PI/SG_180)
#define SG_RADIANS_TO_DEGREES  (SG_180/SG_PI)

inline SGfloat sgSin ( SGfloat s );                
inline SGfloat sgCos ( SGfloat s );                
inline SGfloat sgSqrt( const SGfloat x );
inline SGfloat sgAbs ( const SGfloat a );
inline void sgZeroVec3 ( sgVec3 dst );
inline void sgSetVec3 ( sgVec3 dst, const SGfloat x, const SGfloat y, const SGfloat z );
inline void sgScaleVec3 ( sgVec3 dst, const SGfloat s );
inline void sgScaleVec3 ( sgVec3 dst, const sgVec3 src, const SGfloat s );
inline void sgSubVec3 ( sgVec3 dst, const sgVec3 src1, const sgVec3 src2 );
inline void sgAddVec3 ( sgVec3 dst, const sgVec3 src1, const sgVec3 src2 );
inline SGfloat sgScalarProductVec3 ( const sgVec3 a, const sgVec3 b );
void sgVectorProductVec3 ( sgVec3 dst, const sgVec3 a, const sgVec3 b );
inline SGfloat sgLengthVec3 ( const sgVec3 src );
inline void sgNormaliseVec3 ( sgVec3 dst );
void sgMakeNormal(sgVec3 dst, const sgVec3 a, const sgVec3 b, const sgVec3 c );
inline void sgMakePlane ( sgVec4 dst, const sgVec3 a, const sgVec3 b, const sgVec3 c );
SGfloat sgIsectLinesegPlane ( sgVec3 dst, sgVec3 v1, sgVec3 v2, sgVec4 plane );
void sgMakeCoordMat4 ( sgMat4 m, const SGfloat x, const SGfloat y, const SGfloat z, const SGfloat h, const SGfloat p, const SGfloat r );
void sgXformPnt3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat );
void sgXformPnt4 ( sgVec4 dst, const sgVec4 src, const sgMat4 mat );
void sgFullXformPnt3 ( sgVec3 dst, const sgVec3 src, const sgMat4 mat );
float  myIsectLinesegPlane ( sgVec3 dst, sgVec3 v1, sgVec3 v2, sgVec4 plane );
float sgTriArea( sgVec3 p0, sgVec3 p1, sgVec3 p2 );
int calculateDimensions(float heading, float pitch, float roll, float altitude, float width, float height, 
						float focal_length, float &area, float &rHeight, float &rWidth);

#endif