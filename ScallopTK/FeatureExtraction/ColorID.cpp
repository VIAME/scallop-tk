
#include "ColorID.h"

namespace ScallopTK
{

//------------------------------------------------------------------------------
//                                 Definitions
//------------------------------------------------------------------------------


void createOrientedsummaryImages( IplImage *base, CandidatePtrVector& cds ) {
  int n = 64;
  for( int i=0; i<cds.size(); i++ ) {

    // Add a little extra for boundary comparison
    float add_ratio = 1.36;
    float major = add_ratio * cds[i]->major;
    float minor = add_ratio * cds[i]->minor;
    float angle = (-cds[i]->angle+180)*PI/180;

    // Calculate Bounding Box Size
    int r_min = cds[i]->r - major;
    int c_min = cds[i]->c - major;
    int r_max = cds[i]->r + major;
    int c_max = cds[i]->c + major;
    
    // Special case: Boundary shifts
    float lower_r_cut = 0.0f;
    float upper_r_cut = 0.0f;
    float lower_c_cut = 0.0f;
    float upper_c_cut = 0.0f;
    
    // Adjust for Image Boundaries
    if( r_min < 0 ) {
      lower_r_cut = -r_min / (cds[i]->major*2);
      r_min = 0;
    }
    if( r_max >= base->height ) {
      upper_r_cut = (r_max-base->height) / (cds[i]->major*2);
      r_max = base->height - 1;
    }
    if( c_min < 0 ) {
      lower_c_cut = -c_min / (cds[i]->major*2);
      c_min = 0;
    }
    if( c_max >= base->width ) {
      upper_c_cut = (c_max-base->width) / (cds[i]->major*2);
      c_max = base->width - 1;
    }

    // Calculate new window widths
    int c_size = c_max - c_min;
    int r_size = r_max - r_min;

    //EXIT CASES - Region too small 
    if( c_max <= 0 || r_max <= 0  || c_min >= base->width || r_min >= base->height || c_size < 3 || r_size < 3 ) {
      cds[i]->isActive = false;
      continue;
    }
        
    // Calculate affine transformation
    float center_r = cds[i]->r;// - r_min;
    float center_c = cds[i]->c;// - c_min;
    float angle_cos = cos( angle );
    float angle_sin = sin( angle );
    double map[6];
    CvMat map_matrix = cvMat(2, 3, CV_64FC1, map);
    CvPoint2D32f inputs[3];
    CvPoint2D32f outputs[3];
    inputs[0].y = major*angle_sin - minor*angle_cos + center_r;
    inputs[0].x = -major*angle_cos - minor*angle_sin + center_c;
    inputs[1].y = major*angle_sin + minor*angle_cos + center_r;
    inputs[1].x = -major*angle_cos + minor*angle_sin + center_c;
    inputs[2].y = -major*angle_sin - minor*angle_cos + center_r;
    inputs[2].x = major*angle_cos - minor*angle_sin + center_c;
    outputs[0].y = 0; outputs[0].x = 0;
    outputs[1].y = 0; outputs[1].x = n;
    outputs[2].y = n; outputs[2].x = 0;
    cvGetAffineTransform( inputs, outputs, &map_matrix ); 

    // Create scaled new iamge
    cds[i]->summaryImage = cvCreateImage( cvSize( n, n ), base->depth, base->nChannels );    
    cvWarpAffine( base, cds[i]->summaryImage, &map_matrix, CV_INTER_LINEAR | CV_WARP_FILL_OUTLIERS,  cvScalarAll(0));
  }
}

void createColorQuadrants( IplImage *base, CandidatePtrVector& cds ) {

  // Constants
  float R1_RATIO = 0.74f;
  float R2_RATIO = 1.00f;
  float R3_RATIO = 1.36f;
  for( unsigned int i=0; i < cds.size(); i++ ) {

    //BELOW SAME AS WATERSHED, OPTIMIZE LATER
    float in_major = cds[i]->major * R1_RATIO;
    float in_minor = cds[i]->minor * R1_RATIO;
    float cent_major = cds[i]->major * R2_RATIO;
    float cent_minor = cds[i]->minor * R2_RATIO;
    float out_major = cds[i]->major * R3_RATIO;
    float out_minor = cds[i]->minor * R3_RATIO;

    // Calculate Bounding Box Size
    float angle = (cds[i]->angle)*PI/180; 
    double tr = atan( -out_minor * tan( angle ) / out_major );
    double tc = atan( (out_minor / out_major) / tan( angle ) );
    int r_min = cds[i]->r + out_major*cos(tr)*cos(angle) - out_minor*sin(tr)*sin(angle);
    int c_min = cds[i]->c + out_major*cos(tc)*sin(angle) + out_minor*sin(tc)*cos(angle);
    int r_max = 2*cds[i]->r - r_min;
    int c_max = 2*cds[i]->c - c_min;

    // Adjust - Edit above and remove later
    if( r_min > r_max ) {
      int temp = r_min;
      r_min = r_max;
      r_max = temp;
    }
    if( c_min > c_max ) {
      int temp = c_min;
      c_min = c_max;
      c_max = temp;
    }  

    // Adjust for Image Boundaries
    if( r_min < 0 )
      r_min = 0;
    if( r_max >= base->height )
      r_max = base->height - 1;
    if( c_min < 0 )
      c_min = 0;
    if( c_max >= base->width )
      c_max = base->width - 1;

    // Calculate window widths
    int c_size = c_max - c_min;
    int r_size = r_max - r_min;
    
    // EXIT CASES - Region too small 
    if( c_max <= 0 || r_max <= 0  || c_min >= base->width || r_min >= base->height || c_size < 3 || r_size < 3 ) {
      cds[i]->isActive = false;
      continue;
    }

    // Create mask
    cds[i]->colorQR = r_min;
    cds[i]->colorQC = c_min;
    cds[i]->colorQuadrants = cvCreateImage( cvSize(c_size, r_size), IPL_DEPTH_8U, 1 );
    for( unsigned int j=0; j<COLOR_BINS; j++ ) 
      cds[i]->colorBinCount[j] = 0;
    drawColorRing( cds[i]->colorQuadrants, cds[i]->r-r_min, cds[i]->c-c_min, cds[i]->angle, 
      in_major, in_minor, cent_major, out_major, cds[i]->colorBinCount );

  }
}

void calculateColorFeatures( IplImage* color_img, hfResults *color_class, Candidate *cd ) {
  if( !cd->isActive )
    return;
  assert( color_img->width == color_class->NetScallops->width );

  float class_regions[COLOR_BINS];
  float color_regionsCh1[COLOR_BINS];
  float color_regionsCh2[COLOR_BINS];
  float color_regionsCh3[COLOR_BINS];
  for( int i=0; i<COLOR_BINS; i++ ) {
    class_regions[i] = 0;
    color_regionsCh1[i] = 0;
    color_regionsCh2[i] = 0;
    color_regionsCh3[i] = 0;
  }

  //Initialize matrices for kmeans
  int entries = 0;
  int gcounter1 = 0;
  int gcounter2 = 0;
  int gcounter3 = 0;
  int gcounter4 = 0;
  int gcounter5 = 0;
  int gcounter6 = 0;
  for( int i=16; i<32; i++ )
    entries += cd->colorBinCount[i];

  //START PASS - Collect all color data from both images in this pass
  //Optimize w/ pointer ops later
  int kindex = 0;
  IplImage* mask = cd->colorQuadrants;
  IplImage* cc = color_class->NetScallops;
  int rskip = cd->colorQR;
  int cskip = cd->colorQC;
  for( int r=0; r<mask->height; r++ ) {
    for( int c=0; c<mask->width; c++ ) {
      int regionID = ((char*)(mask->imageData + mask->widthStep*r))[c];
      int posr = rskip + r;
      int posc = cskip + c;
      float cc_value = ((float*)(cc->imageData + cc->widthStep*posr))[posc];
      float *color_ptr = ((float*)(color_img->imageData + color_img->widthStep*posr))+posc*3;
      class_regions[regionID] = class_regions[regionID] + cc_value;
      color_regionsCh1[regionID] = color_regionsCh1[regionID] + color_ptr[0];
      color_regionsCh2[regionID] = color_regionsCh2[regionID] + color_ptr[1];
      color_regionsCh3[regionID] = color_regionsCh3[regionID] + color_ptr[2];
      if( regionID < 32 && regionID >= 16 ) {
        if( cc_value > 0.0f )
          gcounter1++;
        if( cc_value > 0.001f )
          gcounter2++;
        if( cc_value > 0.003f )
          gcounter3++;
        if( cc_value > 0.005f )
          gcounter4++;
        if( cc_value > 0.01f )
          gcounter5++;
        if( cc_value > 0.05f )
          gcounter6++;
      }
    }
  }

  // Get average value for each region
  for( int i=0; i<COLOR_BINS; i++ ) {
    if( cd->colorBinCount[i] != 0 ) {
      class_regions[i] = class_regions[i] / cd->colorBinCount[i];
      color_regionsCh1[i] = color_regionsCh1[i] / cd->colorBinCount[i];
      color_regionsCh2[i] = color_regionsCh2[i] / cd->colorBinCount[i];
      color_regionsCh3[i] = color_regionsCh3[i] / cd->colorBinCount[i];
    } else if( i <= 8 && i != 0 ) {
      class_regions[i] = class_regions[i-1];
      color_regionsCh1[i] = color_regionsCh1[i-1];
      color_regionsCh2[i] = color_regionsCh2[i-1];
      color_regionsCh3[i] = color_regionsCh3[i-1];
      cd->colorBinCount[i] = 1;
    } else if( i <= 15 && i > 8 ) {
      class_regions[i] = class_regions[i-1];
      color_regionsCh1[i] = color_regionsCh1[i-1];
      color_regionsCh2[i] = color_regionsCh2[i-1];
      color_regionsCh3[i] = color_regionsCh3[i-1];
      cd->colorBinCount[i] = 1;
    } else if( i == 8 ) {
      class_regions[i] = class_regions[0];
      color_regionsCh1[i] = color_regionsCh1[0];
      color_regionsCh2[i] = color_regionsCh2[0];
      color_regionsCh3[i] = color_regionsCh3[0];
      cd->colorBinCount[i] = 1;
    } else if( i <= 31 && i > 16 ) {
      class_regions[i] = class_regions[i-1];
      color_regionsCh1[i] = color_regionsCh1[i-1];
      color_regionsCh2[i] = color_regionsCh2[i-1];
      color_regionsCh3[i] = color_regionsCh3[i-1];
      cd->colorBinCount[i] = 1;
    } else if( i == 16 ) {
      class_regions[i] = class_regions[8];
      color_regionsCh1[i] = color_regionsCh1[8];
      color_regionsCh2[i] = color_regionsCh2[8];
      color_regionsCh3[i] = color_regionsCh3[8];
      cd->colorBinCount[i] = 1;
    } else {
      // probabalistically never reached
      //Heavy environmental lvl
      class_regions[i] = -0.05;
      //RGB grayish
      color_regionsCh1[i] = 0.43;
      color_regionsCh2[i] = 0.47;
      color_regionsCh3[i] = 0.48;
      cd->colorBinCount[i] = 1;
    }
  }

  // Create color feature matrix - part 1 - classifier edge differences u2 - u1
  for( int i=0; i<8; i++ )
    cd->colorFeatures[i] = class_regions[16+i] - class_regions[8+i];

  // part 2 - net class radial dir - bins 8->15, bins 16->23, bins 24->31
  float inside_class_avg = 0.0f;
  int inside_class_entries = 0;
  for( int i=0; i<8; i++ ) {

    // calc slice avg
    float slice_avg;
    int count_sum = cd->colorBinCount[24+i] + cd->colorBinCount[16+i];
    slice_avg = class_regions[24+i]*cd->colorBinCount[24+i] + 
          class_regions[16+i]*cd->colorBinCount[16+i];
    slice_avg = slice_avg / count_sum;

    // update total avg
    inside_class_avg = inside_class_avg*inside_class_entries + slice_avg*count_sum;
    inside_class_entries = inside_class_entries + count_sum;
    inside_class_avg = inside_class_avg / inside_class_entries;

    // insert feature entries
    cd->colorFeatures[i+8] = slice_avg - class_regions[8+i];
    cd->colorFeatures[i+16] = slice_avg;
    cd->colorFeatures[i+24] = class_regions[8+i];
  }

  // part 3 - net class full template - bins 32, 33, 34
  float outside_class_avg = 0.0f;
  for( int i=0; i<8; i++ )
    outside_class_avg = outside_class_avg + class_regions[8+i];
  outside_class_avg = outside_class_avg / 8;
  cd->colorFeatures[32] = inside_class_avg - outside_class_avg;
  cd->colorFeatures[33] = outside_class_avg;
  cd->colorFeatures[34] = inside_class_avg;

  // part 4 - color edge differences u2 - u1 - bins 35 - 42, bins 43-50, bins 51-58
  for( int i=0; i<8; i++ ) {
    cd->colorFeatures[i+35] = color_regionsCh1[16+i] - color_regionsCh1[8+i];
    cd->colorFeatures[i+43] = color_regionsCh2[16+i] - color_regionsCh2[8+i];
    cd->colorFeatures[i+51] = color_regionsCh3[16+i] - color_regionsCh3[8+i];
  }

  // part 5 - color edge groups - bins 59-106
  for( int i=0; i<8; i++ ) {
    cd->colorFeatures[i+59] = color_regionsCh1[16+i];
    cd->colorFeatures[i+67] = color_regionsCh2[16+i];
    cd->colorFeatures[i+75] = color_regionsCh3[16+i];
    cd->colorFeatures[i+83] = color_regionsCh1[8+i];
    cd->colorFeatures[i+91] = color_regionsCh2[8+i];
    cd->colorFeatures[i+99] = color_regionsCh3[8+i];
  }

  // part 6 - total color - bins 107-109, 110-112, 113-115
  float avg_outer_color[3] = { 0.0, 0.0, 0.0 };
  float avg_inner_color[3] = { 0.0, 0.0, 0.0 };
  for( int i=8; i<15; i++ ) {
    avg_outer_color[0] = avg_outer_color[0] + color_regionsCh1[i];
    avg_outer_color[1] = avg_outer_color[1] + color_regionsCh2[i];
    avg_outer_color[2] = avg_outer_color[2] + color_regionsCh3[i];
  }
  avg_outer_color[0] = avg_outer_color[0] / 8;
  avg_outer_color[1] = avg_outer_color[1] / 8;
  avg_outer_color[2] = avg_outer_color[2] / 8;
  for( int i=16; i<32; i++ ) {
    avg_inner_color[0] = avg_inner_color[0] + color_regionsCh1[i];
    avg_inner_color[1] = avg_inner_color[1] + color_regionsCh2[i];
    avg_inner_color[2] = avg_inner_color[2] + color_regionsCh3[i];
  }
  avg_inner_color[0] = avg_inner_color[0] / 16;
  avg_inner_color[1] = avg_inner_color[1] / 16;
  avg_inner_color[2] = avg_inner_color[2] / 16;
  for( int i=0; i<3; i++ ) {
    cd->colorFeatures[i+107] = avg_inner_color[i] - avg_outer_color[i];
    cd->colorFeatures[i+110] = avg_inner_color[i];
    cd->colorFeatures[i+113] = avg_outer_color[i];
  }  

  // part 7 - color trace values
  cd->colorFeatures[116] = (float)gcounter1 / entries;
  cd->colorFeatures[117] = (float)gcounter2 / entries;
  cd->colorFeatures[118] = (float)gcounter3 / entries;
  cd->colorFeatures[119] = (float)gcounter4 / entries;
  cd->colorFeatures[120] = (float)gcounter5 / entries;
  cd->colorFeatures[121] = (float)gcounter6 / entries;

  // calculate avg regional colors used for expensive edge selection process
  cd->innerColorAvg[0] = avg_inner_color[0];
  cd->innerColorAvg[1] = avg_inner_color[1];
  cd->innerColorAvg[2] = avg_inner_color[2];
  avg_outer_color[0] = 0.0f;
  avg_outer_color[1] = 0.0f;
  avg_outer_color[2] = 0.0f;
  for( int i=0; i<15; i++ ) {
    avg_outer_color[0] = avg_outer_color[0] + color_regionsCh1[i];
    avg_outer_color[1] = avg_outer_color[1] + color_regionsCh2[i];
    avg_outer_color[2] = avg_outer_color[2] + color_regionsCh3[i];
  }
  cd->outerColorAvg[0] = avg_outer_color[0] / 16;
  cd->outerColorAvg[1] = avg_outer_color[1] / 16;
  cd->outerColorAvg[2] = avg_outer_color[2] / 16;
}

}

