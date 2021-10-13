/*****************************************************************************
 * @file       commom.cpp
 * @brief      Common functions for testbenches - body.
 *
 * @date       September 2021
 * @author     FAB, WEI, NGL, DID, DCO
 * 
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2020 - IBM Research - All Rights Reserved.
 *
 * @ingroup CustomIBMZRL 
 * @addtogroup CustomIBMZRL 
 * \{
 *****************************************************************************/


#include "../include/common.hpp"

using namespace std;




/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 *
 * @param[in] sDataStream the input data stream to set.
 * @param[in] dataStreamName the name of the data stream.
 * @param[in] inpFileName the name of the input file to read from.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool setInputDataStream(stream<UdpWord> &sDataStream, const string dataStreamName, 
			const string inpFileName, int simCnt) {
    string      strLine;
    ifstream    inpFileStream;
    string      datFile = "../../../../test/" + inpFileName;
    UdpWord     udpWord=NetworkWord(0,0,0);

    //-- STEP-1 : OPEN FILE
    inpFileStream.open(datFile.c_str());
    if ( !inpFileStream ) {
        cout << "### ERROR : Could not open the input data file " << datFile << endl;
        return(KO);
    }

    //-- STEP-2 : SET DATA STREAM
    while (inpFileStream) {

        if (!inpFileStream.eof()) {

            getline(inpFileStream, strLine);
            if (strLine.empty()) continue;
            sscanf(strLine.c_str(), "%llx %x %d", &udpWord.tdata, &udpWord.tkeep, &udpWord.tlast);

            // Write to sDataStream
            if (sDataStream.full()) {
                printf("### ERROR : Stream is full. Cannot write stream with data from file \"%s\".\n", inpFileName.c_str());
                return(KO);
            } else {
                sDataStream.write(udpWord);
                // Print Data to console
                printf("[%4.4d] TB is filling input stream [%s] - Data write = {D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                        simCnt, dataStreamName.c_str(),
                        udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int());
            }
        }
    }

    //-- STEP-3: CLOSE FILE
    inpFileStream.close();
    //strLine.clear();

    return(OK);
}




/*****************************************************************************
 * @brief Read data from a stream.
 *
 * @param[in]  sDataStream,    the output data stream to read.
 * @param[in]  dataStreamName, the name of the data stream.
 * @param[out] udpWord,        a pointer to the storage location of the data
 *                              to read.
 * @return VALID if a data was read, otherwise UNVALID.
 ******************************************************************************/
bool readDataStream(stream <UdpWord> &sDataStream, UdpWord *udpWord) {
    // Get the DUT/Data results
    sDataStream.read(*udpWord);
    return(VALID);
}


/*****************************************************************************
 * @brief Pack an array of 8 x ap_uint<8> into a ap_uint<64> word.
 *
 * @param[in]  buffer     A pointer to an array of 8 x ap_uint<8>
 * @return An ap_uint<64> word.
 ******************************************************************************/
ap_uint<64> pack_ap_uint_64_ (ap_uint<8> *buffer) {

    ap_uint<64>  value = 0;

    value = buffer[7];
    value = (value << 8 ) + buffer[6];
    value = (value << 8 ) + buffer[5];
    value = (value << 8 ) + buffer[4];
    value = (value << 8 ) + buffer[3];
    value = (value << 8 ) + buffer[2];
    value = (value << 8 ) + buffer[1];
    value = (value << 8 ) + buffer[0];

    return value ;

}

/*****************************************************************************
 * @brief Unpack an ap_uint<64> word to an array of 8 x ap_uint<8>.
 *
 * @param[in]  buffer     A pointer to an ap_uint<64> word
 * @return An ap_uint<64> word.
 ******************************************************************************/
void unpack_ap_uint_64_ (ap_uint<64> value, ap_uint<8> *buffer) {

  for (unsigned int i=0; i<8; i++) {
    buffer[i] = (value >> 8*i );
  }
}

