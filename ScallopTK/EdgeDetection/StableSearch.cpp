
#include "StableSearch.h"

//------------------------------------------------------------------------------
//                               Benchmarking
//------------------------------------------------------------------------------

#ifdef SS_ENABLE_BENCHMARKINGING
  const string ss_bm_fn = "SSBMResults.dat";
  vector<double> ss_exe_times;
  ofstream ss_bm_output;
#endif

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

/*inline float distActFunc( float& normalizedDistOffset ) {
  if( normalizedDistOffset > 0.2 || normalizedDistOffset < -0.2 ) {
    return 0.4f;
  } else
    return 1.0f;
}

inline float dirActFunc( float& normalizedDirOffset ) {
  if( normalizedDirOffset > 30 || normalizedDirOffset < -30 ) {
    return 0.4f;
  } else
    return 1.0f;
}*/

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

void edgeSearch( GradientChain& Gradients, hfResults* color, IplImage *ImgLab32f, vector<Candidate*> cds, IplImage *rgb ) {

  // Debug Checks
  assert( color->SaliencyMap->width == ImgLab32f->width );

#ifdef SS_ENABLE_BENCHMARKINGING
  ss_bm_output.open( ss_bm_fn.c_str(), fstream::out | fstream::app );
  ss_exe_times.clear();
  initializeTimer();
  startTimer();
#endif

  IplImage *lab_ori = Gradients.dLabOri;
  IplImage *lab_mag = Gradients.dLabMag;

#ifdef SS_ENABLE_BENCHMARKINGING
  ss_exe_times.push_back( getTimeSinceLastCall() );
  int mark = ss_exe_times.size();
  for( int i=0; i<6; i++ )
    ss_exe_times.push_back( 0 );
#endif

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

    if( r_range < 1 || c_range < 1 ) {
      cds[i]->isActive = false;
      continue;
    }

    IplImage *cost = cvCreateImage( cvSize( c_range, r_range ), IPL_DEPTH_32F, 1 );

    float *grdmag = ((float*)(lab_mag->imageData + lab_mag->widthStep*lr))+lc;
    float *grddir = ((float*)(lab_ori->imageData + lab_ori->widthStep*lr))+lc;
    float *outptr = (float*)cost->imageData;

    int step = width - c_range;

    // Perform cost filtering
    int rel_r = -(cd->r - lr);
    int rel_c = -(cd->c - lc);
    float rad = cd->major;
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

    // Smooth cost func [opt]
    //cvSmooth( cost, cost, 2, 3, 3 );

#ifdef SS_ENABLE_BENCHMARKINGING
  ss_exe_times[mark] += getTimeSinceLastCall();
#endif

    // Non-max suppression and selection
    IplImage *bin = cvCreateImage( cvGetSize( cost ), IPL_DEPTH_8U, 1 );
    cvZero(bin);

    // Containers for max selection
    const char EDGEL = 255;
    int best_r[8], best_c[8];
    float best_mag[8];
    for( int b = 0; b < 8; b++ ) {
      best_r[b] = 0;
      best_c[b] = 0;
      best_mag[b] = 0.0f;
    }

    // NMS
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
            if( val > best_mag[0] ) {
              best_mag[0] = val;
              best_r[0] = r;
              best_c[0] = c;
            }
          }
        } else if ( ang <= 67.5 ) {
          float v1 = *(pos+step_dia_4);
          float v2 = *(pos+step_dia_1);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
            if( val > best_mag[1] ) {
              best_mag[1] = val;
              best_r[1] = r;
              best_c[1] = c;
            }
          }
        } else if ( ang <= 112.5 ) {
          float v1 = *(pos+step_up); 
          float v2 = *(pos+step_down);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
            if( val > best_mag[2] ) {
              best_mag[2] = val;
              best_r[2] = r; 
              best_c[2] = c;
            }
          }
        } else if ( ang <= 157.5 ) {
          float v1 = *(pos+step_dia_2);
          float v2 = *(pos+step_dia_3);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
            if( val > best_mag[3] ) {
              best_mag[3] = val;
              best_r[3] = r;
              best_c[3] = c;
            }
          }
        } else if ( ang <= 202.5 ) {
          float v1 = *(pos+step_right);
          float v2 = *(pos+step_left);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
            if( val > best_mag[4] ) {
              best_mag[4] = val;
              best_r[4] = r;
              best_c[4] = c;
            }
          }
        } else if ( ang <= 247.5 ) {
          float v1 = *(pos+step_dia_4);
          float v2 = *(pos+step_dia_1);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
            if( val > best_mag[5] ) {
              best_mag[5] = val;
              best_r[5] = r;
              best_c[5] = c;
            }
          }
        } else if ( ang <= 292.5 ) {
          float v1 = *(pos+step_up);
          float v2 = *(pos+step_down);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
            if( val > best_mag[6] ) {
              best_mag[6] = val;
              best_r[6] = r;
              best_c[6] = c;
            }
          }
        } else {
          float v1 = *(pos+step_dia_3);
          float v2 = *(pos+step_dia_2);
          if( v1 < val && v2 < val ) {
            (bin->imageData + bin->widthStep*r)[c] = EDGEL;
            if( val > best_mag[7] ) {
              best_mag[7] = val;
              best_r[7] = r;
              best_c[7] = c;
            }
          }
        }
      }
    }

