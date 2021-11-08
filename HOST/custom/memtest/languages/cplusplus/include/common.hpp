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
#include <chrono>
#include <ctime> 

using namespace std;
#define MAX_TESTABLE_ADDRESS ((int)(512/8 * 125000000)) //byte addressable!!!
#define MAX_TEST_REPETITION_BITWIDTH 16
#define MAX_BURST_SIZE 512 


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

/*****************************************************************************
 * @brief print the binary representation of a target pointer buffer of a given size.
 *      Assumes little endian.
 * @param[in]  size the bytesize to print from ptr.
 * @param[in] ptr the buffer pointer.
 * @return nothing, print to stdout.
 ******************************************************************************/
void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

/*****************************************************************************
 * @brief Convert a ascii string to a hexadecimal string
 *
 * @param[in]  in the input ascii string
 * @param[out] out the output hexadecimal string
 ******************************************************************************/
void ascii2hex(const string& in, string& out)
{
 std::stringstream sstream;
    for ( string::const_iterator item = in.begin(); item != in.end(); item++){
        sstream << std::hex << int(*item);
    }
    out=sstream.str(); 
}

/*****************************************************************************
 * @brief Create the commands for a memory test with start/max address to test-nop to execute-stop
 *
 * @param[in]  mem_address max target address to test
 * @param[out] out the output string with start/max address-nops4trgtCCsNeeded-stop
 * @param[in]  testingNumber the number of tests to perform on the memory
 * 
 ******************************************************************************/
