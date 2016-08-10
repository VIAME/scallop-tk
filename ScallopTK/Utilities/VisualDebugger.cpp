
//------------------------------------------------------------------------------
// visualDeugger.h
// author: Matt Dawkins
// description: An openGL class defining an easy method to view and save
// intermediate steps in a computer vision algorithm using OpenCV
//------------------------------------------------------------------------------

#include "VisualDebugger.h"

//------------------------------------------------------------------------------
//                                  Constants
//------------------------------------------------------------------------------

//Geometric
const float vdPI = 3.141592654f;
const float vdFilterWidth = 1.0f;

//Camera movement properties
const float vdStrafeStep = 0.10f;
const float vdRotateStep = 1.00f;
const float vdZoomStep = 0.20f;

//Algorithm Properties
const int VD_MAX_FILTERS = 30;

//------------------------------------------------------------------------------
//                               Global Variables
//------------------------------------------------------------------------------

//Current windows Size
int vdW, vdH;

//Right screen parameters
bool vdRightEnabled;
unsigned int vdRightImg;

//Left screen parameters
vector<vdFilter> vdFilterList;

//Camera view
float vdX=0, vdY=0, vdZ=0, vdXRot=0, vdYRot=0, vdZRot=0, vdZoom=1;

//Mouse position
int vdMX, vdMY, vdMW; //<-- Mouse x, y, window (left, right)
int vdLMX, vdLMY; //<-- Last x, y
float vdMXd, vdMYd; //<-- Mouse x, y in drawing coords

//Texture Labels
unsigned int vdSaveIco, vdSaveSelIco;
unsigned int vdFilterTextures[VD_MAX_FILTERS];

//GUI Globals
bool vdSavePos = false; //<-- True if we are over the save button
bool vdRotateEnabled = false; //<-- True if right mouse button held

