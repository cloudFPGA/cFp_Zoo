import sys
import os
import numpy as np

trieres_lib=os.environ['cFpRootDir'] + "/HOST/languages/python/build"
sys.path.append(trieres_lib)

import _trieres

loop_nm = 2;    
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
    
_trieres.mceuropeanengine("localhost", "1234", loop_nm,
                            seed,
                            underlying,
                            volatility,
                            dividendYield,
                            riskFreeRate,
                            timeLength,
                            strike,
                            optionType,
                            out,
                            requiredTolerance,
                            requiredSamples,
                            timeSteps,
                            maxSamples);

print(out)
