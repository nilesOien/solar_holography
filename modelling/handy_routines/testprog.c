
/* Test the handy routines. */

#include <stdio.h>
#include "handy_routines.h"
#include <math.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

  float *field;
  unsigned int nx=800;
  unsigned int ny=400;
  float dx=0.5;
  float dy=0.5;
  float *grad_x_field, *grad_y_field;
  float *grad2_x_field, *grad2_y_field;
  float *laplacian;
  unsigned int ix, iy;
  float pi=3.1415927;
  float distY;

  fprintf(stdout, "Testing handy routines\n");

  field = allocate_2d_array(nx, ny);
  grad_x_field = allocate_2d_array(nx, ny);
  grad_y_field = allocate_2d_array(nx, ny);
  grad2_x_field = allocate_2d_array(nx, ny);
  grad2_y_field = allocate_2d_array(nx, ny);
  laplacian = allocate_2d_array(nx, ny);


  /* Set up a field that varies sinusoidally in X and exponentially in negative Y squared */
  for (ix=0; ix < nx; ix++){
    for (iy=0; iy < ny; iy++){
      distY=iy;
      distY = distY - ny/2;
      distY = distY / (ny/2); 
      field[indexof(ix,iy,nx)] = 5.0*cos(4.0*pi*ix/(nx-1)) + 5.0 * exp(-8.0*distY*distY);
    }
  }

  fprintf(stdout, "Saving field.ppm\n");
  if (savePPM("field.ppm", field, nx, ny, 0.0, 0.0)){
    fprintf(stderr,"Failed to save field.ppm");
  }

  fprintf(stdout, "Saving grad_x.ppm\n");
  grad_x(field, nx, ny, dx, grad_x_field);
  if (savePPM("grad_x.ppm", grad_x_field, nx, ny, 0.0, 0.0)){
    fprintf(stderr,"Failed to save grad_x.ppm");
  }

  fprintf(stdout, "Saving grad_y.ppm\n");
  grad_y(field, nx, ny, dy, grad_y_field);
  if (savePPM("grad_y.ppm", grad_y_field, nx, ny, 0.0, 0.0)){
    fprintf(stderr,"Failed to save grad_y.ppm");
  }

  /* Get second derivatives, and sum them. */
  grad_x(grad_x_field, nx, ny, dx, grad2_x_field);
  grad_y(grad_y_field, nx, ny, dy, grad2_y_field);
  for (ix=0; ix < nx*ny; ix++){
    laplacian[ix] = grad2_x_field[ix] + grad2_y_field[ix];
  }
  fprintf(stdout, "Saving laplacian.ppm\n");
  if (savePPM("laplacian.ppm", laplacian, nx, ny, 0.0, 0.0)){
    fprintf(stderr,"Failed to save laplacian.ppm");
  }

  fprintf(stdout, "Finishing C program\n");
  free(field);     free(grad_x_field);  free(grad_y_field);
  free(laplacian); free(grad2_x_field); free(grad2_y_field);


  return 0;

}

