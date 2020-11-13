/* trieres_harris_numpi.i */

%module trieres_harris_numpi

%{

#include "../../../cplusplus/include/config.h"
#define SWIG_FILE_WITH_INIT

extern void harris(int total_size, unsigned char *input_img, int total_size2, unsigned char *output_img, char *s_servAddress, char *s_servPort);
%}

%include "numpy.i"

%init %{
     import_array();
%}

%apply (int DIM1, unsigned char* IN_ARRAY1) {(int total_size, unsigned char* input_img)}
%apply (int DIM1, unsigned char* ARGOUT_ARRAY1) {(int total_size2, unsigned char* output_img)};

extern void harris(int total_size, unsigned char *input_img, int total_size2, unsigned char *output_img, char *s_servAddress, char *s_servPort);
