/* trieres.i */
%module trieres
%include "cstring.i"
%{
#include "../../../cplusplus/include/config.h"
#define SWIG_FILE_WITH_INIT
 /* Put header files here or function declarations like below */
 extern void mceuropeanengine(int loop_nm, double *outarg, char *s_servAddress, char *s_servPort, 
              DtUsedInt seed,
              double underlying,
              double volatility,
              double dividendYield,
              double riskFreeRate,
              double timeLength,
              double strike,
              DtUsedInt optionType,
              double requiredTolerance,
              DtUsedInt requiredSamples,
              DtUsedInt timeSteps,
              DtUsedInt maxSamples);
%}
%include "numpy.i"

%init %{
     import_array();
%}
%apply (int DIM1, double* ARGOUT_ARRAY1) {(int loop_nm, double *outarg)};

typedef long unsigned int DtUsedInt;

extern void mceuropeanengine(int loop_nm, double *outarg, char *s_servAddress, char *s_servPort, 
              DtUsedInt seed,
              double underlying,
              double volatility,
              double dividendYield,
              double riskFreeRate,
              double timeLength,
              double strike,
              DtUsedInt optionType,
              double requiredTolerance,
              DtUsedInt requiredSamples,
              DtUsedInt timeSteps,
              DtUsedInt maxSamples);