//Automatic Image Adding Options
bool vdAuto = false;
bool vdFirstImage = true;
float vdInitialHeight = 0.0f;
float vdBaseImageHeight = 0.25f;
float vdInitialWidth = 0.0f;
float vdBaseImageWidth = 0.30f;
float vdVertSpacing = 0.05f;
float vdHoriSpacing = 0.05f;
float vdHeightSpacing = 0.016f;
float vdColHeights[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
float vdCurrentPos = -0.85f;

//------------------------------------------------------------------------------
//                         Internal Function Prototypes
//------------------------------------------------------------------------------

void vdCamera();
void vdDisplay(void);
void vdIdleFunc(void);
void vdReshape(int w, int h);
void vdKey(unsigned char k, int x, int y);
void vdMouse(int btn, int state, int x, int y);
void vdDisplayLeft();
void vdDelete();
void vdConfigureTexture(unsigned int &texture, GLubyte *data, int width, int height, GLenum format);
void vdLoadIconSaveTextures();
void vdFilterDisp( unsigned int filtNum );
void vdCamera();
void vdMouseToFrame(int mx, int my, float &px, float &py );
void vdFrameToMouse(float px, float py, int side, int &mx, int &my );
void vdMouseCheckPos();
void vdKey(unsigned char key, int x, int y);
void vdPassiveMouse(int x, int y);

//------------------------------------------------------------------------------
//                               Utility Functions
//------------------------------------------------------------------------------

//Deallocate all memory used by VD
void vdDelete() {
	for( unsigned int i=0; i<vdFilterList.size(); i++ ) {
		cvReleaseImage( &vdFilterList[i].filter );
	}
}

//------------------------------------------------------------------------------
//                               Texture Functions
//------------------------------------------------------------------------------

//Create a new 2d texture from a 1, 3, or 4 channel 8-bit image
void vdConfigureTexture(unsigned int &texture, GLubyte *data, int width, int height, GLenum format)
{	
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	if( format == GL_RGB )
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	else if( format == GL_BGR_EXT )
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
	else if( format == GL_RGBA ) 
		glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	else if( format == GL_LUMINANCE8  ) 
		glTexImage2D(GL_TEXTURE_2D, 0, 1, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data); 
	else
		cerr << "(Visual Debugger) Error: Unsupported texture template used" << endl;
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}

//Turns an OpenCV image into an OpenGL texture, accepts 1 or 3 channel images
int vdImage2Texture( IplImage *img, int mode, float min = 0, float max = 255 ) {

	//General Variables
	unsigned int textureNumber;

	//Image Statistics
	int imWidth = img->width;
	int imHeight = img->height;
	int imChannels = img->nChannels;
	int imDepth = img->depth;
	int imStep = img->widthStep;  

	if( mode == VD_RANGE ) {
		IplImage* temp = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
		cvThreshold(img, temp, max, max, CV_THRESH_TRUNC );
		cvConvertScale( temp, temp, 1/(max-min), -min/(max-min) );
		cvThreshold(temp, temp, 0, 0, CV_THRESH_TOZERO );
		img = temp;
		//Create Texture
		if( img->depth == IPL_DEPTH_32F && img->nChannels == 1 ) {
			IplImage *im8 = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 1);
			cvConvertScale(img, im8, 255.);
			vdConfigureTexture(textureNumber, (GLubyte *)im8->imageData, imWidth, imHeight, GL_LUMINANCE8 );
			cvReleaseImage( &im8 );
		} else if( img->depth == IPL_DEPTH_32F && img->nChannels == 3 ) {
			IplImage *im8c3 = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 3);
			cvConvertScale(img, im8c3, 255.);
			vdConfigureTexture(textureNumber, (GLubyte *)im8c3->imageData, imWidth, imHeight, GL_BGR_EXT );
			cvReleaseImage( &im8c3 );
		} else if( img->depth == IPL_DEPTH_16U ) {
			IplImage *im8 = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, img->nChannels);
			cvConvertScale(img, im8, 1/255.);
			vdConfigureTexture(textureNumber, (GLubyte *)im8->imageData, imWidth, imHeight, GL_BGR_EXT );
			cvReleaseImage( &im8 );
		} else {
			vdConfigureTexture(textureNumber, (GLubyte *)img->imageData, imWidth, imHeight, GL_BGR_EXT);
		}
		cvReleaseImage(&img);
	} else if( mode == VD_STANDARD ) {
		//Create Texture
		if( img->depth == IPL_DEPTH_32F && img->nChannels == 1 ) {
			IplImage *im8 = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 1);
			cvConvertScale(img, im8, 255.);
			vdConfigureTexture(textureNumber, (GLubyte *)im8->imageData, imWidth, imHeight, GL_LUMINANCE8 );
			cvReleaseImage( &im8 );
		} else if( img->depth == IPL_DEPTH_32F && img->nChannels == 3 ) {
			IplImage *im8c3 = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 3);
			cvConvertScale(img, im8c3, 255.);
			vdConfigureTexture(textureNumber, (GLubyte *)im8c3->imageData, imWidth, imHeight, GL_BGR_EXT );
			cvReleaseImage( &im8c3 );
		} else if( img->depth == IPL_DEPTH_16U ) {
			IplImage *im8 = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, img->nChannels);
			cvConvertScale(img, im8, 1/255.);
			vdConfigureTexture(textureNumber, (GLubyte *)im8->imageData, imWidth, imHeight, GL_BGR_EXT );
			cvReleaseImage( &im8 );
		} else {
			vdConfigureTexture(textureNumber, (GLubyte *)img->imageData, imWidth, imHeight, GL_BGR_EXT);
		}
	}

	return textureNumber;
}

//Loads the textures for our save icons
void vdLoadIconSaveTextures() {

	//Format data from header file
	//(We could just precompute this but it takes up no proc power)
	//(And I'm lazy)
	GLubyte savedIco[42][42][4];
	GLubyte savedSelIco[42][42][4];
	for( int r = 0; r < 42; r++ ) {
		for( int c = 0; c < 42; c++ ) {
			savedIco[r][c][0] = vdSaveR[r][c];
			savedIco[r][c][1] = vdSaveG[r][c];
			savedIco[r][c][2] = vdSaveB[r][c];
			savedIco[r][c][3] = 255;
			savedSelIco[r][c][0] = vdSaveSelectedR[r][c];
			savedSelIco[r][c][1] = vdSaveSelectedG[r][c];
			savedSelIco[r][c][2] = vdSaveSelectedB[r][c];
			savedSelIco[r][c][3] = 255;
		}
	}

	//Convert to textures
	vdConfigureTexture(vdSaveIco, (GLubyte*) &savedIco, 42, 42, GL_RGBA);
	vdConfigureTexture(vdSaveSelIco, (GLubyte*) &savedSelIco, 42, 42, GL_RGBA);
}

