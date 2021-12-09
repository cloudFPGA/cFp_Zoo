/*****************************************************************************
 * @file       commom.cpp
 * @brief      Common functions for testbenches - body.
 *
 * @date       June 2020
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
using namespace cv;




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
 * @brief Initialize an input array from a file with format "tdata tkeep tlast"
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] imgArray the array to write the tdata only field from the file.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool setInputFileToArray(const string inpFileName, ap_uint<OUTPUT_PTR_WIDTH>* imgArray, int simCnt) {
    string                   strLine;
    ifstream                 inpFileStream;
    string                   datFile = "../../../../test/" + inpFileName;
    UdpWord                  udpWord;
    unsigned int             index = 0;
    ap_uint<OUTPUT_PTR_WIDTH> current_tdata;
    
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
	    for (unsigned int i=0; i<(BITS_PER_10GBITETHRNET_AXI_PACKET/OUTPUT_PTR_WIDTH); i++) {
	      current_tdata = (ap_uint<OUTPUT_PTR_WIDTH>)(udpWord.tdata >> i*8);//FIXME: check 
	                                                                       // possible seg.fault
	      imgArray[index++] = current_tdata;
	      // Print Data to console
	      printf("[%4.4d] TB is filling input array from [%s] - Data write = {D=0x%16.16llX, K=0x%2.2X, L=%d, current_tdata=0x%16.16llX} \n",
		      simCnt, inpFileName.c_str(),
		      udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int(), 
		      current_tdata.to_long());
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

    ap_uint<64>  value ;

    value = buffer[7] ;
    value = (value << 8 ) + buffer[6] ;
    value = (value << 8 ) + buffer[5] ;
    value = (value << 8 ) + buffer[4] ;
    value = (value << 8 ) + buffer[3] ;
    value = (value << 8 ) + buffer[2] ;
    value = (value << 8 ) + buffer[1] ;
    value = (value << 8 ) + buffer[0] ;

    return value ;

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
bool dumpImgToFile(xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPIX>& _img,
		   const string   outFileName, int simCnt)
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
    printf("came to dumpImgToFile: _img.rows=%u, img.cols=%u\n", _img.rows, _img.cols);
    
    ap_uint<8> value[bytes_per_line];
    
    //-- STEP-2 : DUMP IMAGE DATA TO FILE
    for (unsigned int total_bytes = 0, chan =0 ; chan < _img.channels(); chan++) {
    for (unsigned int j = 0; j < _img.rows; j++) {
      int l = 0;
	for (unsigned int i = 0; i < (_img.cols >> XF_BITSHIFT(NPIX)); i+=bytes_per_line, total_bytes+=bytes_per_line) {
	  //if (NPIX == XF_NPPC8) {
	    for (unsigned int k = 0; k < bytes_per_line; k++) {
	      value[k] = _img.read(j * (_img.cols >> XF_BITSHIFT(NPIX)) + i + k);
	    }
	    udpWord.tdata = pack_ap_uint_64_(value);
	    udpWord.tkeep = 255;
	    // We are signaling a packet termination either at the end of the image or the end of MTU
	    if ((total_bytes >= (_img.rows * _img.cols * _img.channels() - bytes_per_line)) || 
	        ((total_bytes + bytes_per_line) % PACK_SIZE == 0)) {
	      udpWord.tlast = 1;
	    }
	    else {
	      udpWord.tlast = 0;
	    }
            printf("[%4.4d] IMG TB is dumping image to file [%s] - Data read [%u] = {val=%u, D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                    simCnt, datFile.c_str(), total_bytes, value,
                    udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int());
            if (!dumpDataToFile(&udpWord, outFileStream)) {
	      rc = KO;
              break;
            }
	  //}
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
bool dumpImgToFileWarpTransform(xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPIX>& _img,
           const string   outFileName, int simCnt, float * transform_matrix)
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
    printf("came to dumpImgToFile: _img.rows=%u, img.cols=%u\n", _img.rows, _img.cols);
    
    ap_uint<8> value[bytes_per_line];
    ap_uint<8> tx_cmd[bytes_per_line];
    ap_uint<8> img_cmd[bytes_per_line];
    //init tx and img cmd
    for (unsigned int k = 0; k < bytes_per_line; k++) {
       value[k]    = (char)0;
        if (k != 0) {
            tx_cmd[k] = (char)0;
            img_cmd[k] = (char)0;
        }
        else {
            tx_cmd[k] = (char)1; 
            img_cmd[k] = (char)2;
        }
     }

     //dump tx cmd
    udpWord.tdata = pack_ap_uint_64_(tx_cmd);
    udpWord.tkeep = 255;
    udpWord.tlast = 0;
    if (!dumpDataToFile(&udpWord, outFileStream)) {
        rc = KO;
        outFileStream.close();
        return(rc);
    }
    int off = 0;
    for (int i = 0; i < 8; i++)
    {
        memcpy(value+off, (char*)transform_matrix+i, 4);
        off += 4;
        off = off % bytes_per_line;
        if (i%2 && i!=0)
        {
            udpWord.tdata = pack_ap_uint_64_(value);
            udpWord.tkeep = 255;
            udpWord.tlast = 0;
            if (!dumpDataToFile(&udpWord, outFileStream)) {
                rc = KO;
                outFileStream.close();
                return(rc);
            }
        }

    }

    //dump last value
    unsigned int zero_constant = 0;
    memcpy(value, (char*)transform_matrix+8, 4);
    memcpy(value, (char*)&zero_constant, 4);
    udpWord.tdata = pack_ap_uint_64_(value);
    udpWord.tkeep = 255;
    udpWord.tlast = 0;
    if (!dumpDataToFile(&udpWord, outFileStream)) {
        rc = KO;
        outFileStream.close();
        return(rc);
    }

    //creating img mat cmd
    memcpy(img_cmd+6, (char*)&_img.rows, 2);
    memcpy(img_cmd+4, (char*)&_img.cols, 2);
    img_cmd[1]=_img.channels();

    udpWord.tdata = pack_ap_uint_64_(img_cmd);
    udpWord.tkeep = 255;
    udpWord.tlast = 0;
    if (!dumpDataToFile(&udpWord, outFileStream)) {
        rc = KO;
        outFileStream.close();
        return(rc);
    }
    //-- STEP-2 : DUMP IMAGE DATA TO FILE
    for (unsigned int total_bytes = 0, chan =0 ; chan < _img.channels(); chan++) {
    for (unsigned int j = 0; j < _img.rows; j++) {
      int l = 0;
    for (unsigned int i = 0; i < (_img.cols >> XF_BITSHIFT(NPIX)); i+=bytes_per_line, total_bytes+=bytes_per_line) {
      //if (NPIX == XF_NPPC8) {
        for (unsigned int k = 0; k < bytes_per_line; k++) {
          value[k] = _img.read(j * (_img.cols >> XF_BITSHIFT(NPIX)) + i + k);
        }
        udpWord.tdata = pack_ap_uint_64_(value);
        udpWord.tkeep = 255;
        // We are signaling a packet termination either at the end of the image or the end of MTU
        if ((total_bytes >= (_img.rows * _img.cols * _img.channels() - bytes_per_line)) || 
            ((total_bytes + bytes_per_line) % PACK_SIZE == 0)) {
          udpWord.tlast = 1;
        }
        else {
          udpWord.tlast = 0;
        }
            printf("[%4.4d] IMG TB is dumping image to file [%s] - Data read [%u] = {val=%u, D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                    simCnt, datFile.c_str(), total_bytes, value,
                    udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int());
        if (!dumpDataToFile(&udpWord, outFileStream)) {
          rc = KO;
              break;
        }
      //}
    }
    }
    }
    //-- STEP-3: CLOSE FILE
    outFileStream.close();

    return(rc);
}


/*****************************************************************************
 * @brief Write the corners found by Harris into a file.
 * 
 * @return 0 if successful, otherwise 1.
 ******************************************************************************/