/*****************************************************************************
 * @brief Dump a data word to a file.
 *
 * @param[in] udpWord,      a pointer to the data word to dump.
 * @param[in] outFileStream,the output file stream to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpDataToFile(UdpWord *udpWord, ofstream &outFileStream) {
    if (!outFileStream.is_open()) {
        printf("### ERROR : Output file stream is not open. \n");
        return(KO);
    }
    outFileStream << hex << noshowbase << setfill('0') << setw(16) << udpWord->tdata.to_uint64();
    outFileStream << " ";
    outFileStream << hex << noshowbase << setfill('0') << setw(2)  << udpWord->tkeep.to_int();
    outFileStream << " ";
    outFileStream << setw(1) << udpWord->tlast.to_int() << "\n";
    return(OK);
}



/*****************************************************************************
 * @brief Fill an output file with data from an output stream.
 *
 * @param[in] sDataStream,    the output data stream to set.
 * @param[in] dataStreamName, the name of the data stream.
 * @param[in] outFileName,    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool getOutputDataStream(stream<UdpWord> &sDataStream,
                         const string    dataStreamName, const string outFileName, int simCnt)
{
    //string      strLine;
    ofstream    outFileStream;
    string      datFile = "../../../../test/" + outFileName;
    UdpWord     udpWord=NetworkWord(0,0,0);
    bool        rc = OK;

    //-- STEP-1 : OPEN FILE
    outFileStream.open(datFile.c_str());
    if ( !outFileStream ) {
        cout << "### ERROR : Could not open the output data file " << datFile << endl;
        return(KO);
    }

    //-- STEP-2 : EMPTY STREAM AND DUMP DATA TO FILE
    while (!sDataStream.empty()) {
        if (readDataStream(sDataStream, &udpWord) == VALID) {
            // Print DUT/Data to console
            printf("[%4.4d] TB is draining output stream [%s] - Data read = {D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                    simCnt, dataStreamName.c_str(),
                    udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int());
            if (!dumpDataToFile(&udpWord, outFileStream)) {
                rc = KO;
                break;
            }
        }
    }

    //-- STEP-3: CLOSE FILE
    outFileStream.close();

    return(rc);
}

/*****************************************************************************
 * @brief Fill an output file with data from an image.
 * 
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpStringToFile(const string s, const string   outFileName, int simCnt)
{
    //string      strLine;
    ofstream    outFileStream;
    string      datFile = "../../../../test/" + outFileName;
    UdpWord     udpWord=NetworkWord(0,0,0);
    bool        rc = OK;
    unsigned int bytes_per_line = 8;
    
    //-- STEP-1 : OPEN FILE
    outFileStream.open(datFile.c_str());
    if ( !outFileStream ) {
        cout << "### ERROR : Could not open the output data file " << datFile << endl;
        return(KO);
    }
    printf("came to dumpStringToFile: s.length()=%u\n", s.length());
    
    ap_uint<8> value[bytes_per_line];
    unsigned int total_bytes = 0;

    //-- STEP-2 : DUMP STRING DATA TO FILE
    for (unsigned int i = 0; i < s.length(); i+=bytes_per_line, total_bytes+=bytes_per_line) {
      //if (NPIX == XF_NPPC8) {
        for (unsigned int k = 0; k < bytes_per_line; k++) {
          if (i+k < s.length()) {
        value[k] = s[i+k];
          }
          else {
        value[k] = 0;
          }
          printf("DEBUG: In dumpStringToFile: value[%u]=%c\n", k, (char)value[k]);
        }
        udpWord.tdata = pack_ap_uint_64_(value);
        udpWord.tkeep = 255;
        // We are signaling a packet termination either at the end of the image or the end of MTU
        if ((total_bytes >= (s.length() - bytes_per_line)) || 
            ((total_bytes + bytes_per_line) % PACK_SIZE == 0)) {
          udpWord.tlast = 1;
        }
        else {
          udpWord.tlast = 0;
        }
            printf("[%4.4d] IMG TB is dumping string to file [%s] - Data read [%u] = {val=%u, D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                    simCnt, datFile.c_str(), total_bytes, value,
                    udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int());
            if (!dumpDataToFile(&udpWord, outFileStream)) {
          rc = KO;
              break;
            }
    }
    //-- STEP-3: CLOSE FILE
    outFileStream.close();

    return(rc);
}

/*****************************************************************************
 * @brief Fill an output file with data from an image.
 * 
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpStringToFileOnlyRawData(const string s, const string   outFileName, int simCnt, size_t out_size)
{
    //printStringHex(s, out_size);

    //string      strLine;
    ofstream    outFileStream;
    string      datFile = outFileName; //"../../../../test/" + outFileName;
    bool        rc = OK;
    unsigned int bytes_per_line = 8;
    ap_uint<64> tdata = 0;
    
    //-- STEP-1 : OPEN FILE
    outFileStream.open(datFile.c_str());
    if ( !outFileStream ) {
        cout << "### ERROR : Could not open the output data file " << datFile << endl;
        return(KO);
    }
    printf("came to dumpStringToFileOnlyRawData: s.length()=%u\n", out_size);
    
    ap_uint<8> value[bytes_per_line];
    unsigned int total_bytes = 0;
    //-- STEP-2 : DUMP STRING DATA TO FILE
    for (unsigned int i = 0; i < out_size; i+=bytes_per_line, total_bytes+=bytes_per_line) {
        for (unsigned int k = 0; k < bytes_per_line; k++) {
          if (i+k < out_size) {
        value[k] = s[i+k];
          }
          else {
        value[k] = 0;
          }
          printf("DEBUG: In dumpStringToFileOnlyRawData: value[%u]=%c\n", k, (char)value[k]);
        }
        tdata = pack_ap_uint_64_(value);
            printf("[%4.4d] IMG TB is dumping string to file [%s] - Data read [%u] = {val=%u, D=0x%16.16llX} \n",
                    simCnt, datFile.c_str(), total_bytes, value, tdata.to_long());
        outFileStream << hex << setfill('0') << setw(16) << tdata.to_uint64();
        //outFileStream << hex << noshowbase << setfill('0') << setw(16) << tdata.to_uint64();
    outFileStream << "\n";
    }
    //-- STEP-3: CLOSE FILE
    outFileStream.close();

    return(rc);
}

/*****************************************************************************
 * @brief Fill an output file with data from a string and
 *          set the tlast every gno packets
 * 
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @param[in] simCnt         
 * @param[in] gno            the counter value at which this function set the tlast=1
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpStringToFileWithLastSetEveryGnoPackets(string s, const string   outFileName, int simCnt, int gno)
{
    string      strLine;
    ofstream    outFileStream;
    string      datFile = "../../../../test/" + outFileName;
    UdpWord     udpWord=NetworkWord(0,0,0);
    bool        rc = OK;
    unsigned int bytes_per_line = 8;
    
    //-- STEP-1 : OPEN FILE
    outFileStream.open(datFile.c_str());
    if ( !outFileStream ) {
        cout << "### ERROR : Could not open the output data file " << datFile << endl;
        return(KO);
    }
    printf("came to dumpStringToFile: s.length()=%u\n", s.length());
    
    ap_uint<8> value[bytes_per_line];
    unsigned int total_bytes = 0;
    int cntr = 0;

    //-- STEP-2 : DUMP STRING DATA TO FILE
    for (unsigned int i = 0; i < s.length(); cntr += 1, i+=bytes_per_line, total_bytes+=bytes_per_line) {
      //if (NPIX == XF_NPPC8) {
        for (unsigned int k = 0; k < bytes_per_line; k++) {
          if (i+k < s.length()) {
        value[k] = s[i+k];
          }
          else {
        value[k] = 0;
          }
          printf("DEBUG: In dumpStringToFile: value[%u]=%c\n", k, (char)value[k]);
        }
        udpWord.tdata = pack_ap_uint_64_(value);
        udpWord.tkeep = 255;
        // We are signaling a packet termination either at the end of the image or the end of MTU
        if ((total_bytes >= (s.length() - bytes_per_line)) || 
            ((total_bytes + bytes_per_line) % PACK_SIZE == 0) || ( cntr!= 0 && ((cntr+1) % gno == 0))) {
          udpWord.tlast = 1;
        }
        else {
          udpWord.tlast = 0;
        }
            printf("[%4.4d] IMG TB is dumping string to file [%s] - Data read [%u] = {val=%u, D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                    simCnt, datFile.c_str(), total_bytes, value,
                    udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int());
            if (!dumpDataToFile(&udpWord, outFileStream)) {
          rc = KO;
              break;
            }
    }
    //-- STEP-3: CLOSE FILE
    outFileStream.close();

    return(rc);
}

/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] strOutput the output string to set.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool dumpFileToString(const string inpFileName, char* charOutput, int simCnt) {
    string      strLine;
    ifstream    inpFileStream;
    string      datFile = "../../../../test/" + inpFileName;
    UdpWord     udpWord=NetworkWord(0,0,0);
    unsigned int i = 0;
    unsigned int bytes_per_line = 8;
    ap_uint<8> value[bytes_per_line];
    
    for(i=0; i < bytes_per_line; i++){
      value[i]=0;
    }
    i=0;
    //-- STEP-1 : OPEN FILE
    inpFileStream.open(datFile.c_str());
    if ( !inpFileStream ) {
        cout << "### ERROR : Could not open the input data file " << datFile << endl;
        return(KO);
    }

    //-- STEP-2 : SET DATA STREAM
    while (inpFileStream) {

        if (!inpFileStream.eof()) {

            getline(inpFileStream, strLine);
            //cout << strLine << endl;
            if (strLine.empty()) continue;
            sscanf(strLine.c_str(), "%llx %x %d", &udpWord.tdata, &udpWord.tkeep, &udpWord.tlast);
            // Write to strOutput
	    //printf("Debug: (char)udpWord.tdata.to_long()=%c\n", (char)udpWord.tdata.to_long());
	    unpack_ap_uint_64_(udpWord.tdata, value);
	    for (unsigned int k = 0; k < bytes_per_line; k++) {
		charOutput[i++] = value[k];
		// Print Data to console
		//printf("[%4.4d] TB is filling string with character %c\n",
                //   simCnt, (char)value[k]);
	    }
        }
    }

    //-- STEP-3: CLOSE FILE
    inpFileStream.close();
    //strLine.clear();

    return(OK);
}

/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] strOutput the output string to set.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
template<unsigned int bytes_per_line = 8>
bool dumpFileToStringRawData(const string inpFileName, char* charOutput, int * rawdatalines) {
    string      strLine;
    ifstream    inpFileStream;
    string      datFile = inpFileName;
    unsigned long long int  mylongunsigned;
    unsigned int i = 0;
    char my_tmp_buf [bytes_per_line];
    //-- STEP-1 : OPEN FILE
    inpFileStream.open(datFile.c_str());
    if ( !inpFileStream ) {
        cout << "### ERROR : Could not open the input data file " << datFile << endl;
        return(KO);
    }

    //-- STEP-2 : SET DATA STREAM
    while (inpFileStream) {
        if (!inpFileStream.eof()) {
          getline(inpFileStream, strLine);
          //cout << strLine << endl;
          if (strLine.empty()) continue;
          *rawdatalines+=1;
          sscanf(strLine.c_str(), "%llx", &mylongunsigned);
          // Write to strOutput
          printf("my long long %llx\n", mylongunsigned);
          memcpy(my_tmp_buf,&mylongunsigned, bytes_per_line);
          memcpy(charOutput+(i*bytes_per_line),&my_tmp_buf, bytes_per_line);
          printCharBuffHex(charOutput+(i*bytes_per_line), bytes_per_line);
          i++;
        }
    }

    //-- STEP-3: CLOSE FILE
    inpFileStream.close();

    return(OK);
}

// Assumes little endian
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
//cout<<endl<<endl;

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
     // printf("my long long %llx\n", mylongunsigned);
      //printf("my long long non hex %llu\n", mylongunsigned);
      memcpy(my_tmp_buf,(char *)&mylongunsigned, sizeof(unsigned long long int));
      //printBits(sizeof(unsigned long long int), my_tmp_buf);
      //cout << endl;
      // printBits(sizeof(unsigned long long int), tmp_Out.c_str());
      charOutput.append(my_tmp_buf, bytes_per_line);
      i++;
        }
       // strLine.clear();
//cout<<endl<<endl;
    }
    //-- STEP-3: CLOSE FILE
    inpFileStream.close();
    //tmp_Out.clear();

    return string(charOutput);
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

void hex2asciiByteByByte(const string& in, string& out)
{
    out.clear();
    out.reserve(in.length() / 2);
    for (string::const_iterator p = in.begin(); p != in.end(); p++)
    {
       unsigned char c = hexval(*p);
       //p++;
       //if (p == in.end()) break; // incomplete last digit - should report error
       //c = (c << 4) + hexval(*p); // + takes precedence over <<
       out.push_back(c);
    }
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

template<typename T>
T ascii2hexTheRevenge(const string& in)
{
  T out;
 std::stringstream sstream;
 sstream << std::hex << in;
sstream >> out;
  return out;
}

void ascii2hexWithSize(const string& in, string& out, size_t  bytesize)
{
 std::stringstream sstream;
    for ( int i=0; i<bytesize; i++){
        sstream << std::hex << int(in[i]);
    }
    out=sstream.str(); 
}

void ascii2hexCharWithSize(const string& in, string& out, size_t  bytesize)
{
 std::stringstream sstream;
    for ( int i=0; i<bytesize; i++){
        sstream << std::hex << int8_t(in[i]);
    }
    out=sstream.str(); 
}

void hex2Unsigned(const string& in, string& out, size_t  bytesize){
 std::stringstream sstream;
    
    for ( int i=0; i<bytesize; i++){
        sstream << std::hex << uint8_t(in[i]);
    }
    out=sstream.str(); 
}


/*****************************************************************************
 * @brief Check the presence of a given corner value at the begin and the end of a string
 *
 * @param[in]  str the input string to be checked
 * @param[in]  corner the corner char to find
 * @return true if it is present, false otherwise
 ******************************************************************************/
