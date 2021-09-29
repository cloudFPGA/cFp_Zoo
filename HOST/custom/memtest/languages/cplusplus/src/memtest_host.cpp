
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

using namespace std;

void ascii2hex(const string& in, string& out)
{
 std::stringstream sstream;
    for ( string::const_iterator item = in.begin(); item != in.end(); item++){
        sstream << std::hex << int(*item);
    }
    out=sstream.str(); 
}

void attachCommand(const string& in, string& out)
{
	//string start_cmd = "0F0F0F0F0F0F0F0F";
	string start_cmd = "0100000000000000";
	//string stop_cmd = "0E0E0E0E0E0E0E0E";
	string stop_cmd = "0000000000000000";
	out = start_cmd;
	unsigned int bytes_per_line = 8;
	char value[bytes_per_line];
        unsigned int total_bytes = 0;
	for (unsigned int i = 0; i < in.length(); i+=bytes_per_line, total_bytes+=bytes_per_line) {
	    for (unsigned int k = 0; k < bytes_per_line; k++) {
	      if (i+k < in.length()) {
		value[k] = in[i+k];
	      }
	      else {
		value[k] = '0';
	      }
	    }
	   out.append(value,bytes_per_line);
	}
	out.append(stop_cmd);
}
void attachBitsCommandAndRefill(const string& in, string& out)
{
	unsigned int bytes_per_line = 8;
	char start_cmd [bytes_per_line];
	char stop_cmd [bytes_per_line];
	for (unsigned int k = 0; k < bytes_per_line; k++) {
		if (k != 0) {
			start_cmd[k] = (char)0;
			stop_cmd[k] = (char)0;
	      	}
	      	else {
			start_cmd[k] = (char)1;
			stop_cmd[k] = (char)0;
	      	}
	 }
	out = start_cmd;
	char value[bytes_per_line];
        unsigned int total_bytes = 0;
	for (unsigned int i = 0; i < in.length(); i+=bytes_per_line, total_bytes+=bytes_per_line) {
	    for (unsigned int k = 0; k < bytes_per_line; k++) {
	      if (i+k < in.length()) {
		value[k] = in[i+k];
	      }
	      else {
		value[k] = '0';
	      }
	    }
	   out.append(value,bytes_per_line);
	}
	out.append(stop_cmd);
}

void printStringHex(const string inStr, size_t strSize){
	printf("Going to prit a hex string :D\ns");
	for (size_t i = 0; i < strSize; i++)
	{
		printf("%x",inStr[i]);
	}
	printf("\n");
	
}

void printCharBuffHex(const char * inStr, size_t strSize){
	printf("Going to prit a hex char buff :D\n");
	for (size_t i = 0; i < strSize; i++)
	{
		printf("%x",inStr[i]);
	}
	printf("\n");
	
}

void createMemTestCommands(unsigned int mem_address, string& out, int testingNumber)
{
	unsigned int bytes_per_line = 8;
	char start_cmd [bytes_per_line]; // Half of the command filled with start other half with the address
	char stop_cmd [bytes_per_line];
	char filler_cmd [bytes_per_line];
    //WARNING: currently hardcoded way of start and stop commands with a 1 and 2 for start and stop respectively
	for (unsigned int k = 0; k < bytes_per_line; k++) {
        filler_cmd[k]    = (char)0;
		if (k != 0) {
			stop_cmd[k] = (char)0;
			start_cmd[k] = (char)0;
	    }
	    else {
			start_cmd[k] = (char)1; 
			stop_cmd[k] = (char)2;
	    }
	 }
	out.append(start_cmd,bytes_per_line/2);
	//printStringHex(out,bytes_per_line/2);
	char value[bytes_per_line];
    memcpy(value, (char*)&mem_address, sizeof(unsigned int));
    out.append(value,bytes_per_line/2);
	//printStringHex(out,bytes_per_line);
    for (int i = 0; i < (testingNumber * (2 * (mem_address+1)) + 2); i++){
	    out.append(filler_cmd,bytes_per_line);
    }
	out.append(stop_cmd,bytes_per_line);
	//printStringHex(out,bytes_per_line*2+bytes_per_line*(testingNumber * (2 * (mem_address+1)) + 2));

}

