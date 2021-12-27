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
 * @file       uppercase_host.cpp
 * @brief      Uppercase userspace application for cF (x86, ppc64).
 *
 * @date       May 2020
 * @author     DID
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup Uppercase
 * @addtogroup Uppercase
 * \{
 *****************************************************************************/

 
#include <stdio.h>
#include <iostream>                     // For cout and cerr
#include <cstdlib>                      // For atoi()
#include <assert.h>                     // For assert()
#include <string>                       // For to_string
#include <string.h>
#include "../../../../../PracticalSockets/src/PracticalSockets.h" // For UDPSocket and SocketException
#include "../include/config.h"

using namespace std;


void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

                                                                                                    
void print_cFpZoo(void)
        // http://patorjk.com/software/taag/#p=display&f=ANSI%20Shadow&t=cFp_Zoo
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
int uppercase(char *s_servAddress, char *s_servPort, char *input_str, char *output_str, bool net_type)
{
#else  
  /**
   *   Main testbench and user-application for Uppercase on host. Client
   *   @return O on success, 1 on fail 
   */
int main(int argc, char *argv[])
{
    if ((argc < 3) || (argc > 4)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port> <input string> <optional output string>\n";
        exit(1);
    }
#endif

    
    //------------------------------------------------------
    //-- STEP-1 : Socket and variables definition
    //------------------------------------------------------
    
    #ifndef PY_WRAP
    assert (argc == 4);
    string s_servAddress = argv[1]; // First arg: server address
    char *s_servPort = argv[2];
    bool net_type = NET_TYPE;
    #endif

    string servAddress = s_servAddress;
    unsigned short servPort;
    if (net_type == udp) {
	servPort = Socket::resolveService(s_servPort, "udp");
    }
    else if (net_type == tcp) {
	servPort = atoi(s_servPort);
    }
    else {
	cout << "ERROR: Invalid type of socket type provided: " << net_type  << " Choosed one of (tcp=0 or udp=1)" << endl;
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
	#if NET_TYPE == udp
	    #ifndef TB_SIM_CFP_VITIS
	    UDPSocket udpsock(servPort); // NOTE: It is very important to set port here in order to call 
	                                 // bind() in the UDPSocket constructor
	    #else // TB_SIM_CFP_VITIS
	    UDPSocket udpsock; // NOTE: In HOST TB the port is already binded by uppercase_host_fwd_tb.cpp
	    #endif // TB_SIM_CFP_VITIS
	//    udpsock_p = &udpsock;
	#else  // tcp
	    TCPSocket tcpsock(servAddress, servPort);
	//    tcpsock_p = &tcpsock;
	#endif // udp/tcp
        
        //------------------------------------------------------------------------------------
        //-- STEP-3 : Create a string from input argument
        //------------------------------------------------------------------------------------
	#ifdef PY_WRAP
	input_string.assign(input_str);
	#else
        input_string.assign(argv[3]);
	#endif
	if (input_string.length() == 0) {
            cerr << "Empty string provided. Aborting...\n\n" << endl;
            exit(1);
        }
    
        clock_t start_cycle_main = clock();
        cout << " ___________________________________________________________________ " << endl;
        cout << "/                                                                   \\" << endl;
	cout << "INFO: Batch # " << ++num_frame << endl;
	    
	// Ensure that the selection of MTU is a multiple of 8 (Bytes per transaction)
	assert(PACK_SIZE % 8 == 0);
   
        unsigned int total_pack  = 1 + (input_string.length() - 1) / PACK_SIZE;
        unsigned int total_bytes = total_pack * PACK_SIZE;
        unsigned int bytes_in_last_pack = input_string.length() - (total_pack - 1) * PACK_SIZE;

	cout << "INFO: Network socket : " << ((net_type == tcp) ? "TCP" : "UDP") << endl;
	cout << "INFO: Total packets to send/receive = " << total_pack << endl;
        cout << "INFO: Total bytes to send/receive   = " << input_string.length() << endl;
	cout << "INFO: Total bytes in " << total_pack << " packets = "  << total_bytes << endl;
	cout << "INFO: Bytes in last packet          = " << bytes_in_last_pack << endl;
	cout << "INFO: Packet size (custom MTU)      = " << PACK_SIZE << endl;
	    
	//------------------------------------------------------
        //-- STEP-4 : RUN UPPERCASE FROM cF (HW)
        //------------------------------------------------------
        clock_t start_cycle_uppercase_hw = clock();
	    
	//------------------------------------------------------
        //-- STEP-5.1 : TX Loop
        //------------------------------------------------------
        clock_t last_cycle_tx = clock();
	unsigned int sending_now = PACK_SIZE;
        for (unsigned int i = 0; i < total_pack; i++) {
	    if ( i == total_pack - 1 ) {
		sending_now = bytes_in_last_pack;
	    }
	    #if NET_TYPE == udp
	    udpsock.sendTo( & input_string[i * PACK_SIZE], sending_now, servAddress, servPort);
	    #else
            tcpsock.send( & input_string[i * PACK_SIZE], sending_now);
	    #endif
	    delay(1000);  
	}
        clock_t next_cycle_tx = clock();
        double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective SPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
             total_pack / duration_tx / 1024 * 8) << endl;
        last_cycle_tx = next_cycle_tx;
	   
	    
	//------------------------------------------------------
        //-- STEP-5.2 : RX Loop
        //------------------------------------------------------    
	clock_t last_cycle_rx = clock();
	unsigned int receiving_now = PACK_SIZE;
        cout << "INFO: Expecting length of packs:" << total_pack << endl;
        char * longbuf = new char[PACK_SIZE * total_pack];
        for (unsigned int i = 0; i < input_string.length(); ) {
	    //cout << "DEBUG: " << i << endl;
            if ( i == total_pack - 1 ) {
                receiving_now = bytes_in_last_pack;
            }
	    #if NET_TYPE == udp               
	    recvMsgSize = udpsock.recvFrom(buffer, BUF_LEN, servAddress, servPort);
	    #else
	    recvMsgSize = tcpsock.recv(buffer, BUF_LEN);
	    #endif
	    if (recvMsgSize != receiving_now) {
		cerr << "WARNING: Received unexpected size pack:" << recvMsgSize << ". Expected: " << 
			receiving_now << endl;
            }
            memcpy( & longbuf[i], buffer, recvMsgSize);
	    //cout << "DEBUG: recvMsgSize=" << recvMsgSize << endl;
	    i += recvMsgSize;
        }

        cout << "INFO: Received packet from " << servAddress << ":" << servPort << endl;
 
	#ifdef PY_WRAP
	char *output_string = output_str;
	#else
        char *output_string = (char*)malloc((input_string.length()+1)*sizeof(char));
	#endif
	output_string = strncpy(output_string, longbuf, input_string.length());
	output_string[input_string.length()]='\0';
	cout << "INFO: Received string : " << output_string << endl;
        clock_t next_cycle_rx = clock();
        double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective SPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * 
                    total_pack / duration_rx / 1024 * 8) << endl;
        last_cycle_rx = next_cycle_rx;
	   
	clock_t end_cycle_uppercase_hw = next_cycle_rx;
	    
	double duration_uppercase_hw = (end_cycle_uppercase_hw - start_cycle_uppercase_hw) / 
	                                (double) CLOCKS_PER_SEC;
	cout << "INFO: HW exec. time:" << duration_uppercase_hw << " seconds" << endl;
        cout << "INFO: Effective SPS HW:" << (1 / duration_uppercase_hw) << " \tkbps:" << 
                (PACK_SIZE * total_pack / duration_uppercase_hw / 1024 * 8) << endl;	    
	    
        double duration_main = (clock() - start_cycle_main) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective SPS E2E:" << (1 / duration_main) << endl;
        cout << "\\___________________________________________________________________/" << endl
	     << endl;
  
    // Destructor closes the socket
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }

    return 0;
}




/*! \} */
