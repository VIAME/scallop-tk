
#include "ExpensiveSearch.h"

//------------------------------------------------------------------------------
//                            Function Definitions
//------------------------------------------------------------------------------

inline double dirDistance( float& dir1, float& dir2 ) {
  
  float d1 = abs( dir1 - dir2 );
  if( d1 < 0 )
    d1 + 360;
  if( d1 > 180 )
    d1 = 360 - d1;
  if( d1 > 90 )
    d1 = 180 - d1;
  return d1;
}

inline float dirFactor( float& dir1, float& dir2 ) {
  
  float d1 = dirDistance( dir1, dir2 );
  if( d1 < 50 )
    return 1.0;
  return 0.0;
}

inline float distActFunc( const float& normalizedDistOffset ) {
  float d = abs( normalizedDistOffset );
  if( d < 0.15 ) {
    return 1.0f;
  } else if ( normalizedDistOffset < 0.4 && normalizedDistOffset > -0.4 ) {
    return 1.0f - 3.2f*(d - 0.15);
  } else
    return 0.2f;
}

inline float dirActFunc( const float& normalizedDirOffset ) {
  float d = abs( normalizedDirOffset );
  if( d < 20 ) {
    return 1.0f;
  } else if ( d < 45 ) {
    return 1.0f - 0.024 * (d - 20);
  } else
    return 0.4f;
}

