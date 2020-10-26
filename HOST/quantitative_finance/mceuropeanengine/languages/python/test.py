import sys
import os
import numpy as np
import QuantLib as ql
trieres_lib=os.environ['cFpRootDir'] + "HOST/quantitative_finance/mceuropeanengine/languages/python/build"
sys.path.append(trieres_lib)

import _trieres

loop_nm = 1024;    
seed = 4332;
underlying = 36;
volatility = 0.20;
dividendYield = 0.0;
riskFreeRate = 0.06;
timeLength = 1;
strike = 40;
optionType = 1;
requiredTolerance = 0.02;
requiredSamples = 1;
timeSteps = 1;
maxSamples = 1;    
    
out = np.array([1.0,2.0]);
    
out = _trieres.mceuropeanengine(loop_nm, "localhost", "1234",
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
