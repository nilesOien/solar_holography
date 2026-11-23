
// Structure used to hold solar atmospheric data.
typedef struct {
 int num_entries;
 float *depth;
 float *m_on_M;
 float *pressure;
 float *rho;
 float *temperature;
 float *soundspeed;
 float *gravity;
 float *A;
 float *H_rho_on_R;
 float *N_squared;
} atmosphere_t;    

// Forward prototypes.

// Read the data from an interpolated file.
int read_atmosphere(char *filename, atmosphere_t *atmos);

// Print the data
void print_atmosphere(atmosphere_t atmos);

// Free up the memory associated with the data.
void free_atmosphere(atmosphere_t atmos);

