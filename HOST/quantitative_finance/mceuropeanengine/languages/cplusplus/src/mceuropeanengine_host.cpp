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

/*****************************************************************************
 * @file       mceuropeanengine_host.cpp
 * @brief      mceuropeanengine userspace application for cF (x86, ppc64).
 *
 * @date       May 2020
 * @author     DID
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup MCEuropeanEngine
 * @addtogroup MCEuropeanEngine
 * \{
 *****************************************************************************/


#include <stdio.h>
#include <iostream>                     // For cout and cerr
#include <cstdlib>                      // For atoi()
#include <assert.h>                     // For assert()
#include <string>                       // For to_string
#include <string.h>
#include <arpa/inet.h>

#include "../../../../../PracticalSockets/src/PracticalSockets.h" // For UDPSocket and SocketException
#include "../include/config.h"

using namespace std;


void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

                                                                                                    
void print_cFpZoo(void)
{
        cout <<  "                                                          " << endl;
	cout <<  "...build with:                                            " << endl;
	cout <<  " ██████╗███████╗██████╗    ███████╗ ██████╗  ██████╗      " << endl;
	cout <<  "██╔════╝██╔════╝██╔══██╗   ╚══███╔╝██╔═══██╗██╔═══██╗     " << endl;
	cout <<  "██║     █████╗  ██████╔╝     ███╔╝ ██║   ██║██║   ██║     " << endl;
	cout <<  "██║     ██╔══╝  ██╔═══╝     ███╔╝  ██║   ██║██║   ██║     " << endl;
	cout <<  "╚██████╗██║     ██║███████╗███████╗╚██████╔╝╚██████╔╝     " << endl;
	cout <<  " ╚═════╝╚═╝     ╚═╝╚══════╝╚══════╝ ╚═════╝  ╚═════╝      " << endl;
	cout <<  "A cloudFPGA project from IBM ZRL                    v1.0  " << endl;
	cout <<  "Quantitative Finance Monte-Carlo European Pricing Engine  " << endl;
}