//------------------------------------------------------------------------------
//                              Filter Functions
//------------------------------------------------------------------------------

void vdFilterDisp( unsigned int filtNum ) {

	//Retrieve necessary variables
	int slant = vdFilterList[filtNum].slant;
	float px = vdFilterList[filtNum].x;
	float py = vdFilterList[filtNum].y;
	float pz = vdFilterList[filtNum].z;
	float h = vdFilterList[filtNum].height;
	float w = vdFilterList[filtNum].width;


	//Calculate image coordinates
	float x[4], y[4], z[4];
	if( slant == 0 ) {
		x[0] = x[3] = px - h / 2;
		x[1] = x[2] = px + h / 2;
		y[0] = y[1] = y[2] = y[3] = py;
		z[0] = z[1] = pz - w / 2;
		z[2] = z[3] = pz + w / 2;
	} else if( slant == 1 ) {
		y[0] = y[3] = py + h / (2 * sqrt((float)2) );
		y[1] = y[2] = py - h / (2 * sqrt((float)2) );
		x[0] = x[3] = px - h / (2 * sqrt((float)2) );
		x[1] = x[2] = px + h / (2 * sqrt((float)2) );
		z[0] = z[1] = pz - w / 2;
		z[2] = z[3] = pz + w / 2;

	} else if( slant == 2 ) {
		x[0] = x[1] = x[2] = x[3] = px;
		y[0] = y[3] = py + h / 2;
		y[1] = y[2] = py - h / 2;
		z[0] = z[1] = pz - w / 2;
		z[2] = z[3] = pz + w / 2;
	}

	//Display filter image
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, vdFilterList[filtNum].texture);
	glLoadName( filtNum+1 );
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); 
		glVertex3f(x[0], y[0], z[0]);
        glTexCoord2f(0.0, 1.0); 
		glVertex3f(x[1], y[1], z[1]);
        glTexCoord2f(1.0, 1.0); 
		glVertex3f(x[2], y[2], z[2]);
        glTexCoord2f(1.0, 0.0); 
		glVertex3f(x[3], y[3], z[3]);		
    glEnd();
	glDisable(GL_TEXTURE_2D);
	//TODO: Display other sides of prism

}

//------------------------------------------------------------------------------
//                               Camera Functions
//------------------------------------------------------------------------------

//Adjust camera position
void vdCamera() {
	glLoadIdentity();
	glTranslated(-vdX,-vdY,-vdZ);
    glRotatef(vdXRot,1.0,0.0,0.0);
    glRotatef(vdYRot,0.0,1.0,0.0);
	glRotatef(vdZRot,0.0,0.0,1.0);
	glScalef(vdZoom,vdZoom,vdZoom);
}

//------------------------------------------------------------------------------
//                               Mouse Functions
//------------------------------------------------------------------------------

//Takes the xy coords of the mouse position and translates to our planar coords
void vdMouseToFrame(int mx, int my, float &px, float &py ) {

	float w = (float)vdW / 2;
	if( w <= vdH ) {
		py = ((float)vdH/w)*(1 - 2*(float)my / vdH);
		if( mx < w ) {
			px = 2*(float)mx/w - 1;			
			vdMW = 0;
		} else {
			px = 2*(float)mx/w - 3;	
			vdMW = 1;
		}
	} else {
		py = 1 - 2*(float)my / vdH;
		if( mx < w ) {
			px = ((float)w/(float)vdH)*(2*(float)mx/w - 1);
			vdMW = 0;
		} else {
			px = ((float)w/(float)vdH)*(2*(float)mx/w - 3);
			vdMW = 1;
		}
	}
}

//Opposite of the above
void vdFrameToMouse(float px, float py, int side, int &mx, int &my ) {

	float w = (float)vdW / 2;
	if( w <= vdH ) {
		my = vdH*(1-py/((float)vdH/w))/2;
		if( side == 0 ) {
			mx = w*(px+1)/2;		
		} else {
			mx = w*(px+3)/2;	
		}
	} else {
		my = vdH*(1-py)/2;
		if( side == 0 ) {
			mx = w*(px/((float)w/(float)vdH)+1)/2;
		} else {
			mx = w*(px/((float)w/(float)vdH)+3)/2;
		}
	}
}

