
// Small program to test the atmosphere reading routines.

#include <stdio.h>
#include "read_atmosphere.h"

int main(int argc, char *argv[]){

 if (argc < 2){
  fprintf(stderr, "An input filename is required on the command line.\n");
  return -1;
 }
 char *filename=argv[1];


 atmosphere_t atmos;
 int ret = read_atmosphere(filename, &atmos);
 if (ret != 0){
  fprintf(stderr,"Read failed\n");
  return -1;
 }

 print_atmosphere(atmos);

 free_atmosphere(atmos);

 return 0;

}
