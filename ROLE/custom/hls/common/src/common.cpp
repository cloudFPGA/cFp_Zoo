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
    UdpWord     udpWord;

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

    ap_uint<64>  value;

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
    string      strLine;
    ofstream    outFileStream;
    string      datFile = "../../../../test/" + outFileName;
    UdpWord     udpWord;
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
 * @brief Fill an output file with data from an input data string and add start and stop commands.
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpStringToFileWithCommands(string s, const string   outFileName, int simCnt)
{
    string      strLine;
    ofstream    outFileStream;
    string      datFile = "../../../../test/" + outFileName;
    UdpWord     udpWord;
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

// Preparing the start command encoded as 1 bit set and the remaining unset
    for (unsigned int k = 0; k < bytes_per_line; k++)
    {
        if(k==0){
            value[k] = 1;
        } else{
            value[k] = 0;
        }
    }
    udpWord.tdata = pack_ap_uint_64_(value);
    udpWord.tkeep = 255;
    udpWord.tlast = 0;
    if (!dumpDataToFile(&udpWord, outFileStream))
    {
      rc = KO;
    }

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
	    // if ((total_bytes >= (s.length() - bytes_per_line)) || 
	    //     ((total_bytes + bytes_per_line) % PACK_SIZE == 0)) {
	    //   udpWord.tlast = 1;
	    // }
	    // else {
	       udpWord.tlast = 0;
	    // }
            printf("[%4.4d] IMG TB is dumping string to file [%s] - Data read [%u] = {val=%u, D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                    simCnt, datFile.c_str(), total_bytes, value,
                    udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int());
            if (!dumpDataToFile(&udpWord, outFileStream)) {
	      rc = KO;
              break;
            }
	}

// Preparing the start command encoded as only unset bits
    for (unsigned int k = 0; k < bytes_per_line; k++)
    {
        value[k] = 0;
    }
    udpWord.tdata = pack_ap_uint_64_(value);
    udpWord.tkeep = 255;
    udpWord.tlast = 1;
    if (!dumpDataToFile(&udpWord, outFileStream))
    {
      rc = KO;
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
bool dumpStringToFile(string s, const string   outFileName, int simCnt)
{
    string      strLine;
    ofstream    outFileStream;
    string      datFile = "../../../../test/" + outFileName;
    UdpWord     udpWord;
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
    UdpWord     udpWord;
    unsigned int i = 0;
    unsigned int bytes_per_line = 8;
    ap_uint<8> value[bytes_per_line];
    
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

    return(OK);
}


/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] strOutput the output string to set.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool dumpFileToStringWithoutCommands(const string inpFileName, char* charOutput, int simCnt) {
    string      strLine;
    ifstream    inpFileStream;
    string      datFile = "../../../../test/" + inpFileName;
    UdpWord     udpWord;
    unsigned int i = 0;
    unsigned int bytes_per_line = 8;
    ap_uint<8> value[bytes_per_line];
    
    //-- STEP-1 : OPEN FILE
    inpFileStream.open(datFile.c_str());
    if ( !inpFileStream ) {
        cout << "### ERROR : Could not open the input data file " << datFile << endl;
        return(KO);
    }
    // ignore the ack of the start command
    getline(inpFileStream, strLine);
    //-- STEP-2 : SET DATA STREAM
    while (inpFileStream) {

        if (!inpFileStream.eof()) {
            if (strLine.empty()) continue;

            getline(inpFileStream, strLine);
    // ignore the ack of the stop command at the eof
            if (!inpFileStream.eof()){
                if (strLine.empty()) continue;
            sscanf(strLine.c_str(), "%llx %x %d", &udpWord.tdata, &udpWord.tkeep, &udpWord.tlast);
            // Write to strOutput
            //printf("Debug: (char)udpWord.tdata.to_long()=%c\n", (char)udpWord.tdata.to_long());
            unpack_ap_uint_64_(udpWord.tdata, value);
            for (unsigned int k = 0; k < bytes_per_line; k++) {
            charOutput[i++] = value[k];
            }

        // Print Data to console
        //printf("[%4.4d] TB is filling string with character %c\n",
                //   simCnt, (char)value[k]);
                }
        }
    }

    //-- STEP-3: CLOSE FILE
    inpFileStream.close();

    return(OK);
}



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


void ascii2hex(const string& in, string& out)
{
 std::stringstream sstream;
    for ( string::const_iterator item = in.begin(); item != in.end(); item++){
        sstream << std::hex << int(*item);
    }
    out=sstream.str(); 
}



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


//void createMemTestCommands(const string &in, string& out)
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
	char value[bytes_per_line];
    memcpy(value, (char*)&mem_address, sizeof(unsigned int));
    out.append(value,bytes_per_line/2);
    for (int i = 0; i < (testingNumber * (2 * (mem_address+1)) + 2); i++){
	    out.append(filler_cmd,bytes_per_line);
    }
	out.append(stop_cmd,bytes_per_line);
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
