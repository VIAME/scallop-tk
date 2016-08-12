#ifndef SCALLOP_TK_VISUAL_DEBUGGER_H_
#define SCALLOP_TK_VISUAL_DEBUGGER_H_

//Include Files
#include <iostream>
#include <vector>
#include <math.h>

#include <GL/glut.h>

#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

#include "SaveImageIcon.h"

//Configuration
using namespace std;

//Image display modes
const int VD_STANDARD  = 0x01;
const int VD_RANGE     = 0x02;
const int VD_MINMAX	   = 0x03;

//Image display options for automatic mode
const int VD_FAR_LEFT  = 0x04;
const int VD_LEFT      = 0x05;
const int VD_CENTER    = 0x06;
const int VD_RIGHT     = 0x07;
const int VD_FAR_RIGHT = 0x08;

//Height Spacing
const float VD_HEIGHT_SPAC_DEFAULT = 0.016f;

//OpenCV Interface Functions
void vdDelete();
void vdInitiateLoop();


void vdInitialize();

//Manual Placement Mode
bool vdAddImage( IplImage *img, int mode, float px, float py, float pz,
  int slant, float width, float height );

//Expand Image Placement in View to New Row
void vdNewRow( float row_height_spacing = VD_HEIGHT_SPAC_DEFAULT );

//Automatic Placement Mode
bool vdAddImage( IplImage *img, int mode, int lrc, bool new_entry,
  float minRange = 0.0f, float maxRange = 0.0f);

//Struct for representing a filter and its orientation
struct vdFilter {
	IplImage *filter;
	unsigned int texture;
	float x, y, z;
	int slant;
	float width, height;
};

#endif