#ifdef PY_WRAP
void mceuropeanengine(DtUsedInt loop_nm, DtUsed *outarg, char *s_servAddress, char *s_servPort, 
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
              DtUsedInt maxSamples)
{
#else  
  /**
   *   Main testbench and user-application for mceuropeanengine on host. Client
   *   @return O on success, 1 on fail 
   */
int main(int argc, char *argv[])
{
    if ((argc < 3) || (argc > 4)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port> \n";
        exit(1);
    }
#endif

    
    //------------------------------------------------------
    //-- STEP-1 : Socket and variables definition
    //------------------------------------------------------
    
    #ifndef PY_WRAP
    assert (argc == 3);
    string s_servAddress = argv[1]; // First arg: server address
    char *s_servPort = argv[2];
    #endif

    string servAddress = s_servAddress;
    unsigned short servPort;
    bool net_type = NET_TYPE;
    if (net_type == udp) {
	    servPort = Socket::resolveService(s_servPort, "udp");
    }
    else if (net_type == tcp) {
	    servPort = atoi(s_servPort);
    }
    else {
	    cout << "ERROR: Invalid type of socket type provided: " << net_type  << " Choosed one of (tcp=0 or udp=1)" << endl;
    }

    if ((servPort <= 0) || (servPort <= 1024) || (servPort >= 65536)) {
        cerr << "ERROR: Invalid port number " << servPort <<
        ". Please select a value at range [1025-65535]. Aborting..." << endl;
        exit(1);
    }
    
    char buffer[BUF_LEN]; // Buffer for echo string
    unsigned int recvMsgSize; // Size of received message
    unsigned int num_frame = 0;
    std::string input_string;
    //UDPSocket *udpsock_p;
    //TCPSocket *tcpsock_p;
  
    print_cFpZoo();
    
    try {
          
        //------------------------------------------------------
        //-- STEP-2 : Initialize socket connection
        //------------------------------------------------------      
      #if (NET_TYPE == udp)
	    #ifndef TB_SIM_CFP_VITIS
	    UDPSocket udpsock(servPort); // NOTE: It is very important to set port here in order to call 
	                                 // bind() in the UDPSocket constructor
	    #else // TB_SIM_CFP_VITIS
	    UDPSocket udpsock; // NOTE: In HOST TB the port is already binded by mceuropeanengine_host_fwd_tb.cpp
	    #endif // TB_SIM_CFP_VITIS
	    //udpsock_p = &udpsock;
      #else // tcp
	    TCPSocket tcpsock(servAddress, servPort);
	    //tcpsock_p = &tcpsock;
      #endif // udp/tcp
        
        //------------------------------------------------------------------------------------
        //-- STEP-3 : Create an input struct
        //------------------------------------------------------------------------------------
        varin instruct;
	#ifndef PY_WRAP
	instruct.loop_nm = OUTDEP;    
	instruct.timeSteps = 1;
	instruct.requiredTolerance = 0.02f;
	instruct.underlying = 36;
	instruct.riskFreeRate = 0.06;
	instruct.volatility = 0.20;
	instruct.dividendYield = 0.0;
	instruct.strike = 40;
	instruct.optionType = 1;
	instruct.timeLength = 1;
	instruct.seed = 4332 ; // 441242, 42, 13342;
	instruct.requiredSamples = 1; // 262144; // 48128;//0;//1024;//0;
	instruct.maxSamples = 1;
	#else
	instruct.loop_nm = loop_nm;    
	instruct.timeSteps = timeSteps;
	instruct.requiredTolerance = requiredTolerance;
	instruct.underlying = underlying;
	instruct.riskFreeRate = riskFreeRate;
	instruct.volatility = volatility;
	instruct.dividendYield = dividendYield;
	instruct.strike = strike;
	instruct.optionType = optionType;
	instruct.timeLength = timeLength;
	instruct.seed = seed;
	instruct.requiredSamples = requiredSamples;
	instruct.maxSamples = maxSamples;
	#endif
        
	assert(instruct.loop_nm > 0);
	assert(instruct.loop_nm <= OUTDEP);
    
    clock_t start_cycle_main = clock();
    cout << " ___________________________________________________________________ " << endl;
    cout << "/                                                                   \\" << endl;
	cout << "INFO: Batch # " << ++num_frame << endl;
	    
	// Ensure that the selection of MTU is a multiple of 8 (Bytes per transaction)
	assert(PACK_SIZE % 8 == 0);
   
    unsigned int total_pack_tx  = 1 + (sizeof(instruct) - 1) / PACK_SIZE;
	unsigned int total_pack_rx  = 1 + (instruct.loop_nm*sizeof(DtUsed) - 1) / PACK_SIZE;
    unsigned int total_bytes = total_pack_tx * PACK_SIZE;
    unsigned int bytes_in_last_pack_tx = sizeof(instruct) - (total_pack_tx - 1) * PACK_SIZE;
    unsigned int bytes_in_last_pack_rx = instruct.loop_nm*sizeof(DtUsed) - (total_pack_rx - 1) * PACK_SIZE;

	cout << "INFO: Network socket           : " << ((net_type == tcp) ? "TCP" : "UDP") << endl;
	cout << "INFO: Total packets to send    : " << total_pack_tx << endl;
	cout << "INFO: Total packets to receive : " << total_pack_rx << endl;
        cout << "INFO: Total bytes to send      : " << sizeof(instruct) << endl;
        cout << "INFO: Total bytes to receive   : " << instruct.loop_nm*sizeof(DtUsed) << endl;	
	cout << "INFO: Total bytes in " << total_pack_tx << " packets : "  << total_bytes << endl;
	cout << "INFO: Bytes in last packet send: " << bytes_in_last_pack_tx << endl;
	cout << "INFO: Bytes in last packet recv: " << bytes_in_last_pack_rx << endl;
	cout << "INFO: Packet size (custom MTU) : " << PACK_SIZE << endl;
	    
	//------------------------------------------------------
        //-- STEP-4 : RUN mceuropeanengine FROM cF (HW)
        //------------------------------------------------------
        clock_t start_cycle_mceuropeanengine_hw = clock();
	    
	//------------------------------------------------------
        //-- STEP-5.1 : TX Loop
        //------------------------------------------------------
        clock_t last_cycle_tx = clock();
	unsigned int sending_now = PACK_SIZE;
        for (unsigned int i = 0; i < total_pack_tx; i++) {
	    if ( i == total_pack_tx - 1 ) {
		sending_now = bytes_in_last_pack_tx;
	    }
	    #if (NET_TYPE == udp)
		udpsock.sendTo( & instruct, sending_now, servAddress, servPort);
	    #else
		tcpsock.send( & instruct, sending_now);
	    #endif
	    delay(1000);  
	}
           
        clock_t next_cycle_tx = clock();
        double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective SPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
             total_pack_tx / duration_tx / 1024 * 8) << endl;
        last_cycle_tx = next_cycle_tx;
	   
	    
	//------------------------------------------------------
        //-- STEP-5.2 : RX Loop
        //------------------------------------------------------    
	clock_t last_cycle_rx = clock();
	unsigned int receiving_now = PACK_SIZE;
        cout << "INFO: Expecting length of packs:" << total_pack_rx << endl;
        char * longbuf = new char[PACK_SIZE * total_pack_rx];
	memset(longbuf, 0, PACK_SIZE * total_pack_rx);
        for (unsigned int i = 0; i < instruct.loop_nm*sizeof(DtUsed); ) {
	    //cout << "DEBUG: " << i << endl;
            if ( i == total_pack_rx - 1 ) {
                receiving_now = bytes_in_last_pack_rx;
            }
	    #if (NET_TYPE == udp)                
		recvMsgSize = udpsock.recvFrom(buffer, BUF_LEN, servAddress, servPort);
	    #else
		recvMsgSize = tcpsock.recv(buffer, BUF_LEN);
	    #endif
            memcpy( & longbuf[i], buffer, recvMsgSize);
	    i += recvMsgSize;
	    if (recvMsgSize != receiving_now) {
		cerr << "WARNING: Received unexpected size pack:" << recvMsgSize << ". Expected: " << 
			receiving_now + " . Breaking Rx..."<< endl;
			break;
            }
	    //cout << "DEBUG: recvMsgSize=" << recvMsgSize << endl;
        }

        cout << "INFO: Received packet from " << servAddress << ":" << servPort << endl;
 	#ifndef PY_WRAP
	DtUsed *out = (DtUsed*)malloc(instruct.loop_nm*sizeof(DtUsed));
	#else
	DtUsed *out = outarg;
	#endif

	for (unsigned int i = 0, j = 0; i < instruct.loop_nm; i++, j+=sizeof(DtUsed)) {
	    memcpy(&out[i], &longbuf[j], sizeof(DtUsed)); 
	}

	cout << "INFO: Received option price vector: " << endl;
	for (unsigned int i = 0; i < instruct.loop_nm; i++) {
	  cout << i<< " : " << out[i] << endl;
	}
	
        clock_t next_cycle_rx = clock();
        double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective SPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * 
                    total_pack_rx / duration_rx / 1024 * 8) << endl;
        last_cycle_rx = next_cycle_rx;
	   
	clock_t end_cycle_mceuropeanengine_hw = next_cycle_rx;
	    
	double duration_mceuropeanengine_hw = (end_cycle_mceuropeanengine_hw - start_cycle_mceuropeanengine_hw) / 
	                                (double) CLOCKS_PER_SEC;
	cout << "INFO: HW exec. time:" << duration_mceuropeanengine_hw << " seconds" << endl;
        cout << "INFO: Effective SPS HW:" << (1 / duration_mceuropeanengine_hw) << " \tkbps:" << 
                (PACK_SIZE * (total_pack_rx + total_pack_rx) / duration_mceuropeanengine_hw / 1024 * 8) << endl;
	    
        double duration_main = (clock() - start_cycle_main) / (double) CLOCKS_PER_SEC;
	#ifndef PY_WRAP
	free(out);
	#endif
	cout << "INFO: Effective SPS E2E:" << (1 / duration_main) << endl;
        cout << "\\___________________________________________________________________/" << endl
	     << endl;
  
    // Destructor closes the socket
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }
    #ifndef PY_WRAP
    return 0;
    #endif
}




/*! \} */
