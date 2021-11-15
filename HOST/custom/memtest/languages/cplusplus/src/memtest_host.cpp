
/*****************************************************************************
 * @file       memtest_host.cpp
 * @brief      Memtest userspace application for cF (x86, ppc64).
 *
 * @date       Sept 2021
 * @author     DID, DCO
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

#define MAX_MEM_SIZE_BENCHMARKING_POWER_OF_TWO 33
//20  for EMULATION (2^20 == 1MB) 33 --> 8 GB 
#define MIN_MEM_SIZE_BENCHMARKING_POWER_OF_TWO 6
#define MAX_BURST_SIZE_BENCHMARKING MAX_BURST_SIZE
#define MIN_BURST_SIZE_BENCHMARKING 1
#define REPETITIONS_BENCHMARKING 2


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
    if ((argc < 3) || (argc > 7)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port> <number of address to test> <testing times> <burst size> <optional list/interactive mode (type list or nothing)>\n";
        exit(1);
    }
#endif
	cout << argc << endl;
    
    //------------------------------------------------------
    //-- STEP-1 : Socket and variables definition
    //------------------------------------------------------
    
    #ifndef PY_WRAP
    assert (argc >= 6);
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
    std::string strInput_burstSize;
    
	std::string strInput_listMode;

    unsigned long long int memory_addr_under_test=0;
    unsigned int testingNumber = 1;
    unsigned int burst_size = 1;
    //UDPSocket *udpsock_p;
    //TCPSocket *tcpsock_p;
  
    print_cFpMemtest();
    
    try {
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
        //------------------------------------------------------
        //-- STEP-2 : Initialize socket connection
        //------------------------------------------------------    
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////  
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
    strInput_burstSize.assign(argv[5]);
	#endif
	// memory_addr_under_test = stoull(strInput_memaddrUT);
	// testingNumber = stoul(strInput_nmbrTest);
	// burst_size = stoul(strInput_burstSize);

	try{
		memory_addr_under_test = stoull(strInput_memaddrUT);
	} catch  (const std::exception& e) {
		std::cerr << e.what() << '\n';
		cout << "WARNING something bad happened in the insertion, hence default used" << endl;
		memory_addr_under_test = 64;
	}
	if(memory_addr_under_test > MAX_TESTABLE_ADDRESS || memory_addr_under_test<=0){
		cout << "WARNING the address inserted is not allowed, hence default use" << endl;
		memory_addr_under_test = 64;
		strInput_memaddrUT.assign(to_string(memory_addr_under_test));
	}
	unsigned int max_testingNumber = ((int) pow(2,MAX_TEST_REPETITION_BITWIDTH) - 1);
	try{
		testingNumber = stoul(strInput_nmbrTest);
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		cout << "WARNING something bad happened in the insertion, hence default used" << endl;
		testingNumber = 2;
	}
	if(testingNumber > max_testingNumber || testingNumber<=0){
		cout << "WARNING the repetition inserted is not allowed, hence default use" << endl;
		testingNumber = 2;
		strInput_nmbrTest.assign(to_string(testingNumber));
	}
	try{
		burst_size = stoul(strInput_burstSize);
	} catch  (const std::exception& e) {
		std::cerr << e.what() << '\n';
		cout << "WARNING something bad happened in the insertion, hence default used" << endl;
		burst_size = 16;
	}
	if(burst_size > MAX_BURST_SIZE || burst_size<=0){
		cout << "WARNING the burst size inserted is not allowed, hence default use" << endl;
		burst_size = 16;
		strInput_burstSize.assign(to_string(burst_size));
	}


	unsigned long long int max_size_mem_size = pow(2,MAX_MEM_SIZE_BENCHMARKING_POWER_OF_TWO);
	unsigned long long int min_size_mem_size = pow(2,MIN_MEM_SIZE_BENCHMARKING_POWER_OF_TWO);
	unsigned int max_burst_size = MAX_BURST_SIZE_BENCHMARKING;
	unsigned int min_burst_size = MIN_BURST_SIZE_BENCHMARKING;
	unsigned int desired_repetitions = REPETITIONS_BENCHMARKING;
	unsigned int burst_size_opposite = min_burst_size;
	
	if(argc==7){
		strInput_listMode.assign(argv[6]);
	}else{
		strInput_listMode.assign("");
	}
	bool use_the_list_mode=false;
	if(strInput_listMode.compare("list")==0){
		use_the_list_mode=true;
		std::cout << "Employing the List mode with max of " << max_size_mem_size << std::endl;
	}

	//------------------------------------------------------------------------------------
	//-- STEP-4 : Infinite Loop for reexecutinge the application! :D
	//------------------------------------------------------------------------------------
	string user_choice = "r";
	createAVGLogFile();
	createItLogFile();
	if(use_the_list_mode){
		testingNumber=desired_repetitions;
		memory_addr_under_test=min_size_mem_size;
		burst_size=max_burst_size;
	}//else take user params
	//Iterating and interactive loop
	while (user_choice.compare("q") != 0 ) // quit
	{
	
		unsigned int test_pack = 1;
		size_t charOutputSizeRoughBytes=8*1+((8 * (2 + 1 + 1 + 1)) * testingNumber);
		test_pack = charOutputSizeRoughBytes%PACK_SIZE==0 ? charOutputSizeRoughBytes/PACK_SIZE : charOutputSizeRoughBytes/PACK_SIZE + 1;
		size_t charOutputSize = PACK_SIZE*(test_pack); 
		string initial_input_string(input_string);

		cout << " Creating mem test commands with " << memory_addr_under_test << " " <<  testingNumber << " " << burst_size << " as addr, iters, brst" << endl;
		input_string=createMemTestCommands(memory_addr_under_test, testingNumber,burst_size);
		size_t charInputSize = 8*2; //a single tdata*burst

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
		unsigned int bytes_out_last_pack = input_string.length() - (total_out_pack-1) * PACK_SIZE;

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
		cout << "INFO: Bytes in last packet tx          = " << bytes_out_last_pack << endl;
		cout << "INFO: Bytes in last packet  rx         = " << bytes_in_last_pack_in << endl;
		cout << "INFO: Packet size (custom MTU)      = " << PACK_SIZE << endl;
			
		//------------------------------------------------------
		//-- STEP-5 : RUN MEMTEST FROM cF (HW)
		//------------------------------------------------------
		clock_t start_cycle_memtest_hw = clock();
			
		//------------------------------------------------------
		//-- STEP-5.1 : TX Loop
		//------------------------------------------------------
		clock_t last_cycle_tx = clock();
		unsigned int sending_now = PACK_SIZE;
		for (unsigned int i = 0; i < total_out_pack; i++) {
			if ( i == total_out_pack - 1 ) {
			sending_now = bytes_out_last_pack;
			}
			#if NET_TYPE == udp
			//printStringHex(input_string,charInputSize*sizeof(char));
			//printBits(charInputSize,input_string.c_str());
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
			bytes_received  += recvMsgSize;
			i ++;
		}

		cout << "INFO: Received packet from " << servAddress << ":" << servPort << endl;
	
		#ifdef PY_WRAP
		char *output_string = output_str;
		#else
		char *output_string = new char [(PACK_SIZE * total_in_pack*sizeof(char))];
		#endif
		memcpy( output_string, longbuf, bytes_received*sizeof(char));
		output_string[PACK_SIZE * total_in_pack]='\0';
		cout << "INFO: Received string : " << output_string << endl;
		//printCharBuffHex(output_string,bytes_received);

		clock_t next_cycle_rx = clock();
		double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
		cout << "INFO: Effective SPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * 
					total_in_pack / duration_rx / 1024 * 8) << endl;
		last_cycle_rx = next_cycle_rx;
		
		clock_t end_cycle_memtest_hw = next_cycle_rx;
			
		double duration_memtest_hw = (end_cycle_memtest_hw - start_cycle_memtest_hw) / 
										(double) CLOCKS_PER_SEC;
		cout << "INFO: HW exec. time:" << duration_memtest_hw << " seconds" << endl;
		cout << "INFO: Effective SPS HW:" << (1 / duration_memtest_hw) << " \tkbps:" << 
					(PACK_SIZE * total_in_pack / duration_memtest_hw / 1024 * 8) << endl;	  
			
		double duration_main = (clock() - start_cycle_main) / (double) CLOCKS_PER_SEC;
		cout << "INFO: Effective SPS E2E:" << (1 / duration_main) << endl;
		cout << "\\___________________________________________________________________/" << endl
		<< endl;
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
		//------------------------------------------------------
		//-- STEP-5.3 : Parsing and displaying
		//------------------------------------------------------ 
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
		cout << " ___________________________________________________________________ " << endl;
		cout << "/                                                                   \\" << endl;
		cout <<"INFO: Entering in the output parsing section  "<< endl;
		std::vector<MemoryTestResult> testResults_vector;
		string output_to_parse_string;
		output_to_parse_string.append(output_string,charOutputSizeRoughBytes);
		int rawdatalines = charOutputSizeRoughBytes / 8;;
		testResults_vector=parseMemoryTestOutput(output_to_parse_string,charOutputSizeRoughBytes,rawdatalines);
		//otuput showing
		cout << "INFO: Test Results appearing" << endl;
		cout << " The Memory test run for  " << testResults_vector.size() << " iterations " << endl;
		unsigned int mem_word_size = 512;
  		unsigned int mem_word_byte_size = mem_word_size/8;
		double rd_bndwdth=0.0;
		double wr_bndwdth=0.0;
		double avg_rd_bndwdth=0.0;
		double avg_wr_bndwdth=0.0;
		int avg_fault_cnt = 0;
		int iterations=0;
		unsigned long long int written_words=0;
		for(auto it = std::begin(testResults_vector); it != std::end(testResults_vector); ++it) {
			cout << " Test number " << it - testResults_vector.begin() << " stress " << 	it->target_address << " addresses "  << endl;
			cout << " it presented " << it->fault_cntr << " faults " << endl;
			cout << " and the first faulty address (if any) was " << it->first_fault_address << endl;
			written_words = ((it->target_address) %mem_word_byte_size) == 0 ? ((it->target_address)/mem_word_byte_size)  : (((it->target_address)/mem_word_byte_size) + 1);
			rd_bndwdth = ( (double)written_words*(double)mem_word_size / ( (double)it->clock_cycles_read * 6.4 ) ); // Gbit/T
			wr_bndwdth = ( (double)written_words*(double)mem_word_size / ( (double)it->clock_cycles_write * 6.4 ) );
			cout << " RD BW " << rd_bndwdth  << "[GBit/s], with  " << it->clock_cycles_read << " ccs" <<  endl;
      		cout << " WR BW " << wr_bndwdth << "[GBit/s], with  " << it->clock_cycles_write << " ccs" <<  endl;
			cout << endl << endl;
			avg_rd_bndwdth += rd_bndwdth;
			avg_wr_bndwdth += wr_bndwdth;
			avg_fault_cnt += it->fault_cntr;
			iterations++;

			logTheSingleResult(iterations, it->target_address, burst_size, written_words, rd_bndwdth, wr_bndwdth, it->fault_cntr, it->first_fault_address);
		}
		avg_rd_bndwdth = avg_rd_bndwdth / iterations;
		avg_wr_bndwdth = avg_wr_bndwdth / iterations;
		avg_fault_cnt = avg_fault_cnt / iterations;
		cout << "Based on " << iterations << " iterations, AVG WR " << avg_wr_bndwdth << " AVG RD " << avg_rd_bndwdth << " AVG faults " << avg_fault_cnt << endl;

		logTheAvgResult(iterations, memory_addr_under_test, burst_size, written_words, avg_rd_bndwdth, avg_wr_bndwdth, avg_fault_cnt);
		//clear dynamic memory
		delete [] longbuf;
		delete [] output_string;
		testResults_vector.clear();
		output_to_parse_string.clear();
		input_string.clear();
		initial_input_string.clear();
		strInput_memaddrUT.clear();
    	strInput_nmbrTest.clear();
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
		//------------------------------------------------------
		//-- STEP-5.4 : Interaction part
		//------------------------------------------------------ 
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
		//Need to close the application and send to the accelerator a stop for signaling a termination
		if (user_choice.compare("rq")==0)
		{
			cout << "INFO: going to close the application and send a stop command as the user requested." << endl;
			user_choice.clear();
			string cmd_string = createMemTestStopCommand();
			char stop_char_buff [8];
			memcpy(stop_char_buff, cmd_string.data(), 8);
			sending_now = 8;
			//printCharBuffHex(stop_char_buff,8);
			#if NET_TYPE == udp
				udpsock.sendTo(stop_char_buff, sending_now, servAddress, servPort);
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
				//printStringHex(output_stop,8);
			}
			cout << "INFO: IBM ZRL Memtest is closing." << endl<< "Goodbye :D" << endl;
			break;
		}
		
		user_choice.clear();
		string confirmation="";

		cout << "That is all from this application." << endl;

		if(use_the_list_mode){
			user_choice.assign("r");
			testingNumber=desired_repetitions;
			burst_size=burst_size/2;
			burst_size_opposite=burst_size_opposite*2;
			cout << " burst size  " << burst_size << " burst oppostit " << burst_size_opposite << endl; 
			if(burst_size_opposite>max_burst_size){ //if iterated all the burst sizes quit
	//			memory_addr_under_test=min_size_mem_size;
				burst_size=max_burst_size;
				burst_size_opposite=min_burst_size;
				memory_addr_under_test=memory_addr_under_test*2;
				memory_addr_under_test=memory_addr_under_test==max_size_mem_size ? memory_addr_under_test-64 : memory_addr_under_test;
				if(memory_addr_under_test>max_size_mem_size){ //if iterated all the mem trgt address increment the burst
					user_choice.assign("q");
				}
			}
			//unsigned long long int max_gb =  8*pow(10,9);
			//memory_addr_under_test=memory_addr_under_test%max_gb;

		}else{

			while (user_choice.empty() || (confirmation.compare("y")!=0) || ( (confirmation.compare("y")==0)  && (user_choice.compare("r")!=0 && user_choice.compare("q")!=0) && user_choice.compare("rq")!=0) )
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
				const unsigned long long int max_testable_address = MAX_TESTABLE_ADDRESS;
				while (	strInput_memaddrUT.empty() || strInput_nmbrTest.empty() || strInput_burstSize.empty() || (confirmation.compare("y")!=0))
				{
					cout << "Please type in the maximum address to test (no more than "<< to_string(max_testable_address) << ")"<< endl;
					cin >> strInput_memaddrUT;
					try{
						memory_addr_under_test = stoull(strInput_memaddrUT);
					} catch  (const std::exception& e) {
						std::cerr << e.what() << '\n';
						cout << "WARNING something bad happened in the insertion, hence default used" << endl;
						memory_addr_under_test = 64;
					}
					if(memory_addr_under_test > MAX_TESTABLE_ADDRESS || memory_addr_under_test<=0){
						cout << "WARNING the address inserted is not allowed, hence default use" << endl;
						memory_addr_under_test = 64;
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
					if(testingNumber > max_testingNumber || testingNumber<=0){
						cout << "WARNING something bad happened in the insertion, hence default used" << endl;
						testingNumber = 2;
						strInput_nmbrTest.assign(to_string(testingNumber));
					}
					cout << "Please type in the desired burst size (no more than "<< to_string(MAX_BURST_SIZE) << ")"<< endl;
					cin >> strInput_burstSize;
					try{
						burst_size = stoul(strInput_burstSize);
					} catch  (const std::exception& e) {
						std::cerr << e.what() << '\n';
						cout << "WARNING something bad happened in the insertion, hence default used" << endl;
						burst_size = 16;
					}
					if(burst_size > MAX_BURST_SIZE || burst_size<=0){
						cout << "WARNING the burst size inserted is not allowed, hence default use" << endl;
						burst_size = 16;
						strInput_burstSize.assign(to_string(burst_size));
					}
					cout << "Your choice is to test " << testingNumber << " times up to address " << memory_addr_under_test  << " burst size " << burst_size << ", is it ok? (y/n) " << endl;
					cin >> confirmation;
				}
			}
		}  //else interactive
		if (user_choice.compare("q")==0)
		{
			cout << "INFO: IBM ZRL Memtest is closing." << endl<< "Goodbye :D" << endl;
			user_choice.clear();
			string cmd_string = createMemTestStopCommand();
			char stop_char_buff [8];
			memcpy(stop_char_buff, cmd_string.data(), 8);
			sending_now = 8;
			//printCharBuffHex(stop_char_buff,8);
			#if NET_TYPE == udp
				udpsock.sendTo(stop_char_buff, sending_now, servAddress, servPort);
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
				//printStringHex(output_stop,8);
			}
			cout << "INFO: IBM ZRL Memtest is closing." << endl<< "Goodbye :D" << endl;
			break;
		} // if to quit
		
	}//end of while loop

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// Destructor closes the socket
}catch (SocketException & e) {
	cerr << e.what() << endl;
	cout << "INFO: there was a SocketException, now aborting" << endl;
	exit(1);
}


    return 0;
}




/*! \} */
