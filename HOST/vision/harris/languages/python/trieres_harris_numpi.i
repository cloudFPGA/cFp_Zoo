/* trieres_harris_numpi.i */

%module trieres_harris_numpi
%include "cstring.i"

%{

#include "../../../cplusplus/include/config.h"
#define SWIG_FILE_WITH_INIT

extern int harris(int total_size, uint8_t *input_img, char *s_servAddress, char *s_servPort);
%}

%include "numpy.i"

%init %{
     import_array();
%}

%apply (int DIM1, uint8_t* ARGOUT_ARRAY1) {(int total_size, uint8_t *input_img)};

extern int harris(int total_size, uint8_t *input_img, char *s_servAddress, char *s_servPort);