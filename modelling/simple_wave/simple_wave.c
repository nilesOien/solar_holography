
/* Simple Laplacian wave demo. */

#include <stdio.h>
#include "handy_routines.h"
#include <math.h>
#include <stdlib.h>

// Small routine to initialize the velocity field - pluck the violin string, as it were.
// Does its work in situ.
void init_vel(float *vel, int nx, int ny, float dx, float dy){

  int cx = nx/2;  // Center X pixel
  int cy = ny/2;  // ...and Y
  int ix, iy;     // Indicies
  float dist_x, dist_y; // Distance in X and Y
  float dist; // Distance to the center
  const float decay_const = 0.05;
  const float scale=0.1;

  for(ix=0; ix < nx; ix++){
    dist_x = fabsf(dx*(ix-cx));
    for(iy=0; iy<ny; iy++){
      dist_y = fabsf(dy*(iy-cy));
      dist=sqrt(dist_y*dist_y + dist_x*dist_x);
      if (dist < 150.0){
        vel[indexof(ix,iy, nx)] = scale*exp(-decay_const*dist*dist);
      }
    }
  }

  return;
}

// The main program entry point.
int main(int argc, char *argv[]){

  unsigned int nx=256; // Array size in X and Y
  unsigned int ny=256;
  float dx=0.5; // Array spacing in X and Y
  float dy=0.5;
  float c=1.0; // Wave speed - what we scale the Laplacian by to get acceleration.
  float timestep=0.25; // The timestep.
  // Various 2D fields that we will have to allocate space for.
  float *field, *field_velocity;
  float *grad_x_field, *grad_y_field;
  float *grad2_x_field, *grad2_y_field;
  float *laplacian;
  unsigned int ix; // General purpose index.
  unsigned int num_iter=650; // The number of iterations to run for.
  char ppm_filename[32];
  unsigned int iteration;
  float min,max;
  int pdi;
  float pdone;
  float accel;
  int cx=nx/2;
  int cy=ny/2;

  // Set aside space for various 2D work spaces.
  // The allocate_2d_array() routine calls calloc() under the hood so it is initialized to values of 0.0
  field = allocate_2d_array(nx, ny);          // The field.
  field_velocity = allocate_2d_array(nx, ny); // The rate of change of the field.
  grad_x_field = allocate_2d_array(nx, ny);   // Gradient of the field in the X direction
  grad_y_field = allocate_2d_array(nx, ny);   // Same in Y
  grad2_x_field = allocate_2d_array(nx, ny);  // Second derivative in X
  grad2_y_field = allocate_2d_array(nx, ny);  // ... and Y
  laplacian = allocate_2d_array(nx, ny);      // The Laplaciain which is proprtional to acceleration by wavr speed, c

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
    if (savePPM(ppm_filename, field, nx, ny, 0.0, 0.1)){
      fprintf(stderr,"Failed to save %s\n", ppm_filename);
      return -1;
    }

    /* Get first derivarive, in X and Y */
    grad_x(field, nx, ny, dx, grad_x_field);
    grad_y(field, nx, ny, dy, grad_y_field);

    /* Get second derivatives, and sum them to get the laplacian. */
    grad_x(grad_x_field, nx, ny, dx, grad2_x_field);
    grad_y(grad_y_field, nx, ny, dy, grad2_y_field);

    for (ix=0; ix < nx*ny; ix++){
      laplacian[ix] = grad2_x_field[ix] + grad2_y_field[ix];
    }

    smooth2D(laplacian,nx,ny,2);

    /* Update the velocity according to the laplacian acceleration, unless
     * it's the "start" iteration and we need to initialize velocity. */
    if (iteration == 10){
      // It's the start iteration - initialize
      init_vel(field_velocity, nx, ny, dx, dy);
    } else {
      // It's not the first iteration - update velocity accoring to the acceleration (from the scaled Laplacian)
      for (ix=0; ix < nx*ny; ix++){
        accel = c*laplacian[ix];
        field_velocity[ix] = field_velocity[ix] + accel*timestep;
      }
    }

    // Update the field from the field_velocity
    for (ix=0; ix < nx*ny; ix++){
      field[ix] = field[ix] + field_velocity[ix] * timestep;
    }

    smooth2D(field,nx,ny,2);

    ix=indexof(cx,cy,nx);
    fprintf(stdout,"Central field=%g, vel=%g, lap=%g\n", field[ix], field_velocity[ix], laplacian[ix]); 

    // ... and back we go to the top of the loop
    
  } // End of iterating loop


  fprintf(stdout, "Finishing C program\n");
  free(field);     free(grad_x_field);  free(grad_y_field);
  free(laplacian); free(grad2_x_field); free(grad2_y_field);
  free(field_velocity);

  return 0;

}