#ifdef SS_ENABLE_BENCHMARKINGING
  ss_exe_times[mark+1] += getTimeSinceLastCall();
#endif

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

    // For each of our seed points
    for( int p = 0; p < 8; p++ ) {

      // Check to make sure we found a seed pt in this quadrant
      if( best_mag[p] == 0 || cd->isSideBorder[p] )
        continue;

      int r = best_r[p];
      int c = best_c[p];

      if( (bin->imageData + bin->widthStep*r)[c] == EDGEL ) {

        stack<Point2D> sq;
        Point2D pt(r,c);
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
            sq.push( Point2D( ir+1, ic ) );
          if( *(pos+step_right) == EDGEL )
            sq.push( Point2D( ir, ic+1 ) );
          if( *(pos+step_down) == EDGEL )
            sq.push( Point2D( ir-1, ic ) );
          if( *(pos+step_left) == EDGEL )
            sq.push( Point2D( ir, ic-1 ) );
          if( *(pos+step_dia_2) == EDGEL )
            sq.push( Point2D( ir-1, ic+1 ) );
          if( *(pos+step_dia_1) == EDGEL )
            sq.push( Point2D( ir-1, ic-1 ) );
          if( *(pos+step_dia_4) == EDGEL )
            sq.push( Point2D( ir+1, ic+1 ) );
          if( *(pos+step_dia_3) == EDGEL )
            sq.push( Point2D( ir+1, ic-1 ) );
        }  

        ctr->label = label;
        label++;
        
        // Calculate edge weight
        float costsum = 0.0f;
        for( int k = 0; k < ctr->pts.size(); k++ ) {
          int r = ctr->pts[k].r;
          int c = ctr->pts[k].c;
          int ra = r + rel_r;
          int ca = c + rel_c;
          costsum += ((float*)(cost->imageData + cost->widthStep*r))[c];
        }
        ctr->mag = costsum;
        cntrs.push_back( ctr );
      }
    }

#ifdef SS_ENABLE_BENCHMARKINGING
  ss_exe_times[mark+2] += getTimeSinceLastCall();
#endif
  
    // ~~~~~ Basic Analysis ~~~~~

    // Remove lowest cost edge (proabilistic outlier rejection) [OPT]  
    // TODO
  
    // Calculate total pts in identified Contours
    int total_pts = 0;
    for( int j = 0; j < cntrs.size(); j++ )
      total_pts += cntrs[j]->pts.size();

    // Regress ellipse if possible
    if( total_pts > 6 ) {
      cd->hasEdgeFeatures = true;
      CvPoint2D32f* input = (CvPoint2D32f*)malloc(total_pts*sizeof(CvPoint2D32f));
      int pos = 0;
      for( int j = 0; j < cntrs.size(); j++ ) {
        for( int k = 0; k < cntrs[j]->pts.size(); k++ ) {
          input[pos].x = cntrs[j]->pts[k].c;
          input[pos].y = cntrs[j]->pts[k].r;
          pos++;        
        }
      }
      CvBox2D* box = (CvBox2D*)malloc(sizeof(CvBox2D));
      cvFitEllipse( input, total_pts, box );

#ifdef SS_DISPLAY 
      IplImage *temp = cvCloneImage( rgb );
      Candidate *kp = new Candidate;
      kp->angle = box->angle;
      kp->r = box->center.y + lr;
      kp->c = box->center.x + lc;
      kp->minor = box->size.height/2;
      kp->major = box->size.width/2;
      kp->magnitude = 0;
      kp->method = ADAPTIVE;
      for( int j = 0; j < cntrs.size(); j++ ) {
        for( int k = 0; k < cntrs[j]->pts.size(); k++ ) {
          cvSetAt( temp, cvScalar( 1, 0, 0), cntrs[j]->pts[k].r+lr,  cntrs[j]->pts[k].c+lc );
        }
      }
      for( int j = 0; j < cntrs.size(); j++ ) {
        for( int k = 0; k < cntrs[j]->pts.size(); k++ ) {
          cvSetAt( temp, cvScalar( 0, 1, 0), cntrs[j]->pts[k].r+lr,  cntrs[j]->pts[k].c+lc );
        }
      }
      cvEllipse(temp, cvPoint( (int)kp->c, (int)kp->r ), 
        cvSize( kp->major, kp->minor ), 
        (kp->angle), 0, 360, cvScalar(0,0,1), 1 );

      if( cds[i]->major > 12 && cds[i]->major < 30 && cds[i]->isCorner )
        showIP( rgb, temp, cds[i] );
      cvReleaseImage( &temp );
      delete kp;
#endif

      // Set new location
      cd->nangle = box->angle;
      cd->nr = box->center.y;
      cd->nc = box->center.x;
      cd->nminor = box->size.width / 2;
      cd->nmajor = box->size.height / 2;

      // Calculate regional & overall MSE
      float MSE = 0.0;
      float regMSE[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
      float majsq = cd->nmajor * cd->nmajor;
      float minsq = cd->nminor * cd->nminor;
      float absq = majsq * minsq;
      float cosf = cos( (cd->nangle)*PI/180 );
      float sinf = sin( (cd->nangle)*PI/180 );
      int cr = cd->nr;
      int cc = cd->nc;
      int skipped = 0;
      int MSEcount[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
      for( int j = 0; j < cntrs.size(); j++ ) {
        for( int k = 0; k < cntrs[j]->pts.size(); k++ ) {
          int r = cntrs[j]->pts[k].r;
          int c = cntrs[j]->pts[k].c;          
          int posru = r-cr;
          int poscu = c-cc;
          int reg = determine8quads( poscu, posru );
          int posr = posru * cosf - poscu * sinf;
          int posc = poscu * cosf + posru * sinf;
          float rsq = posr * posr;
          float csq = posc * posc;
          float distsq = rsq + csq;
          if( distsq != 0 ) {
            float righthand = absq / ( minsq*rsq/distsq + majsq*csq/distsq );
            float eradius = sqrt( righthand );
            float error = (sqrt( distsq ) - eradius)/eradius;
            error = error * error;
            MSEcount[reg]++;
            MSE += error;
            regMSE[reg] += error;
          } else {
            skipped++;
          }
        }
      }
      for( int j = 0; j < 8; j++ )
        if( MSEcount[j] != 0 )
          regMSE[j] = regMSE[j] / MSEcount[j];
        else
          regMSE[j] = 1.0;
      MSE = MSE / (total_pts-skipped);

      // Insert into feature vector
      if( MSE > -2.5 && MSE < 2.5 )
        cd->edgeFeatures[0] = MSE;
      else 
        cd->edgeFeatures[0] = 2.5;
      for( int j=0; j<8; j++ )
        if( regMSE[j] > -2.5 && regMSE[j] < 2.5 )
          cd->edgeFeatures[j+1] = regMSE[j];
        else
          cd->edgeFeatures[j+1] = 2.5;
      
      // Calculate features around best 2 edge Contours

      //  - Identify 2 best edges (if exist)
      int best1 = -1;
      int best2 = -1;
      float best1mag = 0.0;
      float best2mag = 0.0;
      for( int p = 0; p < cntrs.size(); p++ ) {
        if( best1mag <= cntrs[p]->mag ) {
          best2 = best1;
          best1 = p;
          best2mag = best1mag;
          best1mag = cntrs[p]->mag;
        }
      }

      // First best edge
      if( best1 != -1 ) {

        // Calculate shift values
        const int SHIFTS = 6; // both directions        
        float avgL[2*SHIFTS+1];
        float avgA[2*SHIFTS+1];
        float avgB[2*SHIFTS+1];
        int counter[2*SHIFTS+1];
        int cntr_size = cntrs[best1]->pts.size();
        float *stepsr = new float[cntr_size];
        float *stepsc = new float[cntr_size];
        for( int p = 0; p < cntr_size; p++ ) {
          int r = cntrs[best1]->pts[p].r - cd->nr;
          int c = cntrs[best1]->pts[p].c - cd->nc;
          float d = sqrt( (float)r*r + c*c );
          stepsr[p] = 1.4f*r/d;
          stepsr[p] = 1.4f*c/d;
        }

        // Perform shifts
        int labwidth = ImgLab32f->width;
        int labheight = ImgLab32f->height;
        for( int s = -SHIFTS; s <= SHIFTS; s++ ) {
          int index = s+SHIFTS;
          avgL[index] = 0.0f;
          avgA[index] = 0.0f;
          avgB[index] = 0.0f;
          counter[index] = 0;
          for( unsigned int p = 0; p < cntr_size; p++ ) {
            int r = cntrs[best1]->pts[p].r + s*stepsr[p] + lr;
            int c = cntrs[best1]->pts[p].c + s*stepsc[p] + lc;
            if( r >= 0 && c >= 0 && r < labheight && c < labwidth ) {
              float *pos = ((float*)(ImgLab32f->imageData + r*ImgLab32f->widthStep))+3*c;
              avgL[index] += pos[0];
              avgA[index] += pos[1];
              avgB[index] += pos[2];
              counter[index]++;
            }
          }
        }

        // Normalize/Fill values
        if( counter[0] != 0 ) {
          avgL[0] = avgL[0] / counter[0];
          avgA[0] = avgA[0] / counter[0];
          avgB[0] = avgB[0] / counter[0];
        } else {
          avgL[0] = 0;
          avgA[0] = 0;
          avgB[0] = 0;
        }
        for( int s = 1; s <= SHIFTS; s++ ) {
          int index = s+SHIFTS;
          if( counter[index] != 0 ) {
            avgL[index] = avgL[index] / counter[index];
            avgA[index] = avgA[index] / counter[index];
            avgB[index] = avgB[index] / counter[index];
          } else {
            avgL[index] = avgL[index-1];
            avgA[index] = avgA[index-1];
            avgB[index] = avgB[index-1];
          }
        }
        for( int s = -1; s >= -SHIFTS; s-- ) {
          int index = s+SHIFTS;
          if( counter[index] != 0 ) {
            avgL[index] = avgL[index] / counter[index];
            avgA[index] = avgA[index] / counter[index];
            avgB[index] = avgB[index] / counter[index];
          } else {
            avgL[index] = avgL[index+1];
            avgA[index] = avgA[index+1];
            avgB[index] = avgB[index+1];
          }
        }
        
        // Deallocations
        delete[] stepsr;
        delete[] stepsc;

        // Insert into feature vector
        int pos = 9;
        cd->edgeFeatures[pos++] = (int)cd->hasEdgeFeatures;
        for( int j=2; j<=10; j+=2 ) {
          cd->edgeFeatures[pos++] = avgL[j];
          cd->edgeFeatures[pos++] = avgA[j];
          cd->edgeFeatures[pos++] = avgB[j];
        }
        cd->edgeFeatures[pos++] = (avgL[4]+avgL[6]+avgL[8])/3;
        cd->edgeFeatures[pos++] = (avgA[4]+avgA[6]+avgA[8])/3;
        cd->edgeFeatures[pos++] = (avgB[4]+avgB[6]+avgB[8])/3;
        int gradStart = pos;
        for( int j=0; j<12; j++ ) {
          cd->edgeFeatures[pos++] = avgL[j+1]-avgL[j];
          cd->edgeFeatures[pos++] = avgA[j+1]-avgA[j];
          cd->edgeFeatures[pos++] = avgB[j+1]-avgB[j];
        }
        cd->edgeFeatures[pos++] = avgL[8]-avgL[4];
        cd->edgeFeatures[pos++] = avgA[8]-avgA[4];
        cd->edgeFeatures[pos++] = avgB[8]-avgB[4];
        //Avg grad
        float avgGradL = 0.0f;
        float avgGradA = 0.0f;
        float avgGradB = 0.0f;
        for( int j=0; j<12; j++ ) {
          int strt = gradStart+j*3;
          avgGradL = avgGradL + cd->edgeFeatures[strt+0];
          avgGradA = avgGradA + cd->edgeFeatures[strt+1];
          avgGradB = avgGradB + cd->edgeFeatures[strt+2];
        }
        cd->edgeFeatures[pos++] = avgGradL/12;
        cd->edgeFeatures[pos++] = avgGradA/12;
        cd->edgeFeatures[pos++] = avgGradB/12;
        //Avg double deriv
        float avgDDL = 0.0f;
        float avgDDA = 0.0f;
        float avgDDB = 0.0f;
        for( int j=0; j<11; j++ ) {
          int strt = gradStart+j*3;
          int strt2 = strt+3;
          avgDDL += cd->edgeFeatures[strt2+0]-cd->edgeFeatures[strt+0];
          avgDDA += cd->edgeFeatures[strt2+1]-cd->edgeFeatures[strt+1];
          avgDDB += cd->edgeFeatures[strt2+2]-cd->edgeFeatures[strt+2];
        }
        cd->edgeFeatures[pos++] = avgDDL/11;
        cd->edgeFeatures[pos++] = avgDDA/11;
        cd->edgeFeatures[pos++] = avgDDB/11;

      } else {
        for( int j=9; j<73; j++ )
          cd->edgeFeatures[j] = 0;
      }

      // Second best entry
      if( best2 != -1 ) {

        // Calculate shift values
        const int SHIFTS = 6; // both directions        
        float avgL[2*SHIFTS+1];
        float avgA[2*SHIFTS+1];
        float avgB[2*SHIFTS+1];
        int counter[2*SHIFTS+1];
        int cntr_size = cntrs[best2]->pts.size();
        float *stepsr = new float[cntr_size];
        float *stepsc = new float[cntr_size];
        for( int p = 0; p < cntr_size; p++ ) {
          int r = cntrs[best2]->pts[p].r - cd->nr;
          int c = cntrs[best2]->pts[p].c - cd->nc;
          float d = sqrt( (float)r*r + c*c );
          stepsr[p] = 1.4f*r/d;
          stepsr[p] = 1.4f*c/d;
        }

        // Perform shifts
        int labwidth = ImgLab32f->width;
        int labheight = ImgLab32f->height;
        for( int s = -SHIFTS; s <= SHIFTS; s++ ) {
          int index = s+SHIFTS;
          avgL[index] = 0.0f;
          avgA[index] = 0.0f;
          avgB[index] = 0.0f;
          counter[index] = 0;
          for( unsigned int p = 0; p < cntr_size; p++ ) {
            int r = cntrs[best2]->pts[p].r + s*stepsr[p] + lr;
            int c = cntrs[best2]->pts[p].c + s*stepsc[p] + lc;
            if( r >= 0 && c >= 0 && r < labheight && c < labwidth ) {
              float *pos = ((float*)(ImgLab32f->imageData + r*ImgLab32f->widthStep))+3*c;
              avgL[index] += pos[0];
              avgA[index] += pos[1];
              avgB[index] += pos[2];
              counter[index]++;
            }
          }
        }

        // Normalize/Fill values
        if( counter[0] != 0 ) {
          avgL[0] = avgL[0] / counter[0];
          avgA[0] = avgA[0] / counter[0];
          avgB[0] = avgB[0] / counter[0];
        } else {
          avgL[0] = 0;
          avgA[0] = 0;
          avgB[0] = 0;
        }
        for( int s = 1; s <= SHIFTS; s++ ) {
          int index = s+SHIFTS;
          if( counter[index] != 0 ) {
            avgL[index] = avgL[index] / counter[index];
            avgA[index] = avgA[index] / counter[index];
            avgB[index] = avgB[index] / counter[index];
          } else {
            avgL[index] = avgL[index-1];
            avgA[index] = avgA[index-1];
            avgB[index] = avgB[index-1];
          }
        }
        for( int s = -1; s >= -SHIFTS; s-- ) {
          int index = s+SHIFTS;
          if( counter[index] != 0 ) {
            avgL[index] = avgL[index] / counter[index];
            avgA[index] = avgA[index] / counter[index];
            avgB[index] = avgB[index] / counter[index];
          } else {
            avgL[index] = avgL[index+1];
            avgA[index] = avgA[index+1];
            avgB[index] = avgB[index+1];
          }
        }
        
        // Deallocations
        delete[] stepsr;
        delete[] stepsc;

        // Insert into feature vector
        int pos = 73;
        cd->edgeFeatures[pos++] = (int)cd->hasEdgeFeatures;
        for( int j=2; j<=10; j+=2 ) {
          cd->edgeFeatures[pos++] = avgL[j];
          cd->edgeFeatures[pos++] = avgA[j];
          cd->edgeFeatures[pos++] = avgB[j];
        }
        cd->edgeFeatures[pos++] = (avgL[4]+avgL[6]+avgL[8])/3;
        cd->edgeFeatures[pos++] = (avgA[4]+avgA[6]+avgA[8])/3;
        cd->edgeFeatures[pos++] = (avgB[4]+avgB[6]+avgB[8])/3;
        int gradStart = pos;
        for( int j=0; j<12; j++ ) {
          cd->edgeFeatures[pos++] = avgL[j+1]-avgL[j];
          cd->edgeFeatures[pos++] = avgA[j+1]-avgA[j];
          cd->edgeFeatures[pos++] = avgB[j+1]-avgB[j];
        }
        cd->edgeFeatures[pos++] = avgL[8]-avgL[4];
        cd->edgeFeatures[pos++] = avgA[8]-avgA[4];
        cd->edgeFeatures[pos++] = avgB[8]-avgB[4];
        //Avg grad
        float avgGradL = 0.0f;
        float avgGradA = 0.0f;
        float avgGradB = 0.0f;
        for( int j=0; j<12; j++ ) {
          int strt = gradStart+j*3;
          avgGradL = avgGradL + cd->edgeFeatures[strt+0];
          avgGradA = avgGradA + cd->edgeFeatures[strt+1];
          avgGradB = avgGradB + cd->edgeFeatures[strt+2];
        }
        cd->edgeFeatures[pos++] = avgGradL/12;
        cd->edgeFeatures[pos++] = avgGradA/12;
        cd->edgeFeatures[pos++] = avgGradB/12;
        //Avg double deriv
        float avgDDL = 0.0f;
        float avgDDA = 0.0f;
        float avgDDB = 0.0f;
        for( int j=0; j<11; j++ ) {
          int strt = gradStart+j*3;
          int strt2 = strt+3;
          avgDDL += cd->edgeFeatures[strt2+0]-cd->edgeFeatures[strt+0];
          avgDDA += cd->edgeFeatures[strt2+1]-cd->edgeFeatures[strt+1];
          avgDDB += cd->edgeFeatures[strt2+2]-cd->edgeFeatures[strt+2];
        }
        cd->edgeFeatures[pos++] = avgDDL/11;
        cd->edgeFeatures[pos++] = avgDDA/11;
        cd->edgeFeatures[pos++] = avgDDB/11;

      } else {
        for( int j=73; j<137; j++ )
          cd->edgeFeatures[j] = 0;
      }

      // Adjust new position for offset
      cd->nr = cd->nr + lr;
      cd->nc = cd->nc + lc;

      // Deallocations
      free(input);
      free(box);

    } else {

      // Not enough edgel information
      cd->hasEdgeFeatures = false;
      for( int j=0; j<137; j++ )
        cd->edgeFeatures[j] = 0;
    }

#ifdef SS_ENABLE_BENCHMARKINGING
  ss_exe_times[mark+3] += getTimeSinceLastCall();
#endif
  
    // Deallocations for this cd
    for( int a = 0; a < cntrs.size(); a++ )
      delete cntrs[a];
    cvReleaseImage( &cost );
    cvReleaseImage( &bin );

#ifdef SS_ENABLE_BENCHMARKINGING
  ss_exe_times[mark+4] += getTimeSinceLastCall();
#endif

  }  

#ifdef SS_ENABLE_BENCHMARKINGING
  ss_exe_times.push_back( getTimeSinceLastCall() );  
#endif

#ifdef SS_ENABLE_BENCHMARKINGING
  for( int i = 0; i < ss_exe_times.size(); i++ )
    ss_bm_output << ss_exe_times[i] << " ";
  ss_bm_output << endl;
  ss_bm_output.close();
#endif
  
}

