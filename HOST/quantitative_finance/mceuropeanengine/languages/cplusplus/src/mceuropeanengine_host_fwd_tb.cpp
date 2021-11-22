/*****************************************************************************
 * @file       uppercase_host_fwd_tb.cpp
 * @brief      Testbench for Uppercase userspace application for cF (x86, ppc64).
 *
 * @date       May 2020
 * @author     DID
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup UppercaseTB
 * @addtogroup UppercaseTB
 * \{
 *****************************************************************************/

#include <cstdlib>           // For atoi()
#include <iostream>          // For cout and cerr
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <string.h>
#include <assert.h>         // For assert()
#include <array>
#include <sys/stat.h>
#include <fstream>
#include <limits>
#include "../../../../../PracticalSockets/src/PracticalSockets.h"
#include "../include/config.h"


static inline unsigned int 
fileRead(const char *fname, DtUsed *buff, size_t len) {

    if ((fname == NULL) || (buff == NULL) || (len == 0))
	return -EINVAL;
    
    std::ifstream ifile(fname, std::ios::in);

    //check to see that the file was opened correctly:
    if (!ifile.is_open()) {
        std::cerr << "ERROR: There was a problem opening the input file " + string(fname)+ " \n";
        return -EIO;
    }

    unsigned int i = 0;
    string text;
    //keep storing values from the text file so long as data exists:
    while (ifile >> text) {
	if((text == "Inf") || (text == "-Inf"))  {
	  buff[i] = (DtUsed)std::numeric_limits<double>::infinity();
	}
	else if((text == "nan") || (text == "-nan"))  {
	  buff[i] = (DtUsed)std::numeric_limits<double>::quiet_NaN();
	}
	else {
	  buff[i] = (DtUsed)atof(text.c_str());
	}
	//cout << "DEBUG fileRead: " << i << " : " <<  buff[i] << endl;
	i++;
	if (i == len) {
	  break;
	}
    }
    // We ensure the reading of len values, not less or more
    assert (i == len);
    return i;
}




