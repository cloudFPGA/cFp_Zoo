/* trieres.i */
%module trieres
%include "cstring.i"
%cstring_bounded_output(char *output_str, 1024);
%{
 /* Put header files here or function declarations like below */
 extern int uppercase(char *s_servAddress, char *s_servPort, char *input_str, char *output_str, bool net_type);
%}
extern int uppercase(char *s_servAddress, char *s_servPort, char *input_str, char *output_str, bool net_type);