//Check mouse position to see if we need to highlight the save button
void vdMouseCheckPos() {

	bool prior = vdSavePos;

	//Check to see if the mouse is over the button
	if( vdMXd >= 0.75 && vdMXd <= 0.96 && vdMYd <= 0.90 && vdMYd >= 0.69  && vdMW == 1 )
		vdSavePos = true;
	else
		vdSavePos = false;

	//If there was a change, signal that we need to redisplay
	if( prior != vdSavePos )
		glutPostRedisplay();
}

//------------------------------------------------------------------------------
//                              Interface Functions
//------------------------------------------------------------------------------

//Returns the image clicked when left clicking
int vdIsFilter() {
	GLuint buff[32] = {0};
 	GLint hits, view[4];
 	glSelectBuffer(32, buff);
 	glGetIntegerv(GL_VIEWPORT, view);
 	glRenderMode(GL_SELECT);
 	glInitNames();
 	glPushName(0);
 	glMatrixMode(GL_PROJECTION);
 	glPushMatrix();
 	glLoadIdentity();
 	gluPickMatrix(vdMX, vdH-vdMY, 1.0, 1.0, view);
 	glMatrixMode(GL_MODELVIEW);
 	vdDisplayLeft();
	glFlush();
 	glMatrixMode(GL_PROJECTION);
 	glPopMatrix();
 	hits = glRenderMode(GL_RENDER);
 	glMatrixMode(GL_MODELVIEW);
	if( hits != 0 ) {
		return buff[3] - 1;
	}
	return -1;
}

//On a keypress...
void vdKey(unsigned char key, int x, int y)
{
	if (key=='d') {
		float rad = (vdYRot / 180 * vdPI);
		vdX += float(cos(vdYRot)) * vdStrafeStep;
		vdZ += float(sin(vdYRot)) * vdStrafeStep;
		vdDisplay();
	} else if (key=='a') {
		float rad = (vdYRot / 180 * vdPI);
		vdX -= float(cos(vdYRot)) * vdStrafeStep;
		vdZ -= float(sin(vdYRot)) * vdStrafeStep;
		vdDisplay();
	} else if (key=='+') {
		vdZoom = vdZoom + vdZoomStep;
		vdDisplay();
	} else if (key=='-') {
		vdZoom = vdZoom - vdZoomStep;
		vdDisplay();
	}
}

//Get the current mouse position...
void vdPassiveMouse(int x, int y) {
	vdMX = x; 
	vdMY = y;
	vdMouseToFrame( x, y, vdMXd, vdMYd );
	vdMouseCheckPos();
}

//Get the current mouse position when something's clicked...
void vdActiveMouse(int x, int y) {
	vdMX = x; 
	vdMY = y;
	if( vdRotateEnabled && vdMW == 0 ) {
		float difx = vdMX - vdLMX;
		float dify = vdMY - vdLMY;
		vdYRot = vdYRot + difx * vdRotateStep;
		vdXRot = vdXRot + dify * vdRotateStep; 
		glutWarpPointer(vdW/4,vdH/2);
		vdLMX = vdW/4;
		vdLMY = vdH/2;
		vdDisplay();
	}
}

//On a mouse button press...
void vdMouse(int btn, int state, int x, int y)
{
	if(btn==GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		//Save button selected
		if( vdSavePos ) {
			cvSaveImage("baseImage.jpg", vdFilterList[vdRightImg].filter);
		}
		//Filter selected?
		if( vdMW == 0 ) {
			int sel = vdIsFilter();
			if( sel != -1 ) {
				vdRightEnabled = true;
				vdRightImg = sel;				
				glutPostRedisplay();
			}
		}

	}
	if(btn==GLUT_RIGHT_BUTTON && state == GLUT_DOWN && vdMX < vdW / 2 ) {
		//Enable camera rotate		
		glutSetCursor(GLUT_CURSOR_NONE);
		vdRotateEnabled = true;
		vdLMX = vdMX;
		vdLMY = vdMY;
	}
	if(btn==GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		//Disable camera rotate
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		vdRotateEnabled = false;
	}
} 

//------------------------------------------------------------------------------
//                               GUI Functions
//------------------------------------------------------------------------------

