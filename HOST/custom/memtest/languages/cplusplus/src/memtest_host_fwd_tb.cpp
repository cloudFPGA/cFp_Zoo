/*****************************************************************************
 * @file       memtest_host_fwd_tb.cpp
 * @brief      Testbench for Memtest userspace application for cF (x86, ppc64).
 *
 * @date       May 2020
 * @author     DID
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup MemtestTB
 * @addtogroup MemtestTB
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
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include "../../../../../PracticalSockets/src/PracticalSockets.h"
#include "../include/config.h"
#include<algorithm>
#include <fstream>

//------------------------------------------------------
//-- TESTBENCH DEFINES
//------------------------------------------------------
#define OK          true
#define KO          false

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

unsigned char hexval(unsigned char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
    else if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    else abort();
}

void hex2ascii(const string& in, string& out)
{
    out.clear();
    out.reserve(in.length() / 2);
    for (string::const_iterator p = in.begin(); p != in.end(); p++)
    {
       unsigned char c = hexval(*p);
       p++;
       if (p == in.end()) break; // incomplete last digit - should report error
       c = (c << 4) + hexval(*p); // + takes precedence over <<
       out.push_back(c);
    }
}

template<unsigned int bytes_per_line = 8>
string dumpFileToStringRawDataString(const string inpFileName, int * rawdatalines, size_t outputSize) {
    string      strLine;
    string      tmp_Out;
    ifstream    inpFileStream;
    string      datFile = inpFileName;
    string charOutput;
    charOutput.reserve(outputSize);
    strLine.reserve(outputSize);
    tmp_Out.reserve(bytes_per_line);
    unsigned long long int  mylongunsigned;
    unsigned long long int  zero_byte=0;
    unsigned int i = 0;
    char my_tmp_buf [bytes_per_line];
    //-- STEP-1 : OPEN FILE
    inpFileStream.open(datFile.c_str());
cout<<endl<<endl;

    if ( !inpFileStream ) {
        cout << "### ERROR : Could not open the input data file " << datFile << endl;
        return "";
    }

    //-- STEP-2 : SET DATA STREAM
    while (inpFileStream) {

        if (!inpFileStream.eof()) {

            getline(inpFileStream, strLine);
            memcpy(my_tmp_buf,&zero_byte, bytes_per_line);
            //cout << strLine << endl;
            if (strLine.empty()) continue;
            *rawdatalines+=1;
            //sscanf(strLine.c_str(), "%llx", &mylongunsigned);
            mylongunsigned=stoul(strLine,nullptr,16);
            hex2ascii(strLine, tmp_Out);
            // Write to strOutput
     //	printf("my long long %llx\n", mylongunsigned);
     // printf("my long long non hex %llu\n", mylongunsigned);
      memcpy(my_tmp_buf,(char *)&mylongunsigned, sizeof(unsigned long long int));
     // printBits(sizeof(unsigned long long int), my_tmp_buf);
      // printBits(sizeof(unsigned long long int), tmp_Out.c_str());
      charOutput.append(my_tmp_buf, bytes_per_line);
      i++;
        }
        strLine.clear();
//cout<<endl<<endl;
    }
    //-- STEP-3: CLOSE FILE
    inpFileStream.close();

    return(charOutput);
}

void ascii2hexWithSize(const string& in, string& out, size_t  bytesize)
{
 std::stringstream sstream;
    for ( int i=0; i<bytesize; i++){
        sstream << std::hex << int(in[i]);
    }
    out=sstream.str(); 
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


void printStringHex(const string inStr, size_t strSize){
	printf("Going to prit a hex string :D\n");
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

void printCharBuffHexSafe(const char * inStr, size_t strSize){
	printf("Going to prit a hex char buff :D\n");
	for (size_t i = 0; i < strSize; i++)
	{
		char tmp = inStr[i];
		printf("%x",tmp);
	}
	printf("\n");
	
}

void ascii2hex(const string& in, string& out)
{
 std::stringstream sstream;
    for ( string::const_iterator item = in.begin(); item != in.end(); item++){
        sstream << std::hex << int(*item);
    }
    out=sstream.str(); 
}

void string2hexnumerics(const string& in, char * out, size_t byteSize)
{
	for (int i = 0; i < byteSize; i++)
	{
		std::sprintf(out+i, "%d", (int)in[i]);
	}
}
  /**
   *   Main testbench for the user-application for Memtest on host. Server
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {

    if ((argc < 2) || (argc > 4)) { // Test for correct number of parameters
        cerr << "Usage: " << argv[0] << " <Server Port> <optional simulation mode> <optional number of repetitions>" << endl;
        exit(1);
    }

    unsigned short servPort = atoi(argv[1]); // First arg:  local port
    unsigned int num_batch = 0;
    string clean_cmd, synth_cmd;
    string strInput_nmbrTest = argv[3];
    unsigned int testingNumber;
    
	if (!strInput_nmbrTest.length())
	{
		testingNumber = 3;
	}	else	{
		testingNumber = stoul(strInput_nmbrTest);
	}
	
	

    try {
      	#if NET_TYPE == udp
        UDPSocket sock(servPort);
	#else
	TCPServerSocket servSock(servPort);     // Server Socket object
	TCPSocket *servsock = servSock.accept();     // Wait for a client to connect
	#endif
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
	int total_pack = 1;
	int bytes_in_last_pack;
	size_t total_size =0;
	bool msg_received = false;
        cout << " ___________________________________________________________________ " << endl;
        cout << "/                                                                   \\" << endl;
	cout << "INFO: Proxy tb batch # " << ++num_batch << endl;	    
        char * longbuf = new char[PACK_SIZE * (total_pack+1)];
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
		//printCharBuffHex(buffer, recvMsgSize);

	    memcpy(longbuf+(i*PACK_SIZE), buffer, recvMsgSize);
		//printCharBuffHex(longbuf, recvMsgSize);
		total_size += recvMsgSize;
		longbuf[total_size+1]='\0';

		
	    //printf("DEBUG: recvMsgSize=%u strlen(buffer)=%u nullcharpos=%u\n", recvMsgSize, strlen(buffer), nullcharfound);
	    if (nullcharfound != true) {
		cout << "INFO: The string is not entirely fit in packet " <<  total_pack << endl;
	    }
	    else {
		msg_received = true;
	    }
        }

        cout << "INFO: Received packet from " << sourceAddress << ":" << sourcePort << endl;
 
	string input_string;
	//printCharBuffHex(longbuf, total_size);
	input_string.append(longbuf,total_size);
	//printStringHex(input_string,input_string.length());
	if (input_string.length() == 0) {
	    cerr << "ERROR: received an empty string! Aborting..." << endl;
            return -1;
	}

	//DECODING LOGIC for output size determination
	unsigned int memory_addr_under_test = 0;
	testingNumber = 0;
	printStringHex(input_string,input_string.length());
	// revert the input string and extract substring
	reverse(input_string.begin(), input_string.end());

	string substr_tmp, to_translate_String;
	substr_tmp = input_string.substr(5,2);
	ascii2hexWithSize(substr_tmp,to_translate_String,2);

	try{
	testingNumber = stoul(to_translate_String,nullptr,16);
	}catch(const std::exception& e){
	std::cerr << e.what() << '\n';
	testingNumber=0;
	}
	substr_tmp.clear();
	to_translate_String.clear();

	substr_tmp=input_string.substr(0,5);
	ascii2hexWithSize(substr_tmp,to_translate_String,5);
	try{
	memory_addr_under_test = stoul(to_translate_String,nullptr,16);
	}catch(const std::exception& e){
	std::cerr << e.what() << '\n';
	memory_addr_under_test=0;
	}
	substr_tmp.clear();
	to_translate_String.clear();
	reverse(input_string.begin(), input_string.end());
	//
	// Select simulation mode, default fcsim
	synth_cmd = " ";
	string exec_cmd = "make fcsim -j 4";
	string ouf_file = "../../../../../../ROLE/custom/hls/memtest/memtest_prj/solution1/fcsim/build/hls_out.txt";
	if (argc >= 3) {
	    if (atoi(argv[2]) == 2) {
		exec_cmd = "make csim";
		ouf_file = "../../../../../../ROLE/custom/hls/memtest/memtest_prj/solution1/csim/build/hls_out.txt";
	    }
	    else if (atoi(argv[2]) == 3) {
	        synth_cmd = "make csynth && ";
		exec_cmd = "make cosim";
		ouf_file = "../../../../../../ROLE/custom/hls/memtest/memtest_prj/solution1/sim/wrapc_pc/build/hls_out.txt";
	    }
	    else if (atoi(argv[2]) == 4) {
		exec_cmd = "make kcachegrind";
		ouf_file = "../../../../../../ROLE/custom/hls/memtest/memtest_prj/solution1/fcsim/build/hls_out.txt";
	    }
	}
	// Calling the actual TB over its typical makefile procedure, but passing the save file
	// Skip the rebuilding phase on the 2nd run. However ensure that it's a clean recompile
	// the first time.
	clean_cmd = " ";
	if (num_batch == 1) {
	    clean_cmd = "make clean && ";
	}
	string str_command = "cd ../../../../../../ROLE/custom/hls/memtest/ && ";
    str_command = str_command.append(clean_cmd + synth_cmd+ exec_cmd);//+" COMMAND_STRING=\"");
	size_t str_command_size = str_command.length();
	string hexInputString;
	ascii2hexWithSize(input_string, hexInputString, total_size);
	//str_command = str_command.append(hexInputString.c_str(),total_size);
	//str_command_size+=total_size;
	printStringHex(str_command,str_command_size);
	//string final_cmd = "\" TEST_NUMBER=" + std::to_string(testingNumber) + " INPUT_STRING=" + std::to_string(memory_addr_under_test) +	" && cd ../../../../HOST/custom/memtest/languages/cplusplus/build/ ";
	string final_cmd = " TEST_NUMBER=" + std::to_string(testingNumber) + " INPUT_STRING=" + std::to_string(memory_addr_under_test) +	" && cd ../../../../HOST/custom/memtest/languages/cplusplus/build/ ";
	str_command = str_command.append(final_cmd);
	str_command_size+=final_cmd.length();

  	//cout << "Calling TB with command:" << str_command << endl; 

	char *command =(char*)malloc((str_command_size+1)* sizeof(char));
	for(int i=0; i < (str_command_size+1); i++){
		command[i]=str_command[i];
	}
  	cout << "Calling TB with command:" << command << endl; 
//return 0;
	system(command); 
////////////////////////////////////////////////////////
//////////////TODO: need to check the proper emulation
////////////////////////////////////////////////////////
	ssize_t size = __file_size(ouf_file.c_str());
	size_t charOutputSize = 8*1+((8 * (2 + 1)) * testingNumber); //stop, 3 for each test, potential stop?

	//int rc = __file_read_hex(ouf_file.c_str(), longbuf, charOutputSize*2+1);
	int rawdatalines=0;
  	int rc = 0;
	string out_string =  dumpFileToStringRawDataString(ouf_file.c_str(), &rawdatalines, charOutputSize);
	memcpy(longbuf,out_string.data(),charOutputSize);
	//strncpy(longbuf, out_string.c_str(), charOutputSize);
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
	
	unsigned int total_retx_pack = 1;
	if (charOutputSize / PACK_SIZE > 1)
	{
		total_retx_pack = (unsigned int)(8 * (2 + 3 * testingNumber) / PACK_SIZE);
	}
	if (out_string.length() == 0) {
	      cerr << "ERROR: Received empty string!" << endl; 
	      return -1;
	}
	else {
	    cout << "INFO: Succesfully received string from TB : " << out_string << endl; 
		printStringHex(out_string, charOutputSize);
	    cout << "INFO: Will forward it back to host app ... total_pack=" << total_retx_pack << endl; 
		printCharBuffHexSafe(longbuf, charOutputSize);
	}

	// TX Loop
	unsigned int sending_now = PACK_SIZE;
	clock_t last_cycle_tx = clock();
	unsigned int bytes_in_last_pack_in = charOutputSize - (total_retx_pack - 1) * PACK_SIZE;
        for (int i = 0; i < total_retx_pack; i++) {
	    if ( i == total_retx_pack - 1 ) {
		sending_now = bytes_in_last_pack_in;
	    }
	    #if NET_TYPE == udp
	    sock.sendTo( longbuf+(i * PACK_SIZE), sending_now, sourceAddress, sourcePort);
	    #else
	    servsock->send( & longbuf[i * PACK_SIZE], sending_now);
	    #endif
	}
        clock_t next_cycle_tx = clock();
        double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
        cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
               total_retx_pack / duration_tx / 1024 * 8) << endl;
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