void expensiveEdgeSearch( GradientChain& Gradients, hfResults* color, 
  IplImage *ImgLab32f, IplImage *img_rgb_32f, vector<Candidate*> cds ) {

  assert( color->SaliencyMap->width == ImgLab32f->width );

  IplImage *lab_ori = Gradients.dLabOri;
  IplImage *lab_mag = Gradients.dLabMag;

  // For every Candidate, search for edges
  const float SCAN_DIST = 1.33f;
  int height = lab_mag->height;
  int width = lab_mag->width;
  for( unsigned int i = 0; i < cds.size(); i++ ) {

    Candidate* cd = cds[i];

    int lr = cd->r - SCAN_DIST * cd->major;
    int lc = cd->c - SCAN_DIST * cd->major;
    int ur = cd->r + SCAN_DIST * cd->major;
    int uc = cd->c + SCAN_DIST * cd->major;

    if( lr < 0 )
      lr = 0;
    if( lc < 0 )
      lc = 0;
    if( uc > width )
      uc = width;
    if( ur > height )
      ur = height;

    int r_range = ur - lr;
    int c_range = uc - lc;
    int scan_size = r_range * c_range;

    if( r_range < 1 || c_range < 1 ) {
      cds[i]->is_active = false;
      continue;
    }

    // Create color cost (simularity to avg color of obj)
    IplImage *color = NULL;
    if( 1 /*cd->classification != SCALLOP_BURIED*/ ) {
      float avgCh1 = cd->innerColorAvg[0];
      float avgCh2 = cd->innerColorAvg[1];
      float avgCh3 = cd->innerColorAvg[2];
      color = cvCreateImage( cvSize( c_range, r_range ), IPL_DEPTH_32F, 1 );
      float *ptr_rgb = ((float*)(img_rgb_32f->imageData + img_rgb_32f->widthStep*lr))+lc*3;
      float *outptr = (float*)color->imageData;
      int wstep = (img_rgb_32f->widthStep / sizeof( float )) - 3*c_range;
      for( int r = 0; r < r_range; r++ ) {
        for( int c = 0; c < c_range; c++ ) {

          float ch1dif = ptr_rgb[0] - avgCh1;
          float ch2dif = ptr_rgb[1] - avgCh2;
          float ch3dif = ptr_rgb[2] - avgCh3;

          *outptr = log( 1 / (ch1dif*ch1dif + ch2dif*ch2dif + ch3dif*ch3dif) );

          ptr_rgb += 3;
          outptr++;
        }
        ptr_rgb += wstep;
      }
      cvSmooth( color, color, 2, 5, 5 );
      //showImageRange( color );
      /*IplImage *color_dx = cvCreateImage( cvGetSize( color ), IPL_DEPTH_32F, 1 );
      IplImage *color_dy = cvCreateImage( cvGetSize( color ), IPL_DEPTH_32F, 1 );
      cvSobel( color, color_dx, 1, 0, 3 );
      cvSobel( color, color_dy, 0, 1, 3 );
      float *ptr_dx = (float*)color_dx->imageData;
      float *ptr_dy = (float*)color_dy->imageData;
      float *ptr_mag = (float*)color->imageData;
      for( int i=0; i<scan_size; i++ ) {
        float dx = *ptr_dx;
        float dy = *ptr_dy;
        *ptr_mag = sqrt( dx*dx + dy*dy );
        ptr_dx++;
        ptr_dy++;
        ptr_mag++;
      }
      cvReleaseImage( &color_dx );
      cvReleaseImage( &color_dy );*/    
    }

    // Create cost function
    IplImage *cost = cvCreateImage( cvSize( c_range, r_range ), IPL_DEPTH_32F, 1 );

    float *grdmag = ((float*)(lab_mag->imageData + lab_mag->widthStep*lr))+lc;
    float *grddir = ((float*)(lab_ori->imageData + lab_ori->widthStep*lr))+lc;
    float *outptr = (float*)cost->imageData;

    int step = width - c_range;
    int cstep = step*3;

    // Perform cost filtering
    int rel_r = -(cd->r - lr);
    int rel_c = -(cd->c - lc);
    float rad = cd->major;
    if( 0 /*cd->classification == SCALLOP_BURIED*/ ) {
      for( int r = rel_r; r < r_range + rel_r; r++ ) {
        for( int c = rel_c; c < c_range + rel_c; c++ ) {

          float dist = sqrt( (float)r*r + (float)c*c ) - rad;  
          float drad = dist / rad;
          float ang = cvFastArctan(r, c);
          float dird = dirDistance( ang, *grddir );

          *outptr = *grdmag * distActFunc( drad ) * dirActFunc( dird );

          grdmag++;
          grddir++;
          outptr++;
        }
        grdmag+=step;
        grddir+=step;
      }
    } else {
      float *color_ptr = (float*)color->imageData;
      for( int r = rel_r; r < r_range + rel_r; r++ ) {
        for( int c = rel_c; c < c_range + rel_c; c++ ) {

          float dist = sqrt( (float)r*r + (float)c*c ) - rad;  
          float drad = dist / rad;
          float ang = cvFastArctan(r, c);
          float dird = dirDistance( ang, *grddir );

          *outptr = *grdmag * distActFunc( drad ) * dirActFunc( dird ) * *color_ptr;

          grdmag++;
          grddir++;
          outptr++;
          color_ptr++;
        }
        grdmag+=step;
        grddir+=step;
      }
    }

    // Smooth cost func [opt]
    cvSmooth( cost, cost, CV_BLUR, 5, 5 );

#ifdef SS_ENABLE_BENCHMARKINGING
  ss_exe_times[mark] += getTimeSinceLastCall();
#endif

    // Non-max suppression and selection
    IplImage *bin = cvCreateImage( cvGetSize( cost ), IPL_DEPTH_8U, 1 );
    cvZero(bin);

    // Containers for max selection
    const char EDGEL = 255;
    int cost_step_bytes = cost->widthStep;
    int cost_step = cost_step_bytes / sizeof(float);
    int step_dia_1 = -cost_step - 1;
    int step_dia_2 = -cost_step + 1;
    int step_dia_3 = cost_step - 1;
    int step_dia_4 = cost_step + 1;
    int step_up = cost_step;
    int step_down = -cost_step;
    int step_left = -1;
    int step_right = 1;
    for( int r = 1; r < r_range - 1; r++ ) {
      for( int c = 1; c < c_range - 1; c++ ) {

        float ang = cvFastArctan(r+rel_r, c+rel_c);
        float *pos = ((float*)(cost->imageData + cost_step_bytes*r)) + c;
        float val = *pos;

        if ( ang <= 22.5 || ang > 337.5 ) {
          float v1 = *(pos+step_left);
          float v2 = *(pos+step_right);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
          }
        } else if ( ang <= 67.5 ) {
          float v1 = *(pos+step_dia_4);
          float v2 = *(pos+step_dia_1);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
          }
        } else if ( ang <= 112.5 ) {
          float v1 = *(pos+step_up); 
          float v2 = *(pos+step_down);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
          }
        } else if ( ang <= 157.5 ) {
          float v1 = *(pos+step_dia_2);
          float v2 = *(pos+step_dia_3);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
          }
        } else if ( ang <= 202.5 ) {
          float v1 = *(pos+step_right);
          float v2 = *(pos+step_left);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
          }
        } else if ( ang <= 247.5 ) {
          float v1 = *(pos+step_dia_4);
          float v2 = *(pos+step_dia_1);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
          }
        } else if ( ang <= 292.5 ) {
          float v1 = *(pos+step_up);
          float v2 = *(pos+step_down);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
          }
        } else {
          float v1 = *(pos+step_dia_3);
          float v2 = *(pos+step_dia_2);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
          }
        }
      }
    }

    // Link/Select Edges
    vector< Contour* > cntrs;
    int label = 2;
    int bin_step = bin->widthStep / sizeof(char);
    step_dia_1 = -bin_step - 1;
    step_dia_2 = -bin_step + 1;
    step_dia_3 = bin_step - 1;
    step_dia_4 = bin_step + 1;
    step_up = bin_step;
    step_down = -bin_step;
    step_left = -1;
    step_right = 1;

    // Scan
    float best_mag = 0.0f;
    int best_ind = -1;
    for( int r = 1; r < r_range - 1; r++ ) {
      for( int c = 1; c < c_range - 1; c++ ) {

        // Check to see if we should skip this quadrant
        int octant = determine8quads( c + rel_c, r + rel_r );
        if( cd->is_side_border[octant] )
          continue;

        if( (bin->imageData + bin->widthStep*r)[c] == EDGEL ) {

          stack<ScanPoint> sq;
          ScanPoint pt(r,c);
          Contour *ctr = new Contour;
          sq.push( pt );

          while( sq.size() != 0 ) {

            pt = sq.top();
            int ir = pt.r;
            int ic = pt.c;
            char* pos = (bin->imageData + bin->widthStep*ir)+ic;
            *pos = label;
            ctr->pts.push_back( pt );
            sq.pop();

            // Check 8-connectedness
            if( *(pos+step_up) == EDGEL )
              sq.push( ScanPoint( ir+1, ic ) );
            if( *(pos+step_right) == EDGEL )
              sq.push( ScanPoint( ir, ic+1 ) );
            if( *(pos+step_down) == EDGEL )
              sq.push( ScanPoint( ir-1, ic ) );
            if( *(pos+step_left) == EDGEL )
              sq.push( ScanPoint( ir, ic-1 ) );
            if( *(pos+step_dia_2) == EDGEL )
              sq.push( ScanPoint( ir-1, ic+1 ) );
            if( *(pos+step_dia_1) == EDGEL )
              sq.push( ScanPoint( ir-1, ic-1 ) );
            if( *(pos+step_dia_4) == EDGEL )
              sq.push( ScanPoint( ir+1, ic+1 ) );
            if( *(pos+step_dia_3) == EDGEL )
              sq.push( ScanPoint( ir+1, ic-1 ) );
          }  

          ctr->label = label;

          // Init quadrant
          for( int p = 0; p<8; p++ )
            ctr->covers_oct[p] = false;

          // Calculate edge weight and what quadrants cntr is in
          float costsum = 0.0f;
          for( int k = 0; k < ctr->pts.size(); k++ ) {
            int r = ctr->pts[k].r;
            int c = ctr->pts[k].c;
            int ra = r + rel_r;
            int ca = c + rel_c;
            costsum += ((float*)(cost->imageData + cost->widthStep*r))[c];
            int oct = determine8quads( c+rel_c, r+rel_r );
            ctr->covers_oct[oct] = true;
          }
          ctr->mag = costsum;
          if( costsum > best_mag ) {
            best_mag = costsum;
            best_ind = cntrs.size();
          }
          cntrs.push_back( ctr );          
        }
      }
    }

    // ~~~~~ Basic Selection ~~~~~