//Displays the save button
void vdDisplaySaveButton( bool selected ) {

	//Display button
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	if( !selected )
		glBindTexture(GL_TEXTURE_2D, vdSaveIco);
	else
		glBindTexture(GL_TEXTURE_2D, vdSaveSelIco);
	glBegin(GL_QUADS);
		glTexCoord2f(1.0, 1.0); glVertex2f(0.76f, 0.70f);
        glTexCoord2f(1.0, 0.0); glVertex2f(0.76f, 0.89f);
        glTexCoord2f(0.0, 0.0); glVertex2f(0.95f, 0.89f);
        glTexCoord2f(0.0, 1.0); glVertex2f(0.95f, 0.70f);		
    glEnd();
	glDisable(GL_TEXTURE_2D);
}

//Displays the right screen image
void vdDisplayRightImage() {
	//Center = ( 0, -0.15 )
	//Max Width = 0.95 * 2;
	//Max Height = 0.75 * 2;
	float ratio = vdFilterList[vdRightImg].height / vdFilterList[vdRightImg].width;
	float x1, x2, y1, y2;
	if( ratio < 1 ) {
		x1 = -0.95f;
		x2 = 0.95f;
		y1 = -0.75f * ratio - 0.15f;
		y2 = 0.75f * ratio - 0.15f;
	} else {
		ratio = 1 / ratio;
		x1 = -0.95f * ratio;
		x2 = 0.95f * ratio;
		y1 = -0.75f - 0.15f;
		y2 = 0.75f - 0.15f;
	}	
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, vdFilterList[vdRightImg].texture);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(x1, y1);
        glTexCoord2f(0.0, 1.0); glVertex2f(x1, y2);
        glTexCoord2f(1.0, 1.0); glVertex2f(x2, y2);
        glTexCoord2f(1.0, 0.0); glVertex2f(x2, y1);		
    glEnd();
	glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------
//                             Display Functions
//------------------------------------------------------------------------------

//Display the left viewport
void vdDisplayLeft() {
	glPopMatrix();
	glViewport(0, 0, vdW/2, vdH);
	vdCamera();	
	for( unsigned int i=0; i<vdFilterList.size(); i++ ) {
		vdFilterDisp( i ); 
	}
}

//Display the right viewport
void vdDisplayRight() {
	//Use correct matrix
	glPushMatrix();
	glLoadIdentity();
	glViewport(vdW/2, 0, vdW/2, vdH);
	//Display GUI
	vdDisplaySaveButton( vdSavePos );	
	//Display right selected image
	if( !vdRightEnabled ) {
		//Display a blank rectangle
		glColor3f(0.10f, 0.25f, 0.50f);
		glRectf(-0.95f, -0.90, 0.95f, 0.61f);
	} else if( vdRightEnabled ) {
		//Display the selected image
		vdDisplayRightImage();
	}
}

//Main display function
void vdDisplay(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	vdDisplayLeft();	
	vdDisplayRight();
	glutSwapBuffers();
}

