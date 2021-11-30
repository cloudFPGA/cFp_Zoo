/* trieres_warp_transform_numpi.i */

%module trieres_warp_transform_numpi

%{

#include "../../../cplusplus/include/config.h"
#define SWIG_FILE_WITH_INIT

extern void warp_transform(int total_size, unsigned char *input_img, int total_size2, unsigned char *output_img, char *s_servAddress, char *s_servPort);
%}

%include "numpy.i"
/*
%typemap(python,out) BOOLAPI {
        $target = Py_None;
        Py_INCREF(Py_None);
}

%typemap(python,except) DWORDAPI {
      Py_BEGIN_ALLOW_THREADS
      $function
      Py_END_ALLOW_THREADS
      if ($source!=0)  {
           $cleanup
           return PyWin_SetAPIError("$name", $source);
      }
}
*/

%init %{
     import_array();
%}

/*
https://www.oreilly.com/library/view/python-cookbook/0596001673/ch16s10.html
function or action
*/

%exception {
    Py_BEGIN_ALLOW_THREADS
    $action
    Py_END_ALLOW_THREADS
}


%apply (int DIM1, unsigned char* IN_ARRAY1) {(int total_size, unsigned char* input_img)}
%apply (int DIM1, unsigned char* ARGOUT_ARRAY1) {(int total_size2, unsigned char* output_img)};

extern void warp_transform(int total_size, unsigned char *input_img, int total_size2, unsigned char *output_img, char *s_servAddress, char *s_servPort);