/*****************************************************************************
 * @brief Fill an output file with data from an image.
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
unsigned int 
writeStructToConfFile(const char *fname, varin *instruct) {

    if ((fname == NULL) || (instruct == NULL))
	return -EINVAL;
    
    std::ofstream ifile(fname);

    //check to see that the file was opened correctly:
    if (!ifile.is_open()) {
        std::cerr << "There was a problem creating the output file!\n";
        return -EIO;
    }
    unsigned int i, j;
    for (i = 0, j = 0; i < sizeof(varin); i+=sizeof(DtUsed), j++) {
      switch(j)
      {
	case 0:
	  ifile << instruct->loop_nm << endl;
	  break;
	case 1:
	  ifile << instruct->seed << endl;
	  break;  
	case 2:
	  ifile << instruct->underlying << endl;
	  break;
	case 3:
	  ifile << instruct->volatility << endl;
	  break;
	case 4:
	  ifile << instruct->dividendYield << endl;
	  break;
	case 5:
	  ifile << instruct->riskFreeRate << endl;
	  break;
	case 6:
	  ifile << instruct->timeLength << endl;
	  break;  
	case 7:
	  ifile << instruct->strike << endl;
	  break;
	case 8:
	  ifile << instruct->optionType << endl;
	  break;
	case 9:
	  ifile << instruct->requiredTolerance << endl;
	  break;
	case 10:
	  ifile << instruct->requiredSamples << endl;
	  break;
	case 11:
	  ifile << instruct->timeSteps << endl;
	  break;  
	case 12:
	  ifile << instruct->maxSamples << endl;
	  break;
	default:
	  break;
      }
    }
    ifile.close();
    return (i);
}












  /**
   *   Main testbench for the user-application for MCEuropeanEngine on host. Server
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {

    if ((argc < 2) || (argc > 3)) { // Test for correct number of parameters
        cerr << "Usage: " << argv[0] << " <Server Port> <optional simulation mode>" << endl;
        exit(1);
    }

    unsigned short servPort = atoi(argv[1]); // First arg:  local port
    if ((servPort <= 0) || (servPort <= 1024) || (servPort >= 65536)) {
        cerr << "ERROR: Invalid port number " << servPort <<
        ". Please select a value at range [1025-65535]. Aborting..." << endl;
        exit(1);
    }
    unsigned int num_batch = 0;
    string clean_cmd, synth_cmd;
    
    try {
        cout << " ___________________________________________________________________ " << endl;
        cout << "/                                                                   \\" << endl;
	cout << "INFO: Proxy tb batch # " << ++num_batch << endl;   
      	#if NET_TYPE == udp
        UDPSocket sock(servPort);
	#else
	TCPServerSocket servSock(servPort);	// Server Socket object
	TCPSocket *servsock = servSock.accept();// Wait for a client to connect
	#endif
        char buffer[BUF_LEN]; 			// Buffer for echo string
        unsigned int recvMsgSize; 		// Size of received message
        string sourceAddress = "localhost"; 	// Address of datagram source
	    
	#if NET_TYPE == tcp
	// TCP client handling
	cout << "Handling client ";
	try {
	  cout << servsock->getForeignAddress() << ":";
	} catch (SocketException e) {
	    cerr << "Unable to get foreign address" << endl;
	  }
	try {
	  cout << servsock->getForeignPort();
	} catch (SocketException e) {
	    cerr << "Unable to get foreign port" << endl;
	  }
	cout << endl;
	#endif
	    
        // RX Step
        clock_t last_cycle_rx = clock();

	// Block until receive message from a client
        varin instruct;
	int input_string_total_len = 0;
	#if NET_TYPE == tcp
	int receiving_now_rx = sizeof(instruct);
	#endif
	int total_pack_rx = 1 + (sizeof(instruct) - 1) / PACK_SIZE;
        char * longbuf = new char[PACK_SIZE * total_pack_rx];
	    
	// RX Loop
        for (unsigned int i = 0; i < sizeof(instruct); ) {
	    #if NET_TYPE == udp
            recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, servPort);
	    #else
	    recvMsgSize = servsock->recv(buffer, receiving_now_rx);
	    #endif
	    input_string_total_len += recvMsgSize;
	    //bytes_in_last_pack_rx = recvMsgSize;
            memcpy( & longbuf[i * PACK_SIZE], buffer, recvMsgSize);
	    i += recvMsgSize;
        }
	memcpy(&instruct, & longbuf[0], sizeof(instruct));
        cout << "INFO: Received packet from " << sourceAddress << ":" << servPort << endl;
	
	printf("DEBUG instruct.loop_nm = %u\n", (unsigned int)instruct.loop_nm);
	printf("DEBUG instruct.seed = %u\n", (unsigned int)instruct.seed);
	printf("DEBUG instruct.underlying = %f\n", instruct.underlying);
	printf("DEBUG instruct.volatility = %f\n", instruct.volatility);
	printf("DEBUG instruct.dividendYield = %f\n", instruct.dividendYield);
	printf("DEBUG instruct.riskFreeRate = %f\n", instruct.riskFreeRate);
	printf("DEBUG instruct.timeLength = %f\n", instruct.timeLength);
	printf("DEBUG instruct.strike = %f\n", instruct.strike);
	printf("DEBUG instruct.optionType = %u\n", (unsigned int)instruct.optionType);
	printf("DEBUG instruct.requiredTolerance = %f\n", instruct.requiredTolerance);
	printf("DEBUG instruct.requiredSamples = %u\n", (unsigned int)instruct.requiredSamples);
	printf("DEBUG instruct.timeSteps = %u\n", (unsigned int)instruct.timeSteps);
	printf("DEBUG instruct.maxSamples = %u\n", (unsigned int)instruct.maxSamples);
	    
	if (writeStructToConfFile("../../../../../../ROLE/quantitative_finance/hls/mceuropeanengine/etc/mce_from_net.conf", &instruct) != sizeof(varin)) {
	  cerr << "ERROR: Cannot write struct to configuration file. Aborting ..." << endl;
	  return (-1);
	}
	
	// Select simulation mode, default fcsim
	synth_cmd = " ";
	string exec_cmd = "make fcsim -j 4";
	string ouf_file = "../../../../../../ROLE/quantitative_finance/hls/mceuropeanengine/mceuropeanengine_prj/solution1/fcsim/build/hls_out.txt";
	if (argc == 3) {
	    if (atoi(argv[2]) == 2) {
		exec_cmd = "make csim";
		ouf_file = "../../../../../../ROLE/quantitative_finance/hls/mceuropeanengine/mceuropeanengine_prj/solution1/csim/build/hls_out.txt";
	    }
	    else if (atoi(argv[2]) == 3) {
		synth_cmd = "make csynth && ";
		exec_cmd = "make cosim";     
		ouf_file = "../../../../../../ROLE/quantitative_finance/hls/mceuropeanengine/mceuropeanengine_prj/solution1/sim/wrapc_pc/hls_out.txt";
	    }
	    else if (atoi(argv[2]) == 4) {
		exec_cmd = "make kcachegrind";
		ouf_file = "../../../../../../ROLE/quantitative_finance/mceuropeanengine/mceuropeanengine_prj/solution1/fcsim/build/hls_out.txt";
	    }
	}
	// Calling the actual TB over its typical makefile procedure, but passing the save file
	// Skip the rebuilding phase on the 2nd run. However ensure that it's a clean recompile
	// the first time.
	clean_cmd = " ";
	if (num_batch == 1) {
	    clean_cmd = "make clean && ";
	}
	    
	string str_command = "cd ../../../../../../ROLE/quantitative_finance/hls/mceuropeanengine/ && " + clean_cmd + synth_cmd + "\
				  INPUT_FILE=./etc/mce_from_net.conf " + exec_cmd + " && \
				  cd ../../../../HOST/quantitative_finance/mceuropeanengine/languages/cplusplus/build/ "; 
	const char *command = str_command.c_str(); 
  	cout << "Calling TB with command:" << command << endl; 
	system(command); 


	ssize_t size = instruct.loop_nm*sizeof(DtUsed);
	unsigned int total_pack_tx  = 1 + (size - 1) / PACK_SIZE;
        unsigned int bytes_in_last_pack_tx = instruct.loop_nm*sizeof(DtUsed) - (total_pack_tx - 1) * PACK_SIZE;
	
	
	// Reallocate longbuf for Tx size
	delete longbuf;
	longbuf = new char[PACK_SIZE * total_pack_tx];
	memset(longbuf, 0, PACK_SIZE * total_pack_tx * sizeof(char));
	DtUsed * out = new DtUsed[instruct.loop_nm];
	memset(out, 0, instruct.loop_nm * sizeof(DtUsed));
	unsigned int rc = fileRead(ouf_file.c_str(), out, instruct.loop_nm);
	if (rc < 0) {
	    cerr << "ERROR: Cannot read file " << ouf_file << " . Aborting..."<< endl;
	    return -1;
	}

	memcpy(longbuf, out, instruct.loop_nm*sizeof(DtUsed)); 
	
	clock_t next_cycle_rx = clock();
        double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective FPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * 
                total_pack_rx / duration_rx / 1024 * 8) << endl;
        last_cycle_rx = next_cycle_rx;
    

	// TX step
	cout << "INFO: Succesfully received out vector from TB " << endl; 
	cout << "INFO: Will forward it back to host app ... total_pack_tx=" << total_pack_tx << endl; 
	        

	// TX Loop
	unsigned int sending_now = PACK_SIZE;
	clock_t last_cycle_tx = clock();
        for (unsigned int i = 0; i < total_pack_tx; i++) {
	    if ( i == total_pack_tx - 1 ) {
		sending_now = bytes_in_last_pack_tx;
	    }
	    #if NET_TYPE == udp
	    sock.sendTo( & longbuf[i * PACK_SIZE], sending_now, sourceAddress, servPort);
	    #else
	    servsock->send( & longbuf[i * PACK_SIZE], sending_now);
	    #endif
	}
            
        clock_t next_cycle_tx = clock();
        double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
               (total_pack_rx + total_pack_tx)/ duration_tx / 1024 * 8) << endl;
        last_cycle_tx = next_cycle_tx; 
        delete longbuf;
	delete out;
        cout << "\\___________________________________________________________________/" << endl;
        
	#if NET_TYPE == tcp
        delete servsock;
	#endif
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }
    return 0;
}


/*! \} */
