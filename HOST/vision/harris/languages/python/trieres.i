/* trieres.i */
%module trieres
%include "cstring.i"
%cstring_bounded_output(char *output_img_str, 1024);
%cstring_bounded_output(char *output_points_str, 1024);
%{
 /* Put header files here or function declarations like below */
 extern int harris(char *s_servAddress, char *s_servPort, char *input_img_str, char *output_img_str, char *output_points_str);
%}
extern int harris(char *s_servAddress, char *s_servPort, char *input_img_str, char *output_img_str, char *output_points_str);