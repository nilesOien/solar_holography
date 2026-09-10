
/* Allocate space for a two dimentional array. calloc() will
 * initialize to 0. */
float *allocate_2d_array(unsigned int nx, unsigned int ny);

/* Translate two dimensional indicies to a one dimensional one
 * for use in our arrays, which are one dimensional under the hood.
 * Array starts at top left with X changing fastest in memory. */
int indexof(unsigned int ix, unsigned int iy, unsigned int nx);

/* Calculate slope in the X direction by adjacent pixel subtraction.
 * It is assumed that arrays have already been allocated. */
void grad_x(float *field, unsigned int nx, unsigned int ny, float dx, float *output);

/* Similar, slope in the Y direction. */
void grad_y(float *field, unsigned int nx, unsigned int ny, float dy, float *output);

/* Save a PPM image.  Returns 0 if it went OK. Setting min=max=0.0 means take range from data. */
int savePPM(char *filename, float *field, unsigned int nx, unsigned int ny, float min, float max);

/* Smooth a 2D field by averaging over a centered window. width_index is half width
in pixels, setting it to 1 means smoothing will run over i-1 to 1+1, a 3x3 box. */
void smooth2D(float *field, unsigned int nx, unsigned int ny, int width_index);

