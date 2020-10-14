/*****************************************************************************
 * @file       commom.cpp
 * @brief      Common functions for testbenches - body.
 *
 * @date       October 2020
 * @author     FAB, WEI, NGL, DID
 * 
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2020 - IBM Research - All Rights Reserved.
 *
 * @ingroup VitisVision 
 * @addtogroup VitisVision 
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
 * @brief Fill an output file with data from an image.
 * 
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpStructToFile(varin *instruct, const string outFileName, int simCnt)
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
    printf("came to dumpStructToFile: s size=%u\n", INSIZE);
    
    ap_uint<8> value[bytes_per_line];
    unsigned int total_bytes = 0;

    
    intToFloatUnion intToFloat;
    
    //-- STEP-2 : DUMP STRING DATA TO FILE
	for (unsigned int i = 0, j = 0; i < INSIZE; i+=sizeof(DtUsed), j++, total_bytes+=bytes_per_line) {
	  
	  
	  
	  
	  switch(j)
	  {
	    case 0:
	      intToFloat.i = instruct->loop_nm;
	      printf("DEBUG instruct->loop_nm = %u\n", instruct->loop_nm);
	      break;
	    case 1:
	      intToFloat.i = instruct->seed;
	      printf("DEBUG instruct->seed = %u\n", instruct->seed);
	      break;  
	    case 2:
	      intToFloat.f = instruct->underlying;
	      printf("DEBUG instruct->underlying = %f\n", instruct->underlying);
	      break;
	    case 3:
	      intToFloat.f = instruct->volatility;
	      printf("DEBUG instruct->volatility = %f\n", instruct->volatility);
	      break;
	    case 4:
	      intToFloat.f = instruct->dividendYield;
	      printf("DEBUG instruct->dividendYield = %f\n", instruct->dividendYield);
	      break;
	    case 5:
	      intToFloat.f = instruct->riskFreeRate;
	      printf("DEBUG instruct->riskFreeRate = %f\n", instruct->riskFreeRate);
	      break;
	    case 6:
	      intToFloat.f = instruct->timeLength;
	      printf("DEBUG instruct->timeLength = %f\n", instruct->timeLength);
	      break;  
	    case 7:
	      intToFloat.f = instruct->strike;
	      printf("DEBUG instruct->strike = %f\n", instruct->strike);
	      break;
	    case 8:
	      intToFloat.i = instruct->optionType;
	      printf("DEBUG instruct->optionType = %u\n", instruct->optionType);
	      break;
	    case 9:
	      intToFloat.f = instruct->requiredTolerance;
	      printf("DEBUG instruct->requiredTolerance = %f\n", instruct->requiredTolerance);
	      break;
	    case 10:
	      intToFloat.i = instruct->requiredSamples;
	      printf("DEBUG instruct->requiredSamples = %u\n", instruct->requiredSamples);
	      break;
	    case 11:
	      intToFloat.i = instruct->timeSteps;
	      printf("DEBUG instruct->timeSteps = %u\n", instruct->timeSteps);
	      break;  
	    case 12:
	      intToFloat.i = instruct->maxSamples;
	      printf("DEBUG instruct->maxSamples = %u\n", instruct->maxSamples);
	      break;
	    default:
	      printf("ERROR: unknown value %u\n", j);
	      rc = KO;
	      break;
	  }
	  
	  
	  
	  
	    udpWord.tdata = (ap_uint<64>)intToFloat.i;
	    udpWord.tkeep = 255;
	    // We are signaling a packet termination either at the end of the image or the end of MTU
	    if ((total_bytes >= (INSIZE - bytes_per_line)) || 
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
bool dumpFileToArray(const string inpFileName, DtUsed* out, int simCnt) {
    string      strLine;
    ifstream    inpFileStream;
    string      datFile = "../../../../test/" + inpFileName;
    UdpWord     udpWord;
    unsigned int i = 0;
    unsigned int bytes_per_line = 8;
    ap_uint<8> value[bytes_per_line];
    intToFloatUnion intToFloat;
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
	  //unpack_ap_uint_64_(udpWord.tdata, value);

	  intToFloatUnion intToFloat;
	  intToFloat.i  = (DtUsedInt)(udpWord.tdata);
	  assert(i<OUTDEP);
	  out[i++] = intToFloat.f;
	}
    }

    //-- STEP-3: CLOSE FILE
    inpFileStream.close();

    return(OK);
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
writeArrayToFile(const char *fname, DtUsed* out)
{
	int rc;
	FILE *fp;

	if ((fname == NULL) || (out == NULL))
		return -EINVAL;

	fp = fopen(fname, "w+");
	if (!fp) {
		fprintf(stderr, "err: Cannot open file %s: %s\n",
			fname, strerror(errno));
		return -ENODEV;
	}
	for (unsigned int i = 0; i < OUTDEP; i++) {
	    rc = fprintf(fp,"%f\n", out[i]);
	    if (rc == -1) {
		fprintf(stderr, "err: Cannot write to %s: %s\n",
			fname, strerror(errno));
		fclose(fp);
		return -EIO;
	    }
	}
	fclose(fp);
	return rc;
}


/*! \} */