unsigned int writeCornersIntoFile(cv::Mat& in_img, cv::Mat& ocv_out_img, cv::Mat& out_img, 
		             std::vector<cv::Point>& hls_points,
			     std::vector<cv::Point>& ocv_points,
			     std::vector<cv::Point>& common_pts) {

	cv::Mat ocvpnts, hlspnts;
 
        ocvpnts = in_img.clone();

        int nhls = hls_points.size();

        /// Drawing a circle around corners
        for (int j = 1; j < ocv_out_img.rows - 1; j++) {
            for (int i = 1; i < ocv_out_img.cols - 1; i++) {
                if ((int)ocv_out_img.at<unsigned char>(j, i)) {
                    cv::circle(ocvpnts, cv::Point(i, j), 5, cv::Scalar(0, 0, 255), 2, 8, 0);
                    ocv_points.push_back(cv::Point(i, j));
                }
            }
        }

        printf("ocv corner count = %d, Hls corner count = %d\n", ocv_points.size(), hls_points.size());
        int nocv = ocv_points.size();

        /*									End
         */
        /*							Find common points in among opencv and HLS
         */
        int ocv_x, ocv_y, hls_x, hls_y;
        for (int j = 0; j < nocv; j++) {
            for (int k = 0; k < nhls; k++) {
                ocv_x = ocv_points[j].x;
                ocv_y = ocv_points[j].y;
                hls_x = hls_points[k].x;
                hls_y = hls_points[k].y;

                if ((ocv_x == hls_x) && (ocv_y == hls_y)) {
                    common_pts.push_back(ocv_points[j]);
                    break;
                }
            }
        }
        /*							End */
        imwrite("output_hls.png", out_img); // HLS Image
        imwrite("output_ocv.png", ocvpnts); // Opencv Image
        /*						Success, Loss and Gain Percentages */
        float persuccess, perloss, pergain;

        int totalocv = ocv_points.size();
        int ncommon = common_pts.size();
        int totalhls = hls_points.size();
        persuccess = (((float)ncommon / totalhls) * 100);
        perloss = (((float)(totalocv - ncommon) / totalocv) * 100);
        pergain = (((float)(totalhls - ncommon) / totalhls) * 100);

        printf("Commmon = %d\t Success = %f\t Loss = %f\t Gain = %f\n", ncommon, persuccess, perloss, pergain);

        if (persuccess < 60 || totalhls == 0) 
	  return 1;
	else
	  return 0;

}

