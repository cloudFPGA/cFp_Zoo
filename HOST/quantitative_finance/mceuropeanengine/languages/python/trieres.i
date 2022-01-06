/* trieres.i */

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