/*#ifdef SS_DISPLAY 
    IplImage *temp = cvCloneImage( img_rgb_32f );

    for( int j = 0; j < cntrs.size(); j++ ) {
      for( int k = 0; k < cntrs[j]->pts.size(); k++ ) {
        cvSetAt( temp, cvScalar( 1, 0, 0 ), cntrs[j]->pts[k].r+lr,  cntrs[j]->pts[k].c+lc );
      }
    }
    showIP( img_rgb_32f, temp, cds[i] );
    cvReleaseImage( &temp );
#endif*/

    vector<Contour*> components;
    components.push_back( cntrs[best_ind] );
    bool oct_satisfied[8];
    for( int q = 0; q < 8; q++ ) {
      oct_satisfied[q] = cntrs[best_ind]->covers_oct[q] || cd->is_side_border[q];
    }
    cntrs.erase( cntrs.begin() + best_ind );

    for( int q = 0; q < 8; q++ ) {

      if( !oct_satisfied[q] ) {

        float max_val = 0.0f;
        int max_ind = -1;

        for( int c = 0; c < cntrs.size(); c++ ) {

          if( cntrs[c]->covers_oct[q] && cntrs[c]->mag > max_val ) {
            max_val = cntrs[c]->mag;
            max_ind = c;
          }
        }

        if( max_ind != -1 ) {
          Contour *ct = cntrs[max_ind];
          components.push_back( ct );
          cntrs.erase( cntrs.begin() + max_ind );
          for( int o = 0; o < 8; o++ ) {
            oct_satisfied[o] = oct_satisfied[o] || ct->covers_oct[o];
          }
        }
      }
    }

    // Remove potential outlier if # of components is large
    if( components.size() > 2 ) {
      float min = INF;
      int ind = -1;
      Contour *outlier;
      for( unsigned int i=0; i<components.size(); i++ ) {
        Contour *ct = components[i];
        if( ct->mag < min ) {
          min = ct->mag;
          ind = i;
          outlier = ct;
        }
      }
      components.erase( components.begin() + ind );
      delete outlier;
    }
    
    // Calculate total pts in identified Contours
    int total_pts = 0;
    for( int j = 0; j < components.size(); j++ )
      total_pts += components[j]->pts.size();

    // Regress ellipse if possible
    if( total_pts > 6 ) {
      cd->has_edge_features = true;
      CvPoint2D32f* input = (CvPoint2D32f*)malloc(total_pts*sizeof(CvPoint2D32f));
      int pos = 0;
      for( int j = 0; j < components.size(); j++ ) {
        for( int k = 0; k < components[j]->pts.size(); k++ ) {
          input[pos].x = components[j]->pts[k].c;
          input[pos].y = components[j]->pts[k].r;
          pos++;        
        }
      }
      CvBox2D* box = (CvBox2D*)malloc(sizeof(CvBox2D));
      cvFitEllipse( input, total_pts, box );

      // Set new location
      cd->nangle = box->angle;
      cd->nr = box->center.y;
      cd->nc = box->center.x;
      cd->nminor = box->size.width / 2;
      cd->nmajor = box->size.height / 2;

      // Adjust new position for offset
      cd->nr = cd->nr + lr;
      cd->nc = cd->nc + lc;

      // Deallocations
      free(input);
      free(box);

    } else {

      // Not enough edgel information
      cd->has_edge_features = false;
    }

#ifdef SS_DISPLAY 
    IplImage *temp = cvCloneImage( img_rgb_32f );

    for( int j = 0; j < components.size(); j++ ) {
      for( int k = 0; k < components[j]->pts.size(); k++ ) {
        cvSetAt( temp, cvScalar( 1, 0, 0 ), components[j]->pts[k].r+lr,  components[j]->pts[k].c+lc );
      }
    }
    CvScalar colour = cvScalar( 0.0, 0.0, 1.0 );
    cvEllipse(temp, cvPoint( (int)cd->nc, (int)cd->nr ), 
      cvSize( cd->nminor, cd->nmajor ), 
      cd->nangle, 0, 360, colour, 1 );

    showIP( img_rgb_32f, temp, cds[i] );
    cvReleaseImage( &temp );
#endif


        
    // Deallocations for this cd
    for( int a = 0; a < cntrs.size(); a++ )
      delete cntrs[a];
    for( int a = 0; a < components.size(); a++ )
      delete components[a];
    if( color != NULL )
      cvReleaseImage( &color );
    cvReleaseImage( &cost );
    cvReleaseImage( &bin );

  }  
}
