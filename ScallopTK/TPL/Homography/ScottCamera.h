
#ifndef SCOTT_CAMERA_H_
#define SCOTT_CAMERA_H_

#include "ScallopTK/Utilities/Definitions.h"

int calculateDimensions(float heading, float pitch, float roll, float altitude, float width, float height,
						float focalLength, float &area, float &rHeight, float &rWidth);

#endif
