/* trieres.i */

%module trieres
%include "cstring.i"

%{

#include "../../../cplusplus/include/config.h"
#define SWIG_FILE_WITH_INIT

extern void mceuropeanengine(DtUsedInt loop_nm, DtUsed *outarg, 
              char *s_servAddress, char *s_servPort, 
              DtUsedInt seed,
              DtUsed underlying,
              DtUsed volatility,
              DtUsed dividendYield,
              DtUsed riskFreeRate,
              DtUsed timeLength,
              DtUsed strike,
              DtUsedInt optionType,
              DtUsed requiredTolerance,
              DtUsedInt requiredSamples,
              DtUsedInt timeSteps,
              DtUsedInt maxSamples);
%}

%include "numpy.i"

%init %{
     import_array();
%}

typedef long unsigned int DtUsedInt;
typedef double DtUsed;

%apply (int DIM1, DtUsed* ARGOUT_ARRAY1) {(DtUsedInt loop_nm, DtUsed *outarg)};

extern void mceuropeanengine(DtUsedInt loop_nm, DtUsed *outarg, 
              char *s_servAddress, char *s_servPort, 
              DtUsedInt seed,
              DtUsed underlying,
              DtUsed volatility,
              DtUsed dividendYield,
              DtUsed riskFreeRate,
              DtUsed timeLength,
              DtUsed strike,
              DtUsedInt optionType,
              DtUsed requiredTolerance,
              DtUsedInt requiredSamples,
              DtUsedInt timeSteps,
              DtUsedInt maxSamples);