//On window resize...
void vdReshape (int w, int h) {
	vdW = w;
	vdH = h;
	w /= 2;
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
	if (w <= h)
		glOrtho(-1.0, 1.0, -1.0 * (GLfloat) h / (GLfloat) w,
		1.0 * (GLfloat) h / (GLfloat) w, -1.0, 1.0);
	else
		glOrtho(-1.0 * (GLfloat) w / (GLfloat) h,
		1.0 * (GLfloat) w / (GLfloat) h, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

//------------------------------------------------------------------------------
//                              OpenCV Interfaces
//------------------------------------------------------------------------------

void vdInitialize() {

	//Initialize glut settings
	int init_argc = 1;
	std::string init_args = "Visual Debugger";
  char *init_argv = const_cast<char*>( init_args.c_str() );
	glutInit( &init_argc, &init_argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );

	//Create window
	vdW = 1000;
	vdH = 500;
	glutInitWindowSize( vdW, vdH );
	glutCreateWindow( "Visual Debugger" );
	glClearColor( 0.0, 0.0, 0.0, 0.0 );

	//Enable settings
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glShadeModel( GL_SMOOTH );

	//Load required textures
	vdLoadIconSaveTextures();

	//Configure start view
	vdXRot = 30.0f;
	vdYRot = -35.0f;
	vdZRot = 0.0f;
}

void vdInitiateLoop() {

	//Set up display and callbacks
	glutDisplayFunc(vdDisplay);
	glutReshapeFunc(vdReshape);
	glutKeyboardFunc(vdKey);
	glutMouseFunc(vdMouse);
	glutPassiveMotionFunc(vdPassiveMouse);
	glutMotionFunc(vdActiveMouse);
	glutMainLoop();
}

bool vdAddImage(IplImage *img, int mode,
				float px, float py, float pz, 
				int slant, float width, float height) {
	
	vdFilter newFilter;
	newFilter.texture = vdImage2Texture( img, mode );
	newFilter.height = height;
	newFilter.width = width;
	newFilter.x = px;
	newFilter.y = py;
	newFilter.z = pz;
	newFilter.slant = slant;
	newFilter.filter = cvCloneImage( img );
	vdFilterList.push_back( newFilter );
	return true;
}

void vdNewRow( float row_height_spacing ) { 
	vdCurrentPos = vdCurrentPos + vdVertSpacing + vdBaseImageHeight;
	vdHeightSpacing = row_height_spacing;
	vdColHeights[0] = 0.0f;
	vdColHeights[1] = 0.0f;
	vdColHeights[2] = 0.0f;
	vdColHeights[3] = 0.0f;
	vdColHeights[4] = 0.0f;
}

bool vdAddImage(IplImage *img, int mode, int lrc, bool new_entry, float minRange, float maxRange) {
	
	// Create new texture for image
	vdFilter newFilter;
	newFilter.texture = vdImage2Texture( img, mode, minRange, maxRange );

	// If this is the first image, record its resolution so we know how to size future entries
	if( vdFirstImage ) {
		vdInitialWidth = (float) img->width;
		vdInitialHeight = (float) img->height;
		vdBaseImageHeight = vdBaseImageWidth*(vdInitialHeight/vdInitialWidth);
		vdFirstImage = false;
	}

	// Set image size in VD coordinates
	newFilter.width = ((float)img->width/vdInitialWidth)*vdBaseImageWidth;
	newFilter.height = newFilter.width * ((float)img->height) / ((float)img->width);
	
	// Determine image row
	if( new_entry ) {
		vdCurrentPos = vdCurrentPos + vdVertSpacing + vdBaseImageHeight;
		vdColHeights[0] = 0.0f;
		vdColHeights[1] = 0.0f;
		vdColHeights[2] = 0.0f;
		vdColHeights[3] = 0.0f;
		vdColHeights[4] = 0.0f;
	}

	// Set image row, col, height (VD coords)
	newFilter.x = vdCurrentPos;

	if( lrc == VD_FAR_LEFT ) {
		newFilter.z = -2*(vdHoriSpacing + vdBaseImageWidth);
		newFilter.y = vdColHeights[0];
		vdColHeights[0] = vdColHeights[0] + vdHeightSpacing;
	} else if( lrc == VD_LEFT ) {
		newFilter.z = -(vdHoriSpacing + vdBaseImageWidth);
		newFilter.y = vdColHeights[1];
		vdColHeights[1] = vdColHeights[1] + vdHeightSpacing;
	} else if ( lrc == VD_CENTER ) {
		newFilter.z = 0.0f;
		newFilter.y = vdColHeights[2];
		vdColHeights[2] = vdColHeights[2] + vdHeightSpacing;
	} else if ( lrc == VD_RIGHT ) { 
		newFilter.z = vdHoriSpacing + vdBaseImageWidth;
		newFilter.y = vdColHeights[3];
		vdColHeights[3] = vdColHeights[3] + vdHeightSpacing;
	} else if ( lrc == VD_FAR_RIGHT ) { 
		newFilter.z = 2*(vdHoriSpacing + vdBaseImageWidth);
		newFilter.y = vdColHeights[4];
		vdColHeights[4] = vdColHeights[4] + vdHeightSpacing;
	} else {
		cerr << "ERROR: Visual Debugger - Auto Add Filter Fail\n";
		return false;
	}

	// Set slant (Automode always = 0)
	newFilter.slant = 0;

	// Copy image for internal vd records
	newFilter.filter = cvCloneImage( img );

	// Add the new filter to our filter vector
	vdFilterList.push_back( newFilter );
	return true;
}