template<unsigned int bytes_per_line = 8>
string createMemTestCommands(unsigned int mem_address, unsigned int testingNumber, unsigned int burst_size)
{
  string out;
    char start_cmd [bytes_per_line];
    char stop_cmd [bytes_per_line];
    char value_cmd[bytes_per_line];
    char burst_cmd[bytes_per_line];
    //WARNING: currently hardcoded way of start and stop commands with a 1 and 2 for start and stop respectively
    for (unsigned int k = 0; k < bytes_per_line; k++) {
       value_cmd[k]    = (char)0;
        if (k != 0) {
            stop_cmd[k] = (char)0;
            start_cmd[k] = (char)0;
            burst_cmd[k] = (char)0;
        }
        else {
            start_cmd[k] = (char)1; 
            stop_cmd[k] = (char)2;
            burst_cmd[k] = (char)4;
        }
     }
  memcpy(start_cmd+1, (char*)&testingNumber, 2);
    out = out.append(start_cmd,3);
  memcpy(value_cmd, (char*)&mem_address, 4);
  out = out.append(value_cmd,5);
  memcpy(burst_cmd+1,(char*)&burst_size,2);
  out = out.append(burst_cmd,8);  
  return string(out);
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

/*****************************************************************************
 * @brief convert a char to its hexadecimal representation.
 *
 * @param[in]  c the standard char value to convert to hex
 * @return     the hexadecimal value of the input
 ******************************************************************************/
// C++98 guarantees that '0', '1', ... '9' are consecutive.
// It only guarantees that 'a' ... 'f' and 'A' ... 'F' are
// in increasing order, but the only two alternative encodings
// of the basic source character set that are still used by
// anyone today (ASCII and EBCDIC) make them consecutive.
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

/*****************************************************************************
 * @brief Convert a hexadecimal string to a ascii string
 *
 * @param[in]  in the input hexadecimal string
 * @param[out] out the output ascii string
 ******************************************************************************/
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

template<typename T> 
void number2hexString(const T in, char * out, size_t byteSize)
{
	std::sprintf(out,byteSize, "%x", (char*)&in);
}

/*****************************************************************************
 * @brief Initialize an input data stream from a file with only data
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] strOutput the output string to set.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
template<unsigned int bytes_per_line = 8>
string dumpFileToStringRawDataString(const string inpFileName, int * rawdatalines, size_t outputSize) {
    string      strLine;
    string      tmp_Out;
    ifstream    inpFileStream;
    string      datFile = inpFileName;
    string charOutput;
    //charOutput.reserve(outputSize);
    //strLine.reserve(outputSize);
    //tmp_Out.reserve(bytes_per_line);
    unsigned long long int  mylongunsigned;
    unsigned long long int  zero_byte=0;
    unsigned int i = 0;
    char my_tmp_buf [bytes_per_line];
    //-- STEP-1 : OPEN FILE
    inpFileStream.open(datFile.c_str());

    if ( !inpFileStream ) {
        cout << "### ERROR : Could not open the input data file " << datFile << endl;
        return "";
    }

    //-- STEP-2 : SET DATA STREAM
    while (inpFileStream) {

        if (!inpFileStream.eof()) {

            getline(inpFileStream, strLine);
            memcpy(my_tmp_buf,&zero_byte, bytes_per_line);
            if (strLine.empty()) continue;
            *rawdatalines+=1;
            mylongunsigned=stoul(strLine,nullptr,16);
            hex2ascii(strLine, tmp_Out);
            // Write to strOutput
      memcpy(my_tmp_buf,(char *)&mylongunsigned, sizeof(unsigned long long int));
      charOutput.append(my_tmp_buf, bytes_per_line);
      i++;
        }
    }
    //-- STEP-3: CLOSE FILE
    inpFileStream.close();
    //tmp_Out.clear();

    return string(charOutput);
}

/*****************************************************************************
 * @brief Convert a ascii string to a hexadecimal string
 *
 * @param[in]  in the input ascii string
 * @param[out] out the output hexadecimal string
 * @param[in]  bytesize the input ascii string size
 ******************************************************************************/
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


/*****************************************************************************
 * @brief print byte-per-byte a given string in hexadecimal format
 *
 * @param[in]  inStr string to print
 * @param[in]  strSize bytsize to print (can be even less, NOT more )
 * 
 ******************************************************************************/
void printStringHex(const string inStr, size_t strSize){
    #if DEBUG_LEVEL == TRACE_ALL
	printf("Going to print a hex string :D\n");
  #endif
	for (size_t i = 0; i < strSize; i++)
	{
		printf("%x",inStr[i]);
	}
	printf("\n");
	
}

/*****************************************************************************
 * @brief print byte-per-byte a given char buff in hexadecimal format
 *
 * @param[in]  inStr char buff to print
 * @param[in]  strSize bytsize to print (can be even less, NOT more )
 * 
 ******************************************************************************/
void printCharBuffHex(const char * inStr, size_t strSize){
    #if DEBUG_LEVEL == TRACE_ALL
	printf("Going to prit a hex char buff :D\n");
  #endif
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

//Data structure of a memory test Result
struct MemoryTestResult {
  unsigned int    target_address;
  unsigned int    fault_cntr;
  unsigned int    first_fault_address;
  unsigned int    clock_cycles_read;
  unsigned int    clock_cycles_write;

  MemoryTestResult()      {}
  MemoryTestResult(
    unsigned long long int target_address,
    unsigned int fault_cntr,
    unsigned int  first_fault_address,
    unsigned long long int clock_cycles_write,
    unsigned long long int clock_cycles_read) :
    target_address(target_address), 
    fault_cntr(fault_cntr), 
    first_fault_address(first_fault_address),  
    clock_cycles_write(clock_cycles_write),
    clock_cycles_read(clock_cycles_read) {}
};

/*****************************************************************************
 * @brief Parse the memory test output contained in astring with a given size
 * 
 * @param[in]  longbuf the buffer containing the output
 * @param[in] charOutputSize the bytesize of the buffer
 * @param[in] rawdatalines the number of lines in the given outbuf
 * @return vectpr of MemoryTestResult data strcuture
 ******************************************************************************/
template<unsigned int bytes_per_line=8>
std::vector<MemoryTestResult> parseMemoryTestOutput(const string longbuf, size_t charOutputSize, int rawdatalines)
{
  std::vector<MemoryTestResult> testResults_vector;

  int rawiterations = charOutputSize / 8; //should be equivalent to rawdatalines
  unsigned int mem_word_size = 512;
  unsigned int mem_word_byte_size = mem_word_size/8;
  bool is_stop_present = rawdatalines % (3+1+1) == 0; //guard to check if multiple data of 3 64bytes or with 

  int k = 1;
  char myTmpOutBuff [bytes_per_line];
  for (int i = 0; i < bytes_per_line; ++i)
  {
      myTmpOutBuff[i]=(char)0;
  }
  unsigned int testingNumber_out=0, fault_cntr_out=0, fault_addr_out=0;
  unsigned long long int max_memory_addr_out=0, clock_cycles_read=0, clock_cycles_write=0;
  for (int i = 1; i < rawdatalines+1; i++)
  {
    string tmp_outbuff;
    tmp_outbuff= longbuf.substr((i-1)*bytes_per_line,bytes_per_line);
    if(is_stop_present && k==7){
      cout << "DEBUG the stop is present and is here" << endl;
    } else  if( ( (i == rawdatalines-1) || (i == rawdatalines) ) && k==6){ //check it is either the last or one before the last
      //substr extraction and parsing
      //strncpy(myTmpOutBuff,tmp_outbuff.c_str()+1,bytes_per_line-1);
      //testingNumber_out = *reinterpret_cast<unsigned long long*>(myTmpOutBuff);
      memcpy(&testingNumber_out,tmp_outbuff.c_str()+1,bytes_per_line/2);

    #if DEBUG_LEVEL == TRACE_ALL
      cout << "DEBUG last command with the iterations " << testingNumber_out << endl;
    #endif
    }else if(k==5){
      //strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line);
      //clock_cycles_read = *reinterpret_cast<unsigned long long int*>(myTmpOutBuff);
      memcpy(&clock_cycles_read,tmp_outbuff.c_str(),bytes_per_line);

          #if DEBUG_LEVEL == TRACE_ALL
      cout << "DEBUG clock_cycles_read (or the fourth half data pckt) " << clock_cycles_read << endl;
      cout << "DEBUG clock_cycles_write (or the fourth half data pckt) " << clock_cycles_write << endl;
    #endif
      MemoryTestResult tmpResult(max_memory_addr_out,fault_cntr_out,fault_addr_out,clock_cycles_write,clock_cycles_read);
      testResults_vector.push_back(tmpResult);
      if(!( (i+1 == rawdatalines-1) || (i+1 == rawdatalines) )){
        k=0;
    #if DEBUG_LEVEL == TRACE_ALL
      cout << "DEBUG reinit the counter" << endl;
    #endif
      }
      unsigned int written_words = max_memory_addr_out%mem_word_byte_size == 0 ? max_memory_addr_out/mem_word_byte_size  : max_memory_addr_out/mem_word_byte_size + 1;
      double rd_bndwdth = ( (double)written_words*(double)mem_word_size / ( (double)tmpResult.clock_cycles_read * ( 6.4 ) ) ); // Gbit/T
      double wr_bndwdth = ( (double)written_words*(double)mem_word_size / ( (double)tmpResult.clock_cycles_write * ( 6.4 ) ) );
    #if DEBUG_LEVEL == TRACE_ALL
      cout << "Written " << written_words << " words" << endl;
      cout << "DEBUG overall test results: target address " << tmpResult.target_address << " ";
      cout << "Fault counter: " << tmpResult.fault_cntr << " ";
      cout << "First fault at address: " << tmpResult.first_fault_address << " "  << endl;
      cout << " RD BW " << rd_bndwdth  << "[GBit/s] with cc equal to " << tmpResult.clock_cycles_read << " "  << endl;
      cout << " WR BW " << wr_bndwdth << "[GBit/s] with cc equal to " << tmpResult.clock_cycles_write << " "  << endl;
    #endif
    } else  if(k==4){ //clock cycless
      //char mySecondTmpOutBuff[bytes_per_line/2];
      //string additional_string;
      //init the buffer
      //for(int i=0;i<bytes_per_line;i++){myTmpOutBuff[i]=(char)0;mySecondTmpOutBuff[i%(bytes_per_line/2)]=(char)0;}
      // additional_string=tmp_outbuff.substr(bytes_per_line/2,bytes_per_line/2);
      //
      // tmp_outbuff = tmp_outbuff.erase(bytes_per_line/2,bytes_per_line/2);
      // strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line/2);
      // clock_cycles_read = *reinterpret_cast<unsigned int*>(myTmpOutBuff);
      //
      // strncpy(mySecondTmpOutBuff,additional_string.c_str(),bytes_per_line/2);
      // clock_cycles_write = *reinterpret_cast<unsigned int*>(mySecondTmpOutBuff);
      //strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line);
      //clock_cycles_write = *reinterpret_cast<unsigned long long int*>(myTmpOutBuff);
      memcpy(&clock_cycles_write,tmp_outbuff.c_str(),bytes_per_line);

    }else if(k==3){ // first fault address
      //substr extraction and parsing
      //strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line);
      //fault_addr_out = *reinterpret_cast<unsigned long long int *>(myTmpOutBuff);
      memcpy(&fault_addr_out,tmp_outbuff.c_str(),bytes_per_line/2);

    #if DEBUG_LEVEL == TRACE_ALL
      cout << "DEBUG first fault address (or the third data pckt) " << fault_addr_out << endl;
    #endif
    }else if(k==2){ // fault cntr
      //strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line);
      //fault_cntr_out = *reinterpret_cast<unsigned long long int *>(myTmpOutBuff);
      memcpy(&fault_cntr_out,tmp_outbuff.c_str(),bytes_per_line/2);

    #if DEBUG_LEVEL == TRACE_ALL
      cout << "DEBUG the fault counters (or the second data pack) " <<  fault_cntr_out << endl;
    #endif
    }else { //max addrss
      //substr extraction and parsing
      // strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line);
      // max_memory_addr_out = *reinterpret_cast<unsigned long long *>(myTmpOutBuff);
      memcpy(&max_memory_addr_out,tmp_outbuff.c_str(),bytes_per_line);
      #if DEBUG_LEVEL == TRACE_ALL
      cout << "DEBUG max address (or the first data pack) " << max_memory_addr_out << endl;
    #endif
    }
    k++;
    tmp_outbuff.clear();
  }
  return testResults_vector;
}


std::ofstream loggingAVGFile;
const std::string loggingAVGFileName="cfp_vitis_memtest_avg.csv";

void createAVGLogFile(){
  if (FILE *file = fopen(loggingAVGFileName.c_str(), "r")){
    fclose(file);
  }else{
    loggingAVGFile.open(loggingAVGFileName, std::ios_base::app);
    loggingAVGFile << "TimeStamp,Iterations[#],Trgt_Address[Byte],Brst_size[#beats],WrittenWords[512b],AVG_RD_BW[Gbit/s],AVG_WR_BW[Gbit/s],AVG_faults[#]\n";
    loggingAVGFile.close();
  }
}

void logTheAvgResult(
  unsigned int iters, unsigned long long int trgt_address,
  unsigned int brst_size, unsigned int wr_words,
  double rd_bw, double wr_bw, unsigned int faults )
  {
    loggingAVGFile.open(loggingAVGFileName, std::ios_base::app);
    std::time_t instant_time = std::time(0);   // get time now
    std::tm* now = std::localtime(&instant_time);
    loggingAVGFile << (now->tm_year + 1900)<< '-' << (now->tm_mon + 1) << '-' <<  now->tm_mday << "," << iters<< ",";
    loggingAVGFile<<trgt_address<< ","<<brst_size<< ","<<wr_words<< ",";
    loggingAVGFile<<rd_bw<< ","<<wr_bw<< ","<< faults <<"\n";
    loggingAVGFile.close();
}

std::ofstream loggingMultiItFile;
const std::string loggingMultiItFileName="cfp_vitis_memtest_multi.csv";


void createItLogFile(){
  if (FILE *file = fopen(loggingMultiItFileName.c_str(), "r")){
    fclose(file);
  }else{
    loggingMultiItFile.open(loggingMultiItFileName, std::ios_base::app);
    loggingMultiItFile << "TimeStamp,Iteration[#],Trgt_Address[Byte],";
    loggingMultiItFile << "Brst_size[#beats],WrittenWords[512b],RD_BW[Gbit/s],WR_BW[Gbit/s],";
    loggingMultiItFile << "faults[#],first_faulty_address[byte]\n";
    loggingMultiItFile.close();
  }
}

void logTheSingleResult(
  unsigned int iters, unsigned long long int trgt_address,
  unsigned int brst_size, unsigned int wr_words,
  double rd_bw, double wr_bw, unsigned int faults,
  unsigned long long int first_faulty_address )
  {
    loggingMultiItFile.open(loggingMultiItFileName, std::ios_base::app);
    std::time_t instant_time = std::time(0);   // get time now
    std::tm* now = std::localtime(&instant_time);
    loggingMultiItFile << (now->tm_year + 1900)<< '-' << (now->tm_mon + 1) << '-' <<  now->tm_mday << "," << iters<< ",";
    loggingMultiItFile<<trgt_address<< ","<<brst_size<< ","<<wr_words<< ",";
    loggingMultiItFile<<rd_bw<< ","<<wr_bw<< ","<< faults;
    loggingMultiItFile<<","<<first_faulty_address<<"\n";
    loggingMultiItFile.close();
}
