
/* Simple Laplacian wave demo, with a perfectly matched layer (PML) absorbing boundary.
 *
 * The outer pml_width pixels on every side of the array form a damping layer whose
 * equations are matched to the interior, so waves enter it without reflecting and
 * are attenuated before they reach the array edge. Inside the layer, the velocity
 * is split into x and y parts (vx, vy) and the field gradients (wx, wy) are carried
 * as state, each damped by the sigma profile of its own axis:
 *
 *   d(vx)/dt = -sigma_x*vx + c * d(wx)/dx       d(wx)/dt = -sigma_x*wx + d(v)/dx
 *   d(vy)/dt = -sigma_y*vy + c * d(wy)/dy       d(wy)/dt = -sigma_y*wy + d(v)/dy
 *   v = vx + vy,   field += v*dt
 *
 * With sigma = 0 (the interior) this is exactly the original wave equation
 * d2(field)/dt2 = c * Laplacian(field). Note c multiplies the Laplacian, so the wave
 * speed is sqrt(c).
 *
 * The per-step smooth2D() calls of the original code are commented out (see
 * SMOOTHING below): the box smoothing is numerical diffusion that is not compatible
 * with the PML and leaves a residual reflection. */

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

// Build the PML coefficients for one axis of n pixels with spacing h.
// Each pixel gets a damping rate sigma that is 0 in the interior and rises as
// (depth/width)^pml_power through the layer, and the exact-exponential update factors
//   a = exp(-sigma*dt)   and   b = (1-a)/sigma   (b = dt where sigma = 0)
// so that  q_new = a*q + b*source  integrates  dq/dt = -sigma*q + source.
// The peak sigma is set so that the theoretical round trip reflection is pml_reflect.
void pml_coefficients(float *a, float *b, int n, float h, int width, float speed,
                      float timestep, float pml_power, float pml_reflect){
  int i;
  float depth, sigma, sigma_max=0.0;
  if (width > 0) sigma_max = (pml_power+1.0)*speed*(-log(pml_reflect))/(2.0*width*h);
  for (i=0; i < n; i++){
    depth=0.0;
    if (width > 0){
      if (i < width) depth = width - i;                          // Low edge
      else if (i > n-1-width) depth = i - (n-1-width);           // High edge
    }
    sigma = sigma_max*pow(depth/(float)(width > 0 ? width : 1), pml_power);
    if (sigma > 0.0){
      a[i] = exp(-sigma*timestep);
      b[i] = (1.0-a[i])/sigma;
    } else {
      a[i] = 1.0;
      b[i] = timestep;
    }
  }
  return;
}

