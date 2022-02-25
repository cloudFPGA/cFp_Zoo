/* trieres_harris_filename.i */

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

%module trieres_harris_filename
%include "cstring.i"
%cstring_bounded_output(char *output_img_str, 1024);
%cstring_bounded_output(char *output_points_str, 1024);
%{
 /* Put header files here or function declarations like below */
 extern void harris(char *s_servAddress, char *s_servPort, char *input_img_str, char *output_img_str, char *output_points_str);
%}
extern void harris(char *s_servAddress, char *s_servPort, char *input_img_str, char *output_img_str, char *output_points_str);