void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

void print_cFpMemtest(void)
{
	// http://patorjk.com/software/taag/#p=display&f=ANSI%20Shadow&t=cFp_Memtest
        cout <<  "                                                          " << endl;
	cout <<  "...build with:                                            " << endl;
	cout <<  " ██████╗███████╗██████╗    ███╗   ███╗███████╗███╗   ███╗████████╗███████╗███████╗████████╗" << endl;
	cout <<  "██╔════╝██╔════╝██╔══██╗   ████╗ ████║██╔════╝████╗ ████║╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝" << endl;
	cout <<  "██║     █████╗  ██████╔╝   ██╔████╔██║█████╗  ██╔████╔██║   ██║   █████╗  ███████╗   ██║   " << endl;
	cout <<  "██║     ██╔══╝  ██╔═══╝    ██║╚██╔╝██║██╔══╝  ██║╚██╔╝██║   ██║   ██╔══╝  ╚════██║   ██║   " << endl;
	cout <<  "╚██████╗██║     ██║███████╗██║ ╚═╝ ██║███████╗██║ ╚═╝ ██║   ██║   ███████╗███████║   ██║   " << endl;
	cout <<  " ╚═════╝╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝╚══════╝╚═╝     ╚═╝   ╚═╝   ╚══════╝╚══════╝   ╚═╝   " << endl;
	cout <<  "A cloudFPGA project from IBM ZRL               v0.1 --dco " << endl;
	cout <<  "                                                          " << endl;
}
                                                                                                    




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
    int testingNumber = 1;
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

    size_t charOutputSize = PACK_SIZE*testingNumber; 
	//8 * (2 + 1) * testingNumber; this should be the real number but seems to send everything

	string initial_input_string(input_string);

	createMemTestCommands(memory_addr_under_test, input_string, testingNumber);
	size_t charInputSize = ( (testingNumber * (2 * (memory_addr_under_test+1)) + 2) + 2 ) * 8;
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
   
        unsigned int total_out_pack  =  1 + (input_string.length() - 1) / PACK_SIZE;// only a single tx
        unsigned int total_in_pack  = testingNumber;
		

        unsigned int total_out_bytes = charInputSize;
		////////////////////////
		//////////////TODO: check with others the functionalities////////////////////////
		////////////////////////
        unsigned int total_in_bytes = total_in_pack * PACK_SIZE;//TODO
		//size_t charOutputSize = 8 * (2 + 1) * testingNumber;
        unsigned int bytes_in_last_pack_out = input_string.length() - (total_out_pack-1) * PACK_SIZE;
        unsigned int bytes_in_last_pack_in = 8 * 3; //8 bytes for three tdata words
		//input_string.length() - (total_pack - 1) * PACK_SIZE;

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
	unsigned int receiving_now = PACK_SIZE;
        cout << "INFO: Expecting length of packs:" << total_in_pack << endl;
        char * longbuf = new char[PACK_SIZE * total_in_pack];
        for (unsigned int i = 0; i < charOutputSize; ) {
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
            memcpy( & longbuf[i], buffer, recvMsgSize);
	    //cout << "DEBUG: recvMsgSize=" << recvMsgSize << endl;
	    i += recvMsgSize;
        }

        cout << "INFO: Received packet from " << servAddress << ":" << servPort << endl;
 
	#ifdef PY_WRAP
	char *output_string = output_str;
	#else
        char *output_string = (char*)malloc(PACK_SIZE * total_in_pack*sizeof(char));
	#endif
	output_string = strncpy(output_string, longbuf, PACK_SIZE * total_in_pack*sizeof(char));
	output_string[PACK_SIZE * total_in_pack]='\0';
	cout << "INFO: Received string : " << output_string << endl;
	for(int i=0; i<total_in_pack; i++){
		printCharBuffHex(output_string+(i*PACK_SIZE),(2+1)*8);
		
	}

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
  
    // Destructor closes the socket
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }

    return 0;
}




/*! \} */
