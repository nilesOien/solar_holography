 
#include <stdio.h>
#include <stdlib.h>
#include "read_atmosphere.h"

// Function to read the interpolated solar atmosphere from a file.
int read_atmosphere(char *filename, atmosphere_t *atmos){

 // Open the file.
 FILE *fp = fopen(filename,"r");
 if (fp == NULL){
  fprintf(stderr, "Failed to open atmosphere file %s\n", filename);
  return -1;
 }

 // Count the number of lines (including header line)
 char line[1024];
 int num_entries=0;
 while(NULL != fgets(line, 1024, fp)){
  num_entries++;
 }
 num_entries--; // One of the lines is the header
 rewind(fp); 

 // Allocate space.
 atmos->depth       = malloc(num_entries * sizeof(float));
 atmos->m_on_M      = malloc(num_entries * sizeof(float));
 atmos->pressure    = malloc(num_entries * sizeof(float));
 atmos->rho         = malloc(num_entries * sizeof(float));
 atmos->temperature = malloc(num_entries * sizeof(float));
 atmos->soundspeed  = malloc(num_entries * sizeof(float));
 atmos->gravity     = malloc(num_entries * sizeof(float));
 atmos->A           = malloc(num_entries * sizeof(float));
 atmos->H_rho_on_R  = malloc(num_entries * sizeof(float));
 atmos->N_squared   = malloc(num_entries * sizeof(float));

 // Read the data. The entries are :
 // depth m_on_M pressure rho temperature soundspeed gravity A H_rho_on_R N_squared    
 int line_num=0;
 while(NULL != fgets(line, 1024, fp)){
  line_num++;
  if (line_num == 1) continue; // Skip header line.

  int i = line_num-1;
  if (10 != sscanf(line,"%f %f %f %f %f %f %f %f %f %f",
                   atmos->depth+i, atmos->m_on_M+i, atmos->pressure+i, atmos->rho+i,
                   atmos->temperature+i, atmos->soundspeed+i, atmos->gravity+i,
                   atmos->A+i, atmos->H_rho_on_R+i, atmos->N_squared+i)){
   fclose(fp);
   fprintf(stderr, "Failed to read line %d of %s\n", line_num, filename);
   return -1;
  }
 }

 atmos->num_entries = num_entries; 

 fclose(fp);

 fprintf(stdout, "%d atmospheric entries read from %s\n", num_entries, filename);

 return 0;

}

// Function to print the solar atmosphere that we read.
void print_atmosphere(atmosphere_t atmos){

 fprintf(stdout, "%d entries in atmosphere :\n", atmos.num_entries);
 for(int i=0; i < atmos.num_entries; i++){
  fprintf(stdout,"%d depth=%g soundspeed=%g rho=%g pressure=%g\n",
                  i+1, atmos.depth[i], atmos.soundspeed[i], atmos.rho[i], atmos.pressure[i]);
 }
 return;

}


// Function to free up the atmos structure after use.
void free_atmosphere(atmosphere_t atmos){
 free(atmos.depth);
 free(atmos.m_on_M);
 free(atmos.pressure);
 free(atmos.rho);
 free(atmos.temperature);
 free(atmos.soundspeed);
 free(atmos.gravity);
 free(atmos.A);
 free(atmos.H_rho_on_R);
 free(atmos.N_squared);
 return;
}


