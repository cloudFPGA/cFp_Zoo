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
#include <array>
#include <sys/stat.h>
#include "../../../PracticalSockets/src/PracticalSockets.h"
#include "../include/config.h"


static inline ssize_t
__file_size(const char *fname)
{
	int rc;
	struct stat s;

	rc = lstat(fname, &s);
	if (rc != 0) {
		fprintf(stderr, "err: Cannot find %s!\n", fname);
		return rc;
	}
	return s.st_size;
}

static inline ssize_t
__file_read(const char *fname, char *buff, size_t len)
{
	int rc;
	FILE *fp;

	if ((fname == NULL) || (buff == NULL) || (len == 0))
		return -EINVAL;

	fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "err: Cannot open file %s: %s\n",
			fname, strerror(errno));
		return -ENODEV;
	}
	rc = fread(buff, len, 1, fp);
	if (rc == -1) {
		fprintf(stderr, "err: Cannot read from %s: %s\n",
			fname, strerror(errno));
		fclose(fp);
		return -EIO;
	}
	fclose(fp);
	return rc;
}


bool findCharNullPos (char * str) {
    unsigned int len = strlen(str);
    bool f = false;
    for(unsigned int i=0; i<=len; i++) {
	if(str[i]=='\0') {
	  printf("DEBUG: null character position: %d\n",i+1);
	  f = true;
	}
    }
    if(f == false) {
	printf("DEBUG: null character not found\n");
    }
    return f;
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
    unsigned int num_batch = 0;
    string clean_cmd;
    
    try {
      	//#if NET_TYPE == udp
        UDPSocket sock(servPort);
	//#else
	TCPServerSocket servSock(servPort);     // Server Socket object
	TCPSocket *servsock = servSock.accept();     // Wait for a client to connect
	//#endif
        char buffer[BUF_LEN]; // Buffer for echo string
        unsigned int recvMsgSize; // Size of received message
        string sourceAddress; // Address of datagram source
        unsigned short sourcePort; // Port of datagram source
	    
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
    
	int input_string_total_len = 0;
	//int receiving_now = PACK_SIZE;
	int total_pack = 0;
	int bytes_in_last_pack;
	bool msg_received = false;
        cout << " ___________________________________________________________________ " << endl;
        cout << "/                                                                   \\" << endl;
	cout << "INFO: Proxy tb batch # " << ++num_batch << endl;	    
        char * longbuf = new char[PACK_SIZE * total_pack];
	    
	// RX Loop
        for (int i = 0; msg_received != true; i++, total_pack++) {
	    #if NET_TYPE == udp
            recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
	    #else
	    recvMsgSize = servsock->recv(buffer, receiving_now);
	    #endif
	    input_string_total_len += recvMsgSize;
	    bytes_in_last_pack = recvMsgSize;
	    bool nullcharfound = findCharNullPos(buffer);
	    //printf("DEBUG: recvMsgSize=%u strlen(buffer)=%u nullcharpos=%u\n", recvMsgSize, strlen(buffer), nullcharfound);
            memcpy( & longbuf[i * PACK_SIZE], buffer, recvMsgSize);
	    if (nullcharfound != true) {
		cout << "INFO: The string is not entirely fit in packet " <<  total_pack << endl;
	    }
	    else {
		msg_received = true;
	    }
        }

        cout << "INFO: Received packet from " << sourceAddress << ":" << sourcePort << endl;
 
	string input_string = longbuf;
	if (input_string.length() == 0) {
	    cerr << "ERROR: received an empty string! Aborting..." << endl;
            return -1;
	}
	    
	// Select simulation mode, default fcsim
	string exec_cmd = "make fcsim -j 4";
	string ouf_file = "../../../../ROLE/1/hls/uppercase/uppercase_prj/solution1/fcsim/build/hls_out.txt";
	if (argc == 3) {
	    if (atoi(argv[2]) == 2) {
		exec_cmd = "make csim";
		ouf_file = "../../../../ROLE/1/hls/uppercase/uppercase_prj/solution1/csim/build/hls_out.txt";
	    }
	    else if (atoi(argv[2]) == 3) {
		exec_cmd = "make csynth && make cosim";
		ouf_file = "../../../../ROLE/1/hls/uppercase/uppercase_prj/solution1/cosim/build/hls_out.txt";
	    }
	    else if (atoi(argv[2]) == 4) {
		exec_cmd = "make kcachegrind";
		ouf_file = "../../../../ROLE/1/hls/uppercase/uppercase_prj/solution1/fcsim/build/hls_out.txt";
	    }
	}
	// Calling the actual TB over its typical makefile procedure, but passing the save file
	// Skip the rebuilding phase on the 2nd run. However ensure that it's a clean recompile
	// the first time.
	clean_cmd = " ";
	if (num_batch == 1) {
	    clean_cmd = "make clean && ";
	}
	    
	string str_command = "cd ../../../../ROLE/1/hls/uppercase/ && " + clean_cmd + " " + 
			      exec_cmd + " && cd ../../../../HOST/languages/cplusplus/build/ "; 
	const char *command = str_command.c_str(); 
  	cout << "Calling TB with command:" << command << endl; 
	system(command); 

	ssize_t size = __file_size(ouf_file.c_str());
	int rc = __file_read(ouf_file.c_str(), longbuf, size);
	if (rc < 0) {
	    cerr << "ERROR: Cannot read file " << ouf_file << " . Aborting..."<< endl;
	    return -1;
	}

	clock_t next_cycle_rx = clock();
        double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective FPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * 
                total_pack / duration_rx / 1024 * 8) << endl;
        last_cycle_rx = next_cycle_rx;
    

	// TX step
	string out_string = longbuf;
	if (out_string.length() == 0) {
	      cerr << "ERROR: Received empty string!" << endl; 
	      return -1;
	}
	else {
	    cout << "INFO: Succesfully received string from TB : " << out_string << endl; 
	    cout << "INFO: Will forward it back to host app ... total_pack=" << endl; 
	}
	        

	// TX Loop
	unsigned int sending_now = PACK_SIZE;
	clock_t last_cycle_tx = clock();
        for (int i = 0; i < total_pack; i++) {
	    if ( i == total_pack - 1 ) {
		sending_now = bytes_in_last_pack;
	    }
	    #if NET_TYPE == udp
	    sock.sendTo( & longbuf[i * PACK_SIZE], sending_now, sourceAddress, sourcePort);
	    #else
	    servsock->send( & longbuf[i * PACK_SIZE], sending_now);
	    #endif
	}
            
        clock_t next_cycle_tx = clock();
        double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
               total_pack / duration_tx / 1024 * 8) << endl;
        last_cycle_tx = next_cycle_tx; 
        free(longbuf);
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
