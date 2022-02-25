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

/*****************************************************************************************************
Copyright (c) 2019, Xilinx, Inc.

All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************************************/

// top header file
#include <limits>
#include "../../common/include/common.hpp"
#include "../include/kernel_mceuropeanengine.hpp"

#include "xf_fintech/mc_engine.hpp"
#include "xf_fintech/rng.hpp"

extern "C" void kernel_mc(DtUsedInt loop_nm,
                          DtUsedInt seed,
                          DtUsed underlying,
                          DtUsed volatility,
                          DtUsed dividendYield,
                          DtUsed riskFreeRate, // model parameter
                          DtUsed timeLength,
                          DtUsed strike,
                          DtUsedInt optionType, // option parameter
                          DtUsed out[OUTDEP],
                          DtUsed requiredTolerance,
                          DtUsedInt requiredSamples,
                          DtUsedInt timeSteps,
                          DtUsedInt maxSamples,
			  bool *finished) {
/*
#pragma HLS INTERFACE m_axi port = out bundle = gmem latency = 125

#pragma HLS INTERFACE s_axilite port = loop_nm bundle = control
#pragma HLS INTERFACE s_axilite port = seed bundle = control
#pragma HLS INTERFACE s_axilite port = out bundle = control
#pragma HLS INTERFACE s_axilite port = strike bundle = control
#pragma HLS INTERFACE s_axilite port = underlying bundle = control
#pragma HLS INTERFACE s_axilite port = volatility bundle = control
#pragma HLS INTERFACE s_axilite port = dividendYield bundle = control
#pragma HLS INTERFACE s_axilite port = riskFreeRate bundle = control
#pragma HLS INTERFACE s_axilite port = timeLength bundle = control
#pragma HLS INTERFACE s_axilite port = requiredTolerance bundle = control
#pragma HLS INTERFACE s_axilite port = requiredSamples bundle = control
#pragma HLS INTERFACE s_axilite port = timeSteps bundle = control
#pragma HLS INTERFACE s_axilite port = maxSamples bundle = control
#pragma HLS INTERFACE s_axilite port = optionType bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control
*/
//#pragma HLS inline off
#pragma HLS data_pack variable = out
#ifndef __SYNTHESIS__
#ifdef XF_DEBUG
    std::cout << "loop_nm =" << loop_nm << std::endl;
    std::cout << "seed =" << seed << std::endl;
    std::cout << "underlying=" << underlying << std::endl;
    std::cout << "volatility=" << volatility << std::endl;
    std::cout << "dividendYield=" << dividendYield << std::endl;
    std::cout << "riskFreeRate=" << riskFreeRate << std::endl;
    std::cout << "timeLength=" << timeLength << std::endl;
    std::cout << "strike=" << strike << std::endl;
    std::cout << "optionType=" << optionType << std::endl;
    std::cout << "requiredTolerance=" << requiredTolerance << std::endl;
    std::cout << "requiredSamples=" << requiredSamples << std::endl;
    std::cout << "timeSteps=" << timeSteps << std::endl;
    std::cout << "maxSamples=" << maxSamples << std::endl;
#endif
#endif
	*finished = false;
	
	/* Since the kernel is returning an output array and not a stream, so that we can identify
	 * when this will be full, we assign a NaN value on the last element of the array. This
	 * will be our flag for the termination of the kernel, since this region is in dataflow.
	 * WARNING: If by chance, the kernel will return NaN, the external FSM will be blocked!
	 */
	out[loop_nm-1] = (DtUsed)std::numeric_limits<double>::quiet_NaN();

	#ifdef FAKE_MCEuropeanEngine
	DtUsed offset = (DtUsed)loop_nm + underlying + volatility + dividendYield + riskFreeRate +
			timeLength + strike + (DtUsed)optionType + (DtUsed)seed + 
			requiredTolerance + (DtUsed)requiredSamples + (DtUsed)timeSteps + 
			(DtUsed)maxSamples;
	for (unsigned int i = 0; i < OUTDEP; i++) {
	  out[i] = (DtUsed)i + offset;
	}
	#else // FAKE_MCEuropeanEngine
	ap_uint<32> seeds[MCM_NM];
	for (int i = 0; i < MCM_NM; ++i) {
	    seeds[i] = seed + i * 1000;
	}
	for (int i = 0; i < loop_nm; ++i) {
	    xf::fintech::MCEuropeanEngine<DtUsed, MCM_NM>(underlying, volatility, dividendYield, riskFreeRate, timeLength,
                                                      strike, optionType, seeds, &out[i], requiredTolerance,
                                                      requiredSamples, timeSteps, maxSamples);
	}
	#endif // FAKE_MCEuropeanEngine
	if (out[loop_nm-1] != (DtUsed)std::numeric_limits<double>::quiet_NaN()) {
	    *finished = true;
	}
}
