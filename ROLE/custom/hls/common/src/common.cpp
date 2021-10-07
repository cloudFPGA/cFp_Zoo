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
    string      strLine;
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
 * @brief Fill an output file with data from an input data string and add start and stop commands.
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 * DEPRECATED
 ******************************************************************************/
bool dumpStringToFileWithCommands(string s, const string   outFileName, int simCnt)
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
bool dumpStringToFileOnlyRawData(string s, const string   outFileName, int simCnt, size_t out_size)
{
    printStringHex(s, out_size);

    string      strLine;
    ofstream    outFileStream;
    string      datFile = outFileName; //"../../../../test/" + outFileName;
    bool        rc = OK;
    unsigned int bytes_per_line = 8;
    
    //-- STEP-1 : OPEN FILE
    outFileStream.open(datFile.c_str());
    if ( !outFileStream ) {
        cout << "### ERROR : Could not open the output data file " << datFile << endl;
        return(KO);
    }
    printf("came to dumpStringToFile: s.length()=%u\n", out_size);
    
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
          printf("DEBUG: In dumpStringToFile: value[%u]=%c\n", k, (char)value[k]);
        }
        ap_uint<64> tdata = pack_ap_uint_64_(value);

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
bool dumpStringToFileWithLastInTheLastTwo64Bytes(string s, const string   outFileName, int simCnt)
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
            ((total_bytes + bytes_per_line) % PACK_SIZE == 0) || ( i == s.length() - 2*bytes_per_line ) ) {
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

    return(OK);
}

/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] strOutput the output string to set.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool dumpFileToStringRawData(const string inpFileName, char* charOutput, int * rawdatalines) {
    string      strLine;
    ifstream    inpFileStream;
    string      datFile = inpFileName;
    long unsigned int  mylongunsigned;
    unsigned int i = 0;
    unsigned int bytes_per_line = 8;
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

      memcpy(charOutput+(i*sizeof(long unsigned int)),(char*)&mylongunsigned, sizeof(long unsigned int));
      i++;
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
 * DEPRECATED
 ******************************************************************************/
bool dumpFileToStringWithoutCommands(const string inpFileName, char* charOutput, int simCnt) {
    string      strLine;
    ifstream    inpFileStream;
    string      datFile = "../../../../test/" + inpFileName;
    UdpWord     udpWord=NetworkWord(0,0,0);
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

void ascii2hexWithSize(const string& in, string& out, size_t  bytesize)
{
 std::stringstream sstream;
    for ( int i=0; i<bytesize; i++){
        sstream << std::hex << int(in[i]);
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
	for (int i = 0; i < byteSize; i++)
	{
		std::sprintf(out+i, "%d", (int)in[i]);
	}
}

void string2hexUnsignedNumerics(const string& in, char * out, size_t byteSize)
{
	for (int i = 0; i < byteSize; i++)
	{
		std::sprintf(out+i, "%u", (unsigned int)in[i]);
	}
}

void stringHex2Unsigned(const string& in, unsigned int * out, size_t byteSize)
{
	for (int i = 0; i < byteSize; i++)
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
	for (int i = 0; i < byteSize; i++)
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
void createMemTestCommands(unsigned int mem_address, string& out, unsigned int testingNumber)
{
	unsigned int bytes_per_line = 8;
	char start_cmd [bytes_per_line]; // Half of the command filled with start other half with the address
	char stop_cmd [bytes_per_line];
	char filler_cmd [bytes_per_line];
	char value[bytes_per_line];
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
  memcpy(start_cmd+1, (char*)&testingNumber, 2);
	out.append(start_cmd,3);
  memcpy(value, (char*)&mem_address, 4);
  out.append(value,5);
}

/*****************************************************************************
 * @brief Create the expected output results for the memory test (with FAULT INJECTION)
 *
 * @param[in]  mem_address max target address to test
 * @param[out] out the results of the memory test (with FAULT INJECTION)
 * @param[in]  testingNumber the number of tests to perform on the memory
 * 
 ******************************************************************************/
void createMemTestGoldenOutput(unsigned int mem_address, string& out, unsigned int testingNumber)
{
	unsigned int bytes_per_line = 8;
	char addr_cmd [bytes_per_line]; // Half of the command filled with start other half with the address
	char fault_cntr_cmd [bytes_per_line];
	char fault_addr_cmd [bytes_per_line];
	char filler_cmd [bytes_per_line];
	char end_of_tests_cmd [bytes_per_line];
	char stop_cmd [bytes_per_line];
    unsigned int fault_addr = 0;
    unsigned int first_faultTests = 3;

//given parameters
    unsigned int mem_word_size = 512;
    unsigned int mem_size_of_word_size = 20;
// computations the first faulty address and the the fault counter
    unsigned int mem_addr_per_word = mem_word_size / 8; // byte size of this word
    unsigned int fault_cntr = 0;

//simulating the fault injection
    for (unsigned int j = 0; j < mem_address; j++)
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
        }
        stop_cmd[0]    = (char)2;
        end_of_tests_cmd[0]    = (char)3;
        memcpy(end_of_tests_cmd+1, (char*)&testingNumber, 2);

        memcpy(addr_cmd, (char*)&mem_address, sizeof(unsigned int));
        out.append(addr_cmd,bytes_per_line);
        //if not yet in the fault injection point just let em empty as expected from good tests
        if(i < first_faultTests-1 )
        {
          out.append(filler_cmd,bytes_per_line);
          out.append(filler_cmd,bytes_per_line);
        }else{
            memcpy(fault_cntr_cmd, (char*)&fault_cntr, sizeof(unsigned int));
            out.append(fault_cntr_cmd,bytes_per_line);
            memcpy(fault_addr_cmd, (char*)&mem_addr_per_word, sizeof(unsigned int));
            out.append(fault_addr_cmd,bytes_per_line);
        }
    }
    out.append(end_of_tests_cmd,bytes_per_line);
  //  out.append(stop_cmd,bytes_per_line);
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
__file_read_hex(const char *fname, char *buff, size_t len, size_t byte_per_line)
{
	int rc;
	FILE *fp;
    ifstream infile(fname, fstream::in);


	if ((fname == NULL) || (buff == NULL) || (len == 0))
		return -EINVAL;


  infile.setf (std::ios::hex);
  char tmp;
  char tmp_buff [byte_per_line];
  string tmp_buff_string;
  printf("trying to read this hex file...\n");
  for (size_t i = 0; i < len; i+=byte_per_line)
  {
    infile.getline(tmp_buff, byte_per_line);
    tmp_buff_string.append(tmp_buff,byte_per_line);
    cout << "printing the usigned int " << tmp_buff_string << endl;

    long unsigned int tmp_u = stoul(tmp_buff_string);
    memcpy(buff+(i*byte_per_line), tmp_buff , byte_per_line);
    cout << buff << endl;
    cout << strlen(buff) << " lenght " << endl;
   // infile.get(buff+(i*byte_per_line), byte_per_line);
    printCharBuffHex(buff+(i*byte_per_line), byte_per_line);
    printCharBuffHex(buff, byte_per_line*(i+1));
    printf("GNAAAAAAAA\n");
    tmp_buff_string.erase();
    //remove the \n
    //infile.get(&tmp, 1);
    
  }
  
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
