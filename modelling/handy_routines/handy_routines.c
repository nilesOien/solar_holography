
/* Include stdlib.h for calloc(), stdio.h for I/O, and math.h for rint() */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Allocate space for a two dimentional array. calloc() will
 * initialize it to 0. */
float *allocate_2d_array(unsigned int nx, unsigned int ny){
  return (float *)calloc(nx*ny, sizeof(float));
}

/* Translate two dimensional indicies to a one dimensional one
 * for use in our arrays, which are one dimensional under the hood.
 * Array starts at top left with X changing fastest in memory. */
int indexof(unsigned int ix, unsigned int iy, unsigned int nx){
  return iy*nx + ix;
}

/* Calculate slope in the X direction by adjacent pixel subtraction.
 * It is assumed that arrays have already been allocated. */
void grad_x(float *field, unsigned int nx, unsigned int ny, float dx, float *output){
  int done;
  for (unsigned int ix=0; ix < nx; ix++){
    for (unsigned int iy=0; iy < ny; iy++){
      /* Deal with the edge cases : left edge? */ 
      done=0;
      if (ix == 0){
        output[indexof(ix,iy,nx)] = (field[indexof(ix+1,iy,nx)] - field[indexof(ix,iy,nx)])/dx;
        done=1;
      }
      /* Right edge? */
      if (ix == nx-1){
        output[indexof(ix,iy,nx)] = (field[indexof(ix,iy,nx)] - field[indexof(ix-1,iy,nx)])/dx;
        done=1;
      }
      /* Somewhere in the middle? */
      if (done == 0){
        output[indexof(ix,iy,nx)] = (field[indexof(ix+1,iy,nx)] - field[indexof(ix-1,iy,nx)])/(2.0*dx);
      }
    }
  }
  return;
}




/* Similar, slope in the Y direction. */
void grad_y(float *field, unsigned int nx, unsigned int ny, float dy, float *output){
  int done;
  for (unsigned int ix=0; ix < nx; ix++){
    for (unsigned int iy=0; iy < ny; iy++){
      /* Deal with the edge cases : top edge? */ 
      done=0;
      if (iy == 0){
        output[indexof(ix,iy,nx)] = (field[indexof(ix,iy+1,nx)] - field[indexof(ix,iy,nx)])/dy;
        done=1;
      }
      /* Bottom edge? */
      if (iy == ny-1){
        output[indexof(ix,iy,nx)] = (field[indexof(ix,iy,nx)] - field[indexof(ix,iy-1,nx)])/dy;
        done=1;
      }
      /* Somewhere in the middle? */
      if (done == 0){
        output[indexof(ix,iy,nx)] = (field[indexof(ix,iy+1,nx)] - field[indexof(ix,iy-1,nx)])/(2.0*dy);
      }
    }
  }
  return;
}



/* Save a PPM image.  Returns 0 if it went OK. Setting min=max=0.0 means take range from data. */
int savePPM(char *filename, float *field, unsigned int nx, unsigned int ny, float min, float max){

  FILE *fp;
  float minInUse, maxInUse;
  int val;

  fp = fopen(filename, "w");
  if (fp == NULL){
    fprintf(stderr, "Failed to create PPM file %s\n", filename);
    return -1;
  }

  if (min == 0.0 && max == 0.0){ /* We take this to mean we should take the range from the data. */
    minInUse=field[0];
    maxInUse=field[0];
    for (unsigned int i=1; i < nx*ny; i++){
      if (field[i] < minInUse) minInUse=field[i];
      if (field[i] > maxInUse) maxInUse=field[i];
    }
    fprintf(stdout, "Data for image %s run from %g to %g\n", filename, minInUse, maxInUse);
    if (minInUse == maxInUse){
      /* A constant field has been passed in, widen the range so as to avoid divide by 0.0 */
      minInUse = minInUse - 5.0;
      maxInUse = maxInUse + 5.0;
    }
  } else { /* We use the specified values. */
    minInUse=min; maxInUse=max;
  }

  fprintf(fp, "P3\n%d %d\n255\n", nx, ny);

  for (unsigned int iy=0; iy < ny; iy++){
    for (unsigned int ix=0; ix < nx; ix++){
      val=(int)rint(255.0*(field[indexof(ix,iy,nx)]-minInUse)/(maxInUse-minInUse));
      if (val < 0) val=0;
      if (val > 255) val=255;
      fprintf(fp," %d %d %d ", val, val, val);
    }
    fprintf(fp,"\n");
  }

  fclose(fp);

  return 0;
}

/* Smooth a 2D field by averaging over a centered window. width_index is half width 
in pixels, setting it to 1 means smoothing will run over i-1 to 1+1, a 3x3 box. */
void smooth2D(float *field, unsigned int nx, unsigned int ny, int width_index){

  float *work;

  work = allocate_2d_array(nx, ny);

  for (int ix=0; ix < nx; ix++){
    //fprintf(stdout,"\nix is %d\n", ix);
    for (int iy=0; iy < ny; iy++){
      //fprintf(stdout, "iy is %d\n", iy);
      float total=0.0;
      int num=0;
      for (int ixx = ix - width_index; ixx <= ix + width_index; ixx++){
        //fprintf(stdout, "ixx is %d\n", ixx);
        for(int iyy = iy - width_index; iyy <= iy + width_index; iyy++){
          //fprintf(stdout, "iyy is %d\n", iyy);
          if ((ixx >= 0) && (iyy >= 0) && (ixx < nx) && (iyy < ny)){
            num=num+1;
            //fprintf(stdout, "Accepted, num is now %d\n", num);
            total=total+field[indexof(ixx,iyy,nx)];
          } else {
            //fprintf(stdout,"Rejected\n");
          }
        }
      }
      // fprintf(stdout,"%d %g\n", num, total);
      work[indexof(ix,iy,nx)]=total/num; /* num can't be 0 */
    }
  }


  for(unsigned int i=0; i < nx*ny; i++)
    field[i]=work[i];

  free(work);

  return;
}