// The main program entry point.
int main(int argc, char *argv[]){

  unsigned int nx=512; // Array size in X and Y
  unsigned int ny=512;
  float dx=0.5; // Array spacing in X and Y
  float dy=0.5;
  float c=1.0; // Wave speed - what we scale the Laplacian by to get acceleration.
  float timestep=0.25; // The timestep.
  // Various 2D fields that we will have to allocate space for.
  float *field, *field_velocity;
  float *field_velocity_x, *field_velocity_y; // Split velocity (field_velocity = x part + y part)
  float *field_grad_x, *field_grad_y;         // Damped gradient of the field, carried as state
  float *work_x, *work_y;                     // Work space for derivatives
  float *ax, *bx, *ay, *by;                   // PML update coefficients along X and Y
  unsigned int iy;
  // PML parameters.
  int pml_width=24;          // Layer thickness in pixels, on each side. Set to 0 to disable the PML.
  float pml_power=2.0;       // Damping profile is (depth/width)^pml_power
  float pml_reflect=1.0e-6;  // Target theoretical reflection of the layer
  unsigned int ix; // General purpose index.
  unsigned int num_iter=1000; // The number of iterations to run for.
  char ppm_filename[32];
  unsigned int iteration;
  float min,max;
  int pdi;
  float pdone;
  int k;
  int cx=nx/2;
  int cy=ny/2;

  // Set aside space for various 2D work spaces.
  // The allocate_2d_array() routine calls calloc() under the hood so it is initialized to values of 0.0
  field = allocate_2d_array(nx, ny);          // The field.
  field_velocity = allocate_2d_array(nx, ny); // The rate of change of the field.
  field_velocity_x = allocate_2d_array(nx, ny); // X part of the velocity
  field_velocity_y = allocate_2d_array(nx, ny); // Y part of the velocity
  field_grad_x = allocate_2d_array(nx, ny);   // Gradient of the field in X (damped in the PML)
  field_grad_y = allocate_2d_array(nx, ny);   // Same in Y
  work_x = allocate_2d_array(nx, ny);         // Derivative work space
  work_y = allocate_2d_array(nx, ny);
  ax = (float *)malloc(nx*sizeof(float)); bx = (float *)malloc(nx*sizeof(float));
  ay = (float *)malloc(ny*sizeof(float)); by = (float *)malloc(ny*sizeof(float));
  pml_coefficients(ax, bx, nx, dx, pml_width, sqrtf(c), timestep, pml_power, pml_reflect);
  pml_coefficients(ay, by, ny, dy, pml_width, sqrtf(c), timestep, pml_power, pml_reflect);

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

    /* Acceleration terms: d(wx)/dx and d(wy)/dy, where wx, wy are the (PML damped)
     * gradients of the field. Their sum is the Laplacian of the field. */
    grad_x(field_grad_x, nx, ny, dx, work_x);
    grad_y(field_grad_y, nx, ny, dy, work_y);

    /* SMOOTHING: the original code smoothed the Laplacian here, and the field below.
     * Disabled, since it is not compatible with the PML.
    smooth2D(work_x,nx,ny,2);
    smooth2D(work_y,nx,ny,2);
     */

    /* Update the velocity according to the laplacian acceleration, unless
     * it's the "start" iteration and we need to initialize velocity. */
    if (iteration == 10){
      // It's the start iteration - initialize. Split the velocity evenly between X and Y parts.
      init_vel(field_velocity, nx, ny, dx, dy);
      for (ix=0; ix < nx*ny; ix++){
        field_velocity_x[ix] = 0.5*field_velocity[ix];
        field_velocity_y[ix] = 0.5*field_velocity[ix];
      }
    } else {
      // It's not the first iteration - update velocity according to the acceleration (from the scaled Laplacian)
      // with the PML damping applied along each axis.
      for (iy=0; iy < ny; iy++){
        for (ix=0; ix < nx; ix++){
          k = indexof(ix,iy,nx);
          field_velocity_x[k] = ax[ix]*field_velocity_x[k] + bx[ix]*c*work_x[k];
          field_velocity_y[k] = ay[iy]*field_velocity_y[k] + by[iy]*c*work_y[k];
          field_velocity[k] = field_velocity_x[k] + field_velocity_y[k];
        }
      }
    }

    // Update the gradient state from the velocity (with PML damping)
    grad_x(field_velocity, nx, ny, dx, work_x);
    grad_y(field_velocity, nx, ny, dy, work_y);
    for (iy=0; iy < ny; iy++){
      for (ix=0; ix < nx; ix++){
        k = indexof(ix,iy,nx);
        field_grad_x[k] = ax[ix]*field_grad_x[k] + bx[ix]*work_x[k];
        field_grad_y[k] = ay[iy]*field_grad_y[k] + by[iy]*work_y[k];
      }
    }

    // Update the field from the field_velocity
    for (ix=0; ix < nx*ny; ix++){
      field[ix] = field[ix] + field_velocity[ix] * timestep;
    }

    /* SMOOTHING: disabled, see above.
    smooth2D(field,nx,ny,2);
     */

    ix=indexof(cx,cy,nx);
    fprintf(stdout,"Central field=%g, vel=%g\n", field[ix], field_velocity[ix]); 

    // ... and back we go to the top of the loop
    
  } // End of iterating loop


  fprintf(stdout, "Finishing C program\n");
  free(field); free(field_velocity);
  free(field_velocity_x); free(field_velocity_y);
  free(field_grad_x); free(field_grad_y);
  free(work_x); free(work_y);
  free(ax); free(bx); free(ay); free(by);

  return 0;

}

