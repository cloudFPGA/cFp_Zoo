# *****************************************************************************
# *                            cloudFPGA
# *                Copyright 2016 -- 2022 IBM Corporation
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *     http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *----------------------------------------------------------------------------

import sys
import os
import numpy as np
trieres_lib=os.environ['cFpRootDir'] + "HOST/quantitative_finance/mceuropeanengine/languages/python/build"
sys.path.append(trieres_lib)

import _trieres


loop_nm = 1;
seed = 4332;
underlying = 202.56481854952224;
volatility = 0.005002151044533387;
dividendYield = 0.011160536907441138;
riskFreeRate = 0.008250879551073578;
timeLength = 1;
strike = 185.09682372241906;
optionType = 0;
requiredTolerance = 0.02;
requiredSamples = 100000;
timeSteps = 1;
maxSamples = 1;    
    
out = np.array([1.0,2.0]);
    
out = _trieres.mceuropeanengine(loop_nm, "localhost", "2718",
                            seed,
                            underlying,
                            volatility,
                            dividendYield,
                            riskFreeRate,
                            timeLength,
                            strike,
                            optionType,
                            requiredTolerance,
                            requiredSamples,
                            timeSteps,
                            maxSamples);

print(out)
