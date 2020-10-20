/* trieres.i */
%module trieres
%{
#include "../../../cplusplus/include/config.h"
#define SWIG_FILE_WITH_INIT
 /* Put header files here or function declarations like below */
 extern void mceuropeanengine(char *s_servAddress, char *s_servPort, 
	      DtUsedInt loop_nm,
              DtUsedInt seed,
              DtUsed underlying,
              DtUsed volatility,
              DtUsed dividendYield,
              DtUsed riskFreeRate,
              DtUsed timeLength,
              DtUsed strike,
              DtUsedInt optionType,
              DtUsed *outarg,
              DtUsed requiredTolerance,
              DtUsedInt requiredSamples,
              DtUsedInt timeSteps,
              DtUsedInt maxSamples);
%}
%include "numpy.i"

%init %{
     import_array();
%}
 
%apply (char *s_servAddress, char *s_servPort, 
	      DtUsedInt loop_nm,
              DtUsedInt seed,
              DtUsed underlying,
              DtUsed volatility,
              DtUsed dividendYield,
              DtUsed riskFreeRate,
              DtUsed timeLength,
              DtUsed strike,
              DtUsedInt optionType,
              DtUsed* ARGOUT_ARRAY1, 
              DtUsed requiredTolerance,
              DtUsedInt requiredSamples,
              DtUsedInt timeSteps,
              DtUsedInt maxSamples) {(char *s_servAddress, char *s_servPort, 
	      DtUsedInt loop_nm,
              DtUsedInt seed,
              DtUsed underlying,
              DtUsed volatility,
              DtUsed dividendYield,
              DtUsed riskFreeRate,
              DtUsed timeLength,
              DtUsed strike,
              DtUsedInt optionType,
              DtUsed *outarg, DtUsed requiredTolerance,
              DtUsedInt requiredSamples,
              DtUsedInt timeSteps,
              DtUsedInt maxSamples)}

extern void mceuropeanengine(char *s_servAddress, char *s_servPort, 
	      DtUsedInt loop_nm,
              DtUsedInt seed,
              DtUsed underlying,
              DtUsed volatility,
              DtUsed dividendYield,
              DtUsed riskFreeRate,
              DtUsed timeLength,
              DtUsed strike,
              DtUsedInt optionType,
              DtUsed *outarg,
              DtUsed requiredTolerance,
              DtUsedInt requiredSamples,
              DtUsedInt timeSteps,
              DtUsedInt maxSamples);
              
