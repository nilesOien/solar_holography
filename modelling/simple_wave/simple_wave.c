
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
  float c=1.0;
  float timestep=1.0;
  float *grad_x_field, *grad_y_field;
  float *grad2_x_field, *grad2_y_field;
  float *laplacian;
  unsigned int ix, iy;
  unsigned int num_iter=100;
  char ppm_filename[32];
  unsigned int iteration;
  float min,max, minAdj, maxAdj;
  int first;
  float adj;
  int pdi;
  float pdone;
  int nx10=nx;
  float pi=3.1415927;
  float val;
  float t;


  field = allocate_2d_array(nx, ny);
  grad_x_field = allocate_2d_array(nx, ny);
  grad_y_field = allocate_2d_array(nx, ny);
  grad2_x_field = allocate_2d_array(nx, ny);
  grad2_y_field = allocate_2d_array(nx, ny);
  laplacian = allocate_2d_array(nx, ny);

  /* Set up a field that has a disturbance at the left side. */
  for (ix=0; ix < nx10; ix++){
    t=1.0*ix/(nx10-1.0);
    t=1.0 - t;
    val=sin(8.0*t*pi);
    for (iy=0; iy < ny; iy++){
      field[indexof(ix,iy,nx)] = val;
    }
  }

  for (iteration=0; iteration<num_iter; iteration++){

    max=min=field[0];
    for (ix=1; ix<nx*ny; ix++){
      if (field[ix] > max) max=field[ix];
      if (field[ix] < min) min=field[ix];
    }
    pdone=100.0*iteration/(float)(num_iter-1);
    pdi=(int)rint(pdone);
    fprintf(stdout, "Iteration %08d of %08d field runs from %g to %g (%d percent)\n", iteration, num_iter, min, max,pdi);

    sprintf(ppm_filename, "images/iter_%08d.ppm", iteration);
    if (savePPM(ppm_filename, field, nx, ny, 0.0, 0.0)){
      fprintf(stderr,"Failed to save %s\n", ppm_filename);
      return -1;
    }

    /* Get first derivarive, in X and Y */
    grad_x(field, nx, ny, dx, grad_x_field);

    sprintf(ppm_filename, "images/grad_x_%08d.ppm", iteration);
    if (savePPM(ppm_filename, grad_x_field, nx, ny, 0.0, 0.0)){
      fprintf(stderr,"Failed to save %s\n", ppm_filename);
      return -1;
    }



    grad_y(field, nx, ny, dy, grad_y_field);

    sprintf(ppm_filename, "images/grad_y_%08d.ppm", iteration);
    if (savePPM(ppm_filename, grad_y_field, nx, ny, 0.0, 0.0)){
      fprintf(stderr,"Failed to save %s\n", ppm_filename);
      return -1;
    }


    /* Get second derivatives, and sum them to get the laplacian. */
    grad_x(grad_x_field, nx, ny, dx, grad2_x_field);

    sprintf(ppm_filename, "images/grad2_x_%08d.ppm", iteration);
    if (savePPM(ppm_filename, grad2_x_field, nx, ny, 0.0, 0.0)){
      fprintf(stderr,"Failed to save %s\n", ppm_filename);
      return -1;
    }



    grad_y(grad_y_field, nx, ny, dy, grad2_y_field);

    sprintf(ppm_filename, "images/grad2_y_%08d.ppm", iteration);
    if (savePPM(ppm_filename, grad2_y_field, nx, ny, 0.0, 0.0)){
      fprintf(stderr,"Failed to save %s\n", ppm_filename);
      return -1;
    }




    for (ix=0; ix < nx*ny; ix++){
      laplacian[ix] = grad2_x_field[ix] + grad2_y_field[ix];
    }

    smooth2D(laplacian,nx,ny,2);

    sprintf(ppm_filename, "images/lap_%08d.ppm", iteration);
    if (savePPM(ppm_filename, laplacian, nx, ny, 0.0, 0.0)){
      fprintf(stderr,"Failed to save %s\n", ppm_filename);
      return -1;
    }

    /* Update the field using the acceleration from the laplacian */
    first=1;
    for (ix=0; ix < nx*ny; ix++){
      adj = -c*c*laplacian[ix]*timestep*timestep;
      // fprintf(stdout,"Adustment to %g is %g\n", field[ix], adj);
      field[ix] = field[ix] + adj;
      if (first){
        first=0;
        minAdj=adj;
        maxAdj=adj;
      } else {
        if (adj < minAdj) minAdj = adj;
        if (adj > maxAdj) maxAdj = adj;
      }
    }
    fprintf(stdout, "Adjustments run from %g to %g\n", minAdj, maxAdj);

  }

  FILE *fp = fopen("huh.dat", "w");
  iy=ny/2;
  for (ix=0; ix < nx; ix++){
    fprintf(fp,"%d %g\n", ix, field[indexof(ix,iy, nx)]);
  }
  fclose(fp);

  fprintf(stdout, "Finishing C program\n");
  free(field);     free(grad_x_field);  free(grad_y_field);
  free(laplacian); free(grad2_x_field); free(grad2_y_field);


  return 0;

}