bool isCornerPresent(string str, string corner)
{
    int n = str.length();
    int cl = corner.length();
 
    // If length of corner string is more, it
    // cannot be present at corners.
    if (n < cl)
       return false;
 
    // Return true if corner string is present at
    // both corners of given string.
    return (str.substr(0, cl).compare(corner) == 0 &&
            str.substr(n-cl, cl).compare(corner) == 0);
}

/*****************************************************************************
 * @brief Convert a hex string to a integer into a char buffer with the SAME dimensions
 *
 * @param[in]  in the input hex string
 * @param[out] out the output numerical hexadec string string
 * @param[in]  byteSize the bytesize of the input string and the buffer, it assumes equal dimension
 ******************************************************************************/
void string2hexnumerics(const string& in, char * out, size_t byteSize)
{
	for (unsigned int i = 0; i < byteSize; i++)
	{
		std::sprintf(out+i, "%d", (int)in[i]);
	}
}

void string2hexUnsignedNumerics(const string& in, char * out, size_t byteSize)
{
	for (unsigned int i = 0; i < byteSize; i++)
	{
		std::sprintf(out+i, "%u", (unsigned int)in[i]);
	}
}

void stringHex2Unsigned(const string& in, unsigned int * out, size_t byteSize)
{
	for (unsigned int i = 0; i < byteSize; i++)
	{
		std::sprintf((char*)out+i, "%u", (unsigned int)in[i]);
	}
}
/*****************************************************************************
 * @brief Convert a hex string to a integer into a char buffer with the SAME dimensions
 *
 * @param[in]  in the input hex string
 * @param[out] out the output numerical hexadec string string
 * @param[in]  byteSize the bytesize of the input string and the buffer, it assumes equal dimension
 ******************************************************************************/