/*****************************************************************************
 * @brief Mark the points found by Harris into the image.
 * 
 * @return Nothing
 ******************************************************************************/
void markPointsOnImage(xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPIX>& imgOutput, cv::Mat& in_img, 
		       cv::Mat& out_img, std::vector<cv::Point>& hls_points) {

        for (int j = 0; j < imgOutput.rows; j++) {
            int l = 0;
            for (int i = 0; i < (imgOutput.cols >> XF_BITSHIFT(NPIX)); i++) {
                if (NPIX == XF_NPPC8) {
                    ap_uint<64> value =
                        imgOutput.read(j * (imgOutput.cols >> XF_BITSHIFT(NPIX)) + i); //.at<unsigned char>(j,i);
                    for (int k = 0; k < 64; k += 8, l++) {
                        uchar pix = value.range(k + 7, k);
                        if (pix != 0) {
                            cv::Point tmp;
                            tmp.x = l;
                            tmp.y = j;
                            if ((tmp.x < in_img.cols) && (tmp.y < in_img.rows) && (j > 0)) {
                                hls_points.push_back(tmp);
                            }
                            short int y, x;
                            y = j;
                            x = l;
                            if (j > 0) cv::circle(out_img, cv::Point(x, y), 5, cv::Scalar(0, 0, 255, 255), 2, 8, 0);
                        }
                    }
                }
                if (NPIX == XF_NPPC1) {
                    unsigned char pix = imgOutput.read(j * (imgOutput.cols >> XF_BITSHIFT(NPIX)) + i);
                    if (pix != 0) {
                        cv::Point tmp;
                        tmp.x = i;
                        tmp.y = j;
                        if ((tmp.x < in_img.cols) && (tmp.y < in_img.rows) && (j > 0)) {
                            hls_points.push_back(tmp);
                        }
                        short int y, x;
                        y = j;
                        x = i;
                        if (j > 0) cv::circle(out_img, cv::Point(x, y), 5, cv::Scalar(0, 0, 255, 255), 2, 8, 0);
                    }
                }
            }
        }
}




/*! \} */