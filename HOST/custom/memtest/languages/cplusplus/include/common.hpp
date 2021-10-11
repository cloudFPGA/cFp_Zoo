#include <stdio.h>
#include <iostream>                     // For cout and cerr
#include <cstdlib>                      // For atoi()
#include <assert.h>                     // For assert()
#include <string>                       // For to_string
#include <string.h>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <sys/stat.h>
#include <stdlib.h>
//#include "../include/config.h"
#include "config.h"
#include<algorithm>
#include <fstream>
#include <bitset>
#include <vector>
#include <cmath>

using namespace std;
#define MAX_TESTABLE_ADDRESS 20
#define MAX_TEST_REPETITION_BITWIDTH 16


//------------------------------------------------------
//-- TESTBENCH DEFINES
//------------------------------------------------------
#define OK          true
#define KO          false


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


void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}



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


template<unsigned int bytes_per_line = 8>
string createMemTestCommands(unsigned int mem_address, unsigned int testingNumber)
{
  string out;
	char start_cmd [bytes_per_line]; // Half of the command filled with start other half with the address
	char stop_cmd [bytes_per_line];
	//char filler_cmd [bytes_per_line];
	char value_cmd[bytes_per_line];
    //WARNING: currently hardcoded way of start and stop commands with a 1 and 2 for start and stop respectively
	for (unsigned int k = 0; k < bytes_per_line; k++) {
       value_cmd[k]    = (char)0;
		if (k != 0) {
			stop_cmd[k] = (char)0;
			start_cmd[k] = (char)0;
	    }
	    else {
			start_cmd[k] = (char)1; 
			stop_cmd[k] = (char)2;
	    }
	 }
  memcpy(start_cmd+1, (char*)&testingNumber, 2);
	out.append(start_cmd,3);
  memcpy(value_cmd, (char*)&mem_address, 4);
  out.append(value_cmd,5);
  return out;
  }

template<unsigned int bytes_per_line = 8>
string createMemTestStopCommand()
{
  string out;
	char stop_cmd [bytes_per_line];
	for (unsigned int k = 0; k < bytes_per_line; k++) {
		if (k != 0) {
			stop_cmd[k] = (char)0;
	    }
	    else {
			stop_cmd[k] = (char)2;
	    }
	 }
  out.append(stop_cmd,bytes_per_line);
  return out;
  }

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

void string2hexnumerics(const string& in, char * out, size_t byteSize)
{
	for (int i = 0; i < byteSize; i++)
	{
		std::sprintf(out+i, "%d", (int)in[i]);
	}
}


struct MemoryTestResult {
  unsigned int    target_address;
  unsigned int    fault_cntr;
  unsigned int    first_fault_address;

  MemoryTestResult()      {}
  MemoryTestResult(unsigned int target_address, unsigned int fault_cntr, unsigned int  first_fault_address) :
    target_address(target_address), fault_cntr(fault_cntr), first_fault_address(first_fault_address) {}
};

template<unsigned int bytes_per_line=8>
std::vector<MemoryTestResult> parseMemoryTestOutput(string longbuf, size_t charOutputSize, int rawdatalines)
{
  std::vector<MemoryTestResult> testResults_vector;

  int rawiterations = charOutputSize / 8;
  //cout << "my calculations " << rawiterations << " the function iterations " << rawdatalines << endl;
  bool is_stop_present = rawdatalines % (3+1+1) == 0; //guard to check if multiple data of 3 64bytes or with 

  int k = 1;
  string tmp_outbuff;
  char myTmpOutBuff [bytes_per_line+1];
  unsigned int testingNumber_out=0, max_memory_addr_out=0, fault_cntr_out=0, fault_addr_out=0;
  for (int i = 1; i < rawdatalines+1; i++)
  {
    tmp_outbuff.clear();
    tmp_outbuff= longbuf.substr((i-1)*bytes_per_line,bytes_per_line);
    if(is_stop_present && k==5){
      cout << "DEBUG the stop is present and is here" << endl;
    } else  if( ( (i == rawdatalines-1) || (i == rawdatalines) ) && k==4){ //check it is either the last or one before the last
      //substr extraction and parsing
      tmp_outbuff.erase(tmp_outbuff.begin());
      strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line-1);
      testingNumber_out = *reinterpret_cast<unsigned long long*>(myTmpOutBuff);
      cout << "DEBUG last command with the iterations " << testingNumber_out << endl;

    } else  if(k==3){ //first faut addres
      //substr extraction and parsing
      strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line);
      fault_addr_out = *reinterpret_cast<unsigned long long*>(myTmpOutBuff);
      MemoryTestResult tmp(max_memory_addr_out,fault_cntr_out,fault_addr_out);
      testResults_vector.push_back(tmp);
      cout << "DEBUG first fault address (or the third data pckt) " << fault_addr_out << endl;
      if(!( (i+1 == rawdatalines-1) || (i+1 == rawdatalines) )){
        k=0;
      //cout << "DEBUG reinit the counter" << endl;
      }
      cout << "DEBUG overall test results: target address " << tmp.target_address << " ";
      cout << " fault counter: " << tmp.fault_cntr << " ";
      cout << "first fault at address: " << tmp.first_fault_address << " "  << endl;

    }else if(k==2){ // fault cntr
      strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line);
      fault_cntr_out = *reinterpret_cast<unsigned long long*>(myTmpOutBuff);
      cout << "DEBUG the fault counters (or the second data pack) " <<  fault_cntr_out << endl;
    }else { //max addrss
      //substr extraction and parsing
      strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line);
      max_memory_addr_out = *reinterpret_cast<unsigned long long*>(myTmpOutBuff);
      cout << "DEBUG max address (or the first data pack) " << max_memory_addr_out << endl;

    }
    k++;
  }
  tmp_outbuff.clear();
  return testResults_vector;
}