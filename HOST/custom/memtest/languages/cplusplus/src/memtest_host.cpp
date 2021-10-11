
/*****************************************************************************
 * @file       memtest_host.cpp
 * @brief      Memtest userspace application for cF (x86, ppc64).
 *
 * @date       Sept 2021
 * @author     DCO
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup Memtest
 * @addtogroup Memtest
 * \{
 *****************************************************************************/


#include <stdio.h>
#include <iostream>                     // For cout and cerr
#include <cstdlib>                      // For atoi()
#include <assert.h>                     // For assert()
#include <string>                       // For to_string
#include <string.h>
#include <sstream>
#include "../../../../../PracticalSockets/src/PracticalSockets.h" // For UDPSocket and SocketException
#include "../include/config.h"
#include "../include/common.hpp"


#ifdef PY_WRAP
int memtest(char *s_servAddress, char *s_servPort, char *input_str, char *output_str, bool net_type)
{
#else  
  /**
   *   Main testbench and user-application for Memtest on host. Client
   *   @return O on success, 1 on fail 
   */
int main(int argc, char *argv[])
{
    if ((argc < 3) || (argc > 5)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port> <number of address to test> <testing times>\n";
        exit(1);
    }
#endif

    
    //------------------------------------------------------
    //-- STEP-1 : Socket and variables definition
    //------------------------------------------------------
    
    #ifndef PY_WRAP
    assert (argc == 5);
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
	std::string strInput_memaddrUT;
    std::string strInput_nmbrTest;
	unsigned int memory_addr_under_test=0;
    unsigned int testingNumber = 1;
    //UDPSocket *udpsock_p;
    //TCPSocket *tcpsock_p;
  
    print_cFpMemtest();
    
    try {
          
        //------------------------------------------------------
        //-- STEP-2 : Initialize socket connection
        //------------------------------------------------------      
	#if NET_TYPE == udp
	    #ifndef TB_SIM_CFP_VITIS
	    UDPSocket udpsock(servPort); // NOTE: It is very important to set port here in order to call 
	                                 // bind() in the UDPSocket constructor
	    #else // TB_SIM_CFP_VITIS
	    UDPSocket udpsock; // NOTE: In HOST TB the port is already binded by memtest_host_fwd_tb.cpp
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
	input_string.assign(input_str); //TBC
	#else
    strInput_memaddrUT.assign(argv[3]);
    strInput_nmbrTest.assign(argv[4]);
	#endif
	memory_addr_under_test = stoul(strInput_memaddrUT);
	testingNumber = stoul(strInput_nmbrTest);

	//
	string user_choice = "r";
	//Iterating and interactive loop
	while (user_choice.compare("q") != 0 ) //quit and run && quit
	{
	
		unsigned int test_pack = 1;
		size_t charOutputSizeRoughBytes=8*1+((8 * (2 + 1)) * testingNumber);
		if( charOutputSizeRoughBytes / PACK_SIZE > 1){
			test_pack = (unsigned int)(8 * (2 + 3 * testingNumber) / PACK_SIZE);
		}
		size_t charOutputSize = PACK_SIZE*(test_pack); 
		//size_t charOutputSizeRoughBytes= = (8 * (2 + 3 * testingNumber) ); 
		//8 * (2 + 1) * testingNumber; this should be the real number but seems to send everything

		string initial_input_string(input_string);

		input_string=createMemTestCommands(memory_addr_under_test, testingNumber);
		size_t charInputSize = 8; //a single tdata
		//printStringHex(input_string,charInputSize*sizeof(char));

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
	
		//packt host2FPGA
		unsigned int total_out_pack  =  1 + (input_string.length() - 1) / PACK_SIZE;// only a single tx
		unsigned int total_out_bytes = charInputSize;
		unsigned int bytes_in_last_pack_out = input_string.length() - (total_out_pack-1) * PACK_SIZE;

		//packt FPGA2host
		unsigned int total_in_pack  = test_pack;
		unsigned int total_in_bytes = total_in_pack * PACK_SIZE;
		unsigned int bytes_in_last_pack_in = charOutputSizeRoughBytes - (total_in_pack - 1) * PACK_SIZE;

		cout << "INFO: Network socket : " << ((net_type == tcp) ? "TCP" : "UDP") << endl;
		cout << "INFO: Total packets to send = " << total_out_pack << endl;
		cout << "INFO: Total packets to receive = " << total_in_pack << endl;
		cout << "INFO: Total bytes to send   = " << total_out_bytes << endl;
		cout << "INFO: Total bytes to receive   = " << total_in_bytes << endl;
		cout << "INFO: Total bytes rx " << total_in_bytes << " packets = "  << total_in_pack << endl;
		cout << "INFO: Bytes in last packet tx          = " << bytes_in_last_pack_out << endl;
		cout << "INFO: Bytes in last packet  rx         = " << bytes_in_last_pack_in << endl;
		cout << "INFO: Packet size (custom MTU)      = " << PACK_SIZE << endl;
			
		//------------------------------------------------------
			//-- STEP-4 : RUN MEMTEST FROM cF (HW)
			//------------------------------------------------------
		clock_t start_cycle_memtest_hw = clock();
			
		//------------------------------------------------------
			//-- STEP-5.1 : TX Loop
			//------------------------------------------------------
		clock_t last_cycle_tx = clock();
		unsigned int sending_now = PACK_SIZE;
		for (unsigned int i = 0; i < total_out_pack; i++) {
			if ( i == total_out_pack - 1 ) {
			sending_now = bytes_in_last_pack_out;
			}
			#if NET_TYPE == udp
			//printStringHex(input_string,charInputSize*sizeof(char));
			udpsock.sendTo( & input_string[i * PACK_SIZE], sending_now, servAddress, servPort);
			#else
				tcpsock.send( & input_string[i * PACK_SIZE], sending_now);
			#endif
			delay(1000);  
		}
		clock_t next_cycle_tx = clock();
		double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
		cout << "INFO: Effective SPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
			total_out_pack / duration_tx / 1024 * 8) << endl;
		last_cycle_tx = next_cycle_tx;
	
			
		//------------------------------------------------------
			//-- STEP-5.2 : RX Loop
			//------------------------------------------------------    
		clock_t last_cycle_rx = clock();
		unsigned int bytes_received = 0;
		unsigned int receiving_now = PACK_SIZE;
		cout << "INFO: Expecting length of packs:" << total_in_pack << endl;
		char * longbuf = new char[PACK_SIZE * total_in_pack+1];
		for (unsigned int i = 0; i < total_in_pack; ) {
			////////////////////////
			//TODO: check the receiving loop
			////////////////////////

			//cout << "DEBUG: " << i << endl;
				if ( i == total_in_pack - 1 ) {
					receiving_now = bytes_in_last_pack_in;
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
				memcpy( longbuf+(i*bytes_received), buffer, recvMsgSize);
			cout << "DEBUG: recvMsgSize=" << recvMsgSize << endl;
			//i += recvMsgSize;
			bytes_received  += recvMsgSize;
			i ++;
		}

		cout << "INFO: Received packet from " << servAddress << ":" << servPort << endl;
	
		#ifdef PY_WRAP
		char *output_string = output_str;
		#else
		//char *output_string = (char*)malloc(PACK_SIZE * total_in_pack*sizeof(char));
		char *output_string = new char [(PACK_SIZE * total_in_pack*sizeof(char))];
		#endif
		memcpy( output_string, longbuf, bytes_received*sizeof(char));
		//output_string = strncpy(output_string, longbuf, PACK_SIZE * total_in_pack*sizeof(char));
		output_string[PACK_SIZE * total_in_pack]='\0';
		cout << "INFO: Received string : " << output_string << endl;
		//for(int i=0; i<total_in_pack; i++){
			//printCharBuffHex(output_string+(i*PACK_SIZE),PACK_SIZE);
		printCharBuffHex(output_string,bytes_received);
		//}

		clock_t next_cycle_rx = clock();
		double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
		cout << "INFO: Effective SPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * 
					total_in_pack / duration_rx / 1024 * 8) << endl;
		//TODO: Check this number :D
		last_cycle_rx = next_cycle_rx;
		
		clock_t end_cycle_memtest_hw = next_cycle_rx;
			
		double duration_memtest_hw = (end_cycle_memtest_hw - start_cycle_memtest_hw) / 
										(double) CLOCKS_PER_SEC;
		cout << "INFO: HW exec. time:" << duration_memtest_hw << " seconds" << endl;
		cout << "INFO: Effective SPS HW:" << (1 / duration_memtest_hw) << " \tkbps:" << 
					(PACK_SIZE * total_in_pack / duration_memtest_hw / 1024 * 8) << endl;	
					//TODO: CHECK THIS OUT    
			
		double duration_main = (clock() - start_cycle_main) / (double) CLOCKS_PER_SEC;
		cout << "INFO: Effective SPS E2E:" << (1 / duration_main) << endl;
		cout << "\\___________________________________________________________________/" << endl
		<< endl;

		cout << " ___________________________________________________________________ " << endl;
		cout << "/                                                                   \\" << endl;
		cout <<"INFO: Entering in the output parsing section  "<< endl;
		std::vector<MemoryTestResult> testResults_vector;
		string output_to_parse_string;
		output_to_parse_string.append(output_string,charOutputSizeRoughBytes);
		int rawdatalines = charOutputSizeRoughBytes / 8;;
		testResults_vector=parseMemoryTestOutput(output_to_parse_string,charOutputSizeRoughBytes,rawdatalines);
		cout << "\\___________________________________________________________________/" << endl
		<< endl;
		//otuput showing
		cout << " ___________________________________________________________________ " << endl;
		cout << "/                                                                   \\" << endl;
		cout << "INFO: Test Results appearing" << endl;
		cout << " The Memory test run for  " << testResults_vector.size() << " iterations " << endl;
		for(auto it = std::begin(testResults_vector); it != std::end(testResults_vector); ++it) {
			cout << " Test number " << it - testResults_vector.begin() << " stress " << 	it->target_address << " addresses "  << endl;
			cout << " it presented " << it->fault_cntr << " faults " << endl;
			cout << " and the first faulty address (if any) was " << it->first_fault_address << endl;
			cout << endl << endl;
		}
		////////////////
		///// TODO:
		// from here interaction cycle
		////////////////
		//clear dynamic memory
		delete [] longbuf;
		delete [] output_string;
		testResults_vector.clear();
		output_to_parse_string.clear();
		input_string.clear();
		initial_input_string.clear();
		strInput_memaddrUT.clear();
    	strInput_nmbrTest.clear();
		//Need to close the application and send to the accelerator a stop for signaling a termination
		if (user_choice.compare("rq")==0)
		{
			cout << "INFO: going to close the application and send a stop command as the user requested." << endl;
			user_choice.clear();
			string cmd_string = createMemTestStopCommand();
			char stop_char_buff [8];
			memcpy(stop_char_buff, cmd_string.data(), 8);
			sending_now = 8;
			#if NET_TYPE == udp
			udpsock.sendTo( stop_char_buff, sending_now, servAddress, servPort);
			#else
				tcpsock.send(stop_char_buff, sending_now);
			#endif
			delay(1000); 
			#if NET_TYPE == udp               
			recvMsgSize = udpsock.recvFrom(buffer, BUF_LEN, servAddress, servPort);
			#else
			recvMsgSize = tcpsock.recv(buffer, BUF_LEN);
			#endif
			delay(1000); 
			char output_stop [8];
			memcpy(output_stop,buffer,8);
			if(strcmp(output_stop,stop_char_buff)==0){
				cout << "INFO: Accelerator answered ok" << endl;
			}else{
				cout << "INFO: Bad answer received, take care "<< endl;
			}
			cout << "INFO: IBM ZRL Memtest is closing." << endl<< "Goodbye :D" << endl;
			break;
		}
		
		user_choice.clear();
		string confirmation="";

		cout << "That is all from this application." << endl;
		while (user_choice.empty() && (confirmation.compare("y")!=0))
		{
			cout << "What do you want to do now?" << endl;
			cout << " <r>: run a new test, <q>: quit, <rq>: run a new test and quit "<<endl;
			cout << "Please type your choice "<<endl;
			cin >> user_choice;
			cout << "Your choice is " << user_choice << ", is it ok? (y/n) " << endl;
			cin >> confirmation;
		}
		if (user_choice.compare("q")!=0)
		{
			confirmation.clear();
			const unsigned int max_testable_address = MAX_TESTABLE_ADDRESS;
			while (	strInput_memaddrUT.empty() && strInput_nmbrTest.empty() && (confirmation.compare("y")!=0))
			{
				cout << "Please type in the maximum address to test (no more than "<< to_string(max_testable_address) << ")"<< endl;
				cin >> strInput_memaddrUT;
				try{
					memory_addr_under_test = stoul(strInput_memaddrUT);
				} catch  (const std::exception& e) {
					std::cerr << e.what() << '\n';
					cout << "WARNING something bad happened in the insertion, hence default used" << endl;
					memory_addr_under_test = 5;
				}
				if(memory_addr_under_test > MAX_TESTABLE_ADDRESS){
					cout << "WARNING the address inserted is not allowed, hence it will be moduled" << endl;
					memory_addr_under_test = memory_addr_under_test % MAX_TESTABLE_ADDRESS;
					strInput_memaddrUT.assign(to_string(memory_addr_under_test));
				}
				unsigned int max_testingNumber = ((int) pow(2,MAX_TEST_REPETITION_BITWIDTH) - 1);
				cout << "Please type in the repetition of the test (no more than " << to_string(max_testingNumber) << ")"<< endl;
				cin >> strInput_nmbrTest;
				try{
					testingNumber = stoul(strInput_nmbrTest);
				} catch (const std::exception& e) {
					std::cerr << e.what() << '\n';
					cout << "WARNING something bad happened in the insertion, hence default used" << endl;
					testingNumber = 3;
				}
				if(testingNumber > max_testingNumber){
					cout << "WARNING the repetition inserted is not allowed, hence it will be moduled" << endl;
					testingNumber = testingNumber % max_testingNumber;
					strInput_nmbrTest.assign(to_string(testingNumber));
				}
				cout << "Your choice is to test " << testingNumber << " times up to address " << memory_addr_under_test  << ", is it ok? (y/n) " << endl;
				cin >> confirmation;
			}
		}
		if (user_choice.compare("q")==0)
		{
			cout << "INFO: IBM ZRL Memtest is closing." << endl<< "Goodbye :D" << endl;
		}
		

	}//end of while loop
// Destructor closes the socket
}catch (SocketException & e) {
	cerr << e.what() << endl;
	cout << "INFO: there was a SocketException, now aborting" << endl;
	exit(1);
}


    return 0;
}




/*! \} */
