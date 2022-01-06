/* trieres_median_blur_numpi.i */

/*******************************************************************************
 * Copyright 2016 -- 2022 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*******************************************************************************/

%module trieres_median_blur_numpi

%{

#include "../../../cplusplus/include/config.h"
#define SWIG_FILE_WITH_INIT

extern void median_blur(int total_size, unsigned char *input_img, int total_size2, unsigned char *output_img, char *s_servAddress, char *s_servPort);
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

extern void median_blur(int total_size, unsigned char *input_img, int total_size2, unsigned char *output_img, char *s_servAddress, char *s_servPort);