void string2hexnumericsString(const string& in, string &out, size_t byteSize)
{
  char tmp_out [byteSize];
	for (unsigned int i = 0; i < byteSize; i++)
	{
		std::sprintf(tmp_out+i, "%d", (int)in[i]);
	}
  out.append(tmp_out);
}

/*****************************************************************************
 * @brief Attach the commands start (000000001) and stop (000000002) 
 * to a given input string, aligning to a bytesize of 8 byte per tdata values
 *
 * @param[in]  in the input string
 * @param[out] out the output string with start-in(8byte aligned)-stop
 ******************************************************************************/
void attachBitformattedStringCommandAndRefill(const string& in, string& out)
{
	unsigned int bytes_per_line = 8;
	char start_cmd [bytes_per_line];
	char stop_cmd [bytes_per_line];
    //WARNING: currently hardcoded way of start and stop commands with a 1 and 2 for start and stop respectively
	for (unsigned int k = 0; k < bytes_per_line; k++) {
		if (k != 0) {
			start_cmd[k] = (char)0;
			stop_cmd[k] = (char)0;
	      	}
	      	else {
			start_cmd[k] = (char)1; 
			stop_cmd[k] = (char)2;
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

/*****************************************************************************
 * @brief Create the commands for a memory test with start/max address to test-nop to execute-stop
 *
 * @param[in]  mem_address max target address to test
 * @param[out] out the output string with start/max address-nops4trgtCCsNeeded-stop
 * @param[in]  testingNumber the number of tests to perform on the memory
 * 
 ******************************************************************************/
template<unsigned int bytes_per_line = 8>
string createMemTestCommands(unsigned int mem_address, unsigned int testingNumber)
{
  string out;
  //out.reserve(bytes_per_line);
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
	out = out.append(start_cmd,3);
  memcpy(value_cmd, (char*)&mem_address, 4);
  out = out.append(value_cmd,5);
  return string(out);
  }

/*****************************************************************************
 * @brief Create the expected output results for the memory test (with FAULT INJECTION)
 *
 * @param[in]  mem_address max target address to test
 * @param[out] out the results of the memory test (with FAULT INJECTION)
 * @param[in]  testingNumber the number of tests to perform on the memory
 * 
 ******************************************************************************/
template<unsigned int bytes_per_line = 8>
string createMemTestGoldenOutput(unsigned int mem_address, unsigned int testingNumber, bool with_bw_analysis)
{
	char addr_cmd [bytes_per_line]; // Half of the command filled with start other half with the address
	char fault_cntr_cmd [bytes_per_line];
	char fault_addr_cmd [bytes_per_line];
	char filler_cmd [bytes_per_line];
	char clock_cycles_cmd [bytes_per_line];
	char end_of_tests_cmd [bytes_per_line];
	char stop_cmd [bytes_per_line];
  unsigned int fault_addr = 0;
  unsigned int clock_cycles = 0;
  const unsigned int first_faultTests = 3;
  string out;

//given parameters
    unsigned int mem_word_size = 512;
    unsigned int mem_size_of_word_size = 20;
// computations the first faulty address and the the fault counter
    unsigned int mem_addr_per_word = mem_word_size / 8; // byte size of this word
    unsigned int fault_cntr = 0;
//precomputing the cc
 clock_cycles = mem_address % mem_addr_per_word == 0 ? mem_address / mem_addr_per_word : (unsigned int) mem_address / mem_addr_per_word + 1;


//simulating the fault injection
    for (unsigned int j = 0; j < mem_address; j+=mem_addr_per_word)
    {
        ap_uint<32> currentNumber = j;
        ap_uint<32> nextNumber = (currentNumber+1) xor 1;
        ap_uint<32> prevNumber = currentNumber;
        ap_uint<32> tmpNumber = nextNumber;
        ap_uint<32> mask = 0;

        for (unsigned int i = 0; i < mem_word_size/32 && j > 0; i++){
            tmpNumber = nextNumber & 0;

        if( nextNumber != (tmpNumber)){
            fault_cntr+=1;
        }
        prevNumber = currentNumber;
        currentNumber = nextNumber;
        nextNumber = (nextNumber + 1 ) xor i;
        }
    }

//init the commands and fill em out of the fault simulation before
    for(unsigned int i = 0; i < testingNumber; i++){
        for (unsigned int k = 0; k < bytes_per_line; k++) {
            addr_cmd[k]          = (char)0;
            filler_cmd[k]        = (char)0;
            fault_cntr_cmd[k]    = (char)0;
            fault_addr_cmd[k]    = (char)0;
            stop_cmd[k]          = (char)0;
            stop_cmd[k]          = (char)0;
            end_of_tests_cmd[k]  = (char)0;
            clock_cycles_cmd[k]  = (char)0;   
        }
        stop_cmd[0]    = (char)2;
        end_of_tests_cmd[0]    = (char)3;
        memcpy(end_of_tests_cmd+1, (char*)&testingNumber, 2);
        memcpy(addr_cmd, (char*)&mem_address, sizeof(unsigned int));
        out = out.append(addr_cmd,bytes_per_line);
        //if not yet in the fault injection point just let em empty as expected from good tests
        if(i < first_faultTests-1 || mem_address <= mem_addr_per_word)
        {
          out = out.append(filler_cmd,bytes_per_line);
          out = out.append(filler_cmd,bytes_per_line);
        }else{
            memcpy(fault_cntr_cmd, (char*)&fault_cntr, sizeof(unsigned int));
            out = out.append(fault_cntr_cmd,bytes_per_line);
            memcpy(fault_addr_cmd, (char*)&mem_addr_per_word, sizeof(unsigned int));
            out = out.append(fault_addr_cmd,bytes_per_line);
        }
        memcpy(clock_cycles_cmd, (char*)&clock_cycles, sizeof(unsigned int));
        memcpy(clock_cycles_cmd+sizeof(unsigned int), (char*)&clock_cycles, sizeof(unsigned int));
        out = out.append(clock_cycles_cmd,bytes_per_line);
    }
    out = out.append(end_of_tests_cmd,bytes_per_line);
  //  out.append(stop_cmd,bytes_per_line);
  return string(out);
}


/*****************************************************************************
 * @brief print byte-per-byte a given string in hexadecimal format
 *
 * @param[in]  inStr string to print
 * @param[in]  strSize bytsize to print (can be even less, NOT more )
 * 
 ******************************************************************************/
void printStringHex(const string inStr, size_t strSize){
	printf("Going to prit a hex string :D\n");
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
	printf("Going to prit a hex char buff :D\n");
	for (size_t i = 0; i < strSize; i++)
	{
		printf("%x",inStr[i]);
	}
	printf("\n");
	
}

// Function to reverse a string
void reverseStr(string& str)
{
    int n = str.length();
 
    // Swap character starting from two
    // corners
    for (int i = 0; i < n / 2; i++)
        swap(str[i], str[n - i - 1]);
}

struct MemoryTestResult {
  unsigned int    target_address;
  unsigned int    fault_cntr;
  unsigned int    first_fault_address;
  unsigned int    clock_cycles_read;
  unsigned int    clock_cycles_write;

  MemoryTestResult()      {}
  MemoryTestResult(
    unsigned int target_address,
    unsigned int fault_cntr,
    unsigned int  first_fault_address,
    unsigned int clock_cycles_write,
    unsigned int clock_cycles_read) :
    target_address(target_address), 
    fault_cntr(fault_cntr), 
    first_fault_address(first_fault_address),  
    clock_cycles_write(clock_cycles_write),
    clock_cycles_read(clock_cycles_read) {}
};

template<unsigned int bytes_per_line=8>
std::vector<MemoryTestResult> parseMemoryTestOutput(const string longbuf, size_t charOutputSize, int rawdatalines)
{
  std::vector<MemoryTestResult> testResults_vector;

  int rawiterations = charOutputSize / 8;
  unsigned int mem_word_size = 512;
  unsigned int mem_word_byte_size = mem_word_size/8;
  cout << "my calculations " << rawiterations << " the function iterations " << rawdatalines << endl;
  bool is_stop_present = rawdatalines % (3+1+1) == 0; //guard to check if multiple data of 3 64bytes or with 

  int k = 1;
  char myTmpOutBuff [bytes_per_line+1];
  unsigned int testingNumber_out=0, max_memory_addr_out=0, fault_cntr_out=0, fault_addr_out=0, clock_cycles_read=0, clock_cycles_write=0;
  for (int i = 1; i < rawdatalines+1; i++)
  {
    string tmp_outbuff;
    tmp_outbuff= longbuf.substr((i-1)*bytes_per_line,bytes_per_line);
    if(is_stop_present && k==6){
      cout << "DEBUG the stop is present and is here" << endl;
    } else  if( ( (i == rawdatalines-1) || (i == rawdatalines) ) && k==5){ //check it is either the last or one before the last
      //substr extraction and parsing
      //tmp_outbuff.erase(tmp_outbuff.begin());
      strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line-1);
      testingNumber_out = *reinterpret_cast<unsigned long long*>(myTmpOutBuff);
      //cout << "DEBUG last command with the iterations " << testingNumber_out << endl;

    } else  if(k==4){ //clock cycless
      char mySecondTmpOutBuff[bytes_per_line/2];
      string additional_string;
      for(int i=0;i<bytes_per_line;i++){myTmpOutBuff[i]=(char)0;mySecondTmpOutBuff[i%(bytes_per_line/2)]=(char)0;}
      //printBits(bytes_per_line, tmp_outbuff.c_str());
      additional_string=tmp_outbuff.substr(bytes_per_line/2,bytes_per_line/2);
      //printBits(bytes_per_line/2, additional_string.c_str());

      tmp_outbuff = tmp_outbuff.erase(bytes_per_line/2,bytes_per_line/2);
      strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line/2);
      clock_cycles_read = *reinterpret_cast<unsigned int*>(myTmpOutBuff);
      cout << "DEBUG clock_cycles_read (or the fourth half data pckt) " << clock_cycles_read << endl;

      strncpy(mySecondTmpOutBuff,additional_string.c_str(),bytes_per_line/2);
      clock_cycles_write = *reinterpret_cast<unsigned int*>(mySecondTmpOutBuff);

      cout << "DEBUG clock_cycles_write (or the fourth half data pckt) " << clock_cycles_write << endl;

      MemoryTestResult tmpResult(max_memory_addr_out,fault_cntr_out,fault_addr_out,clock_cycles_read,clock_cycles_write);
      testResults_vector.push_back(tmpResult);
      if(!( (i+1 == rawdatalines-1) || (i+1 == rawdatalines) )){
        k=0;
      //cout << "DEBUG reinit the counter" << endl;
      }
      unsigned int written_words = max_memory_addr_out%mem_word_byte_size == 0 ? max_memory_addr_out/mem_word_byte_size  : max_memory_addr_out/mem_word_byte_size + 1;
      cout << "Written " << written_words << " words" << endl;
      double rd_bndwdth = ( (double)written_words*(double)mem_word_size / ( (double)tmpResult.clock_cycles_read * ( 1.0 / 200.0 ) ) ) / 1000.0; // Gbit/T
      double wr_bndwdth = ( (double)written_words*(double)mem_word_size / ( (double)tmpResult.clock_cycles_write * ( 1.0 / 200.0 ) ) ) / 1000.0;

      cout << "DEBUG overall test results: target address " << tmpResult.target_address << " ";
      cout << "Fault counter: " << tmpResult.fault_cntr << " ";
      cout << "First fault at address: " << tmpResult.first_fault_address << " "  << endl;
      cout << " RD BW " << rd_bndwdth  << "[GBit/s] with cc equal to " << tmpResult.clock_cycles_read << " "  << endl;
      cout << " WR BW " << wr_bndwdth << "[GBit/s] with cc equal to " << tmpResult.clock_cycles_write << " "  << endl;
    }else if(k==3){ // first fault address
      //substr extraction and parsing
      strncpy(myTmpOutBuff,tmp_outbuff.c_str(),bytes_per_line);
      fault_addr_out = *reinterpret_cast<unsigned long long*>(myTmpOutBuff);
      cout << "DEBUG first fault address (or the third data pckt) " << fault_addr_out << endl;
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
    tmp_outbuff.clear();
  }
  //tmp_outbuff.clear();
  return testResults_vector;
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

static inline ssize_t
__file_write(const char *fname, const char *buff, size_t len)
{
	int rc;
	FILE *fp;

	if ((fname == NULL) || (buff == NULL) || (len == 0))
		return -EINVAL;

	fp = fopen(fname, "w+");
	if (!fp) {
		fprintf(stderr, "err: Cannot open file %s: %s\n",
			fname, strerror(errno));
		return -ENODEV;
	}
	rc = fwrite(buff, len, 1, fp);
	if (rc == -1) {
		fprintf(stderr, "err: Cannot write to %s: %s\n",
			fname, strerror(errno));
		fclose(fp);
		return -EIO;
	}
	fclose(fp);
	return rc;
}

/*! \} */
