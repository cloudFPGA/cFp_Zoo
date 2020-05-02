/*****************************************************************************
 * @file       : test_harris_app_flash.cpp
 * @brief      : Testbench for UDP Application Flash (UAF).
 *
 * System:     : cloudFPGA
 * Component   : Role
 * Language    : Vivado HLS
 *
 * Created: April 2020
 * Authors: FAB, WEI, NGL, DID
 * 
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2020 - IBM Research - All Rights Reserved.
 *
 *****************************************************************************/

#include <stdio.h>
#include <hls_stream.h>

#include "../src/harris_app.hpp"

#include "common/xf_headers.hpp"
#include "../include/xf_harris_config.h"
#include "../include/xf_ocv_ref.hpp"

using namespace std;

//---------------------------------------------------------
// HELPERS FOR THE DEBUGGING TRACES
//  .e.g: DEBUG_LEVEL = (MDL_TRACE | IPS_TRACE)
//---------------------------------------------------------
#define THIS_NAME "TB"

#define TRACE_OFF     0x0000
#define TRACE_URIF   1 <<  1
#define TRACE_UAF    1 <<  2
#define TRACE_MMIO   1 <<  3
#define TRACE_ALL     0xFFFF

#define DEBUG_LEVEL (TRACE_ALL)


//------------------------------------------------------
//-- TESTBENCH DEFINES
//------------------------------------------------------
#define OK          true
#define KO          false
#define VALID       true
#define UNVALID     false
#define DEBUG_TRACE true

#define ENABLED     (ap_uint<1>)1
#define DISABLED    (ap_uint<1>)0


//------------------------------------------------------
//-- DUT INTERFACES AS GLOBAL VARIABLES
//------------------------------------------------------

//-- SHELL / Uaf / Mmio / Config Interfaces
//ap_uint<2>          piSHL_This_MmioEchoCtrl;
ap_uint<1>          piSHL_This_MmioPostPktEn;
ap_uint<1>          piSHL_This_MmioCaptPktEn;

//-- SHELL / Uaf / Udp Interfaces
stream<UdpWord>   sSHL_Uaf_Data ("sSHL_Uaf_Data");
stream<UdpWord>   sUAF_Shl_Data ("sUAF_Shl_Data");
stream<UdpWord>   image_stream_from_harris_app ("image_stream_from_harris_app");

ap_uint<32>             s_udp_rx_ports = 0x0;
stream<NetworkMetaStream>   siUdp_meta          ("siUdp_meta");
stream<NetworkMetaStream>   soUdp_meta          ("soUdp_meta");
ap_uint<32>             node_rank;
ap_uint<32>             cluster_size;

//------------------------------------------------------
//-- TESTBENCH GLOBAL VARIABLES
//------------------------------------------------------
int         simCnt;


/*****************************************************************************
 * @brief Run a single iteration of the DUT model.
 * @ingroup HarrisTB
 * @return Nothing.
 ******************************************************************************/
void stepDut() {
    harris_app(
        &node_rank, &cluster_size,
      sSHL_Uaf_Data, sUAF_Shl_Data,
      siUdp_meta, soUdp_meta,
      &s_udp_rx_ports);
    simCnt++;
    printf("[%4.4d] STEP DUT \n", simCnt);
}


/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 * @ingroup HarrisTB
 *
 * @param[in] sDataStream the input data stream to set.
 * @param[in] dataStreamName the name of the data stream.
 * @param[in] inpFileName the name of the input file to read from.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool setInputDataStream(stream<UdpWord> &sDataStream, const string dataStreamName, const string inpFileName) {
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
 * @ingroup HarrisTB
 *
 * @param[in]  inpFileName the name of the input file to read from.
 * @param[out] imgOutputArray the array to write the tdata only field from the file.
 * @return OK if successful otherwise KO.
 ******************************************************************************/
bool setInputFileToArray(const string inpFileName, ap_uint<64>* imgOutputArray) {
    string      strLine;
    ifstream    inpFileStream;
    string      datFile = "../../../../test/" + inpFileName;
    UdpWord     udpWord;
    unsigned int index = 0;
    
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
	    imgOutputArray[index++] = udpWord.tdata; //FIXME: check possible seg.fault
            // Print Data to console
            printf("[%4.4d] TB is filling input array from [%s] - Data write = {D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
	      simCnt, inpFileName.c_str(),
              udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int());
        }
    }

    //-- STEP-3: CLOSE FILE
    inpFileStream.close();

    return(OK);
}


/*****************************************************************************
 * @brief Read data from a stream.
 * @ingroup HarrisTB
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
 * @ingroup HarrisTB
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
 * @ingroup HarrisTB
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
 * @ingroup HarrisTB
 *
 * @param[in] sDataStream,    the output data stream to set.
 * @param[in] dataStreamName, the name of the data stream.
 * @param[in] outFileName,    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool getOutputDataStream(stream<UdpWord> &sDataStream,
                         const string    dataStreamName, const string   outFileName)
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
 * @ingroup HarrisTB
 *
 * @param[in] sDataStream    the input image in xf::cv::Mat format.
 * @param[in] outFileName    the name of the output file to write to.
 * @return OK if successful, otherwise KO.
 ******************************************************************************/
bool dumpImgToFile(xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& _img,
		   const string   outFileName)
{
    string      strLine;
    ofstream    outFileStream;
    string      datFile = "../../../../test/" + outFileName;
    UdpWord     udpWord;
    bool        rc = OK;
    unsigned int vals_per_line = 8;
    
    //-- STEP-1 : OPEN FILE
    outFileStream.open(datFile.c_str());
    if ( !outFileStream ) {
        cout << "### ERROR : Could not open the output data file " << datFile << endl;
        return(KO);
    }
    printf("came to dumpImgToFile: _img.rows=%u, img.cols=%u\n", _img.rows, _img.cols);
    
    ap_uint<8> value[vals_per_line];
    
    //-- STEP-2 : DUMP IMAGE DATA TO FILE
    for (unsigned int count = 0, j = 0; j < _img.rows; j++) {
      int l = 0;
	for (unsigned int i = 0; i < (_img.cols >> XF_BITSHIFT(NPIX)); i+=vals_per_line, count+=vals_per_line) {
	  //if (NPIX == XF_NPPC8) {
	    for (unsigned int k = 0; k < vals_per_line; k++) {
	      value[k] = _img.read(j * (_img.cols >> XF_BITSHIFT(NPIX)) + i + k);
	    }
	    udpWord.tdata = pack_ap_uint_64_(value);
	    udpWord.tkeep = 255;
	    if (count >= (_img.rows * _img.cols - vals_per_line)) {
	      udpWord.tlast = 1;
	    }
	    else {
	      udpWord.tlast = 0;
	    }
            printf("[%4.4d] IMG TB is dumping image to file [%s] - Data read [%u] = {val=%u, D=0x%16.16llX, K=0x%2.2X, L=%d} \n",
                    simCnt, datFile.c_str(), count, value,
                    udpWord.tdata.to_long(), udpWord.tkeep.to_int(), udpWord.tlast.to_int());
            if (!dumpDataToFile(&udpWord, outFileStream)) {
	      rc = KO;
              break;
            }
	  //}
	}
    }
    
    //-- STEP-3: CLOSE FILE
    outFileStream.close();

    return(rc);
}


/*****************************************************************************
 * @brief Write the corners found by Harris into a file.
 * @ingroup HarrisTB
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
 * @ingroup HarrisTB
 *
 * @return Nothing
 ******************************************************************************/
void markPointsOnImage(xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>& imgOutput, cv::Mat& in_img, cv::Mat& out_img, std::vector<cv::Point>& hls_points) {

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


/*****************************************************************************
 * @brief Main testbench of Hrris.
 * @ingroup HarrisTB
 *
 * @return 0 upon success, nrErr else.
 ******************************************************************************/
int main(int argc, char** argv) {

    //------------------------------------------------------
    //-- TESTBENCH LOCAL VARIABLES
    //------------------------------------------------------
    int         nrErr = 0;

    printf("#####################################################\n");
    printf("## TESTBENCH STARTS HERE                           ##\n");
    printf("#####################################################\n");

    simCnt = 0;
    nrErr  = 0;

    //------------------------------------------------------
    //-- TESTBENCH LOCAL VARIABLES FOR HARRIS
    //------------------------------------------------------
    cv::Mat in_img, img_gray;
    cv::Mat hls_out_img, ocv_out_img;

    if (argc != 2) {
        printf("Usage : %s <input image> \n", argv[0]);
        return -1;
    }
    in_img = cv::imread(argv[1], 0); // reading in the color image

    if (!in_img.data) {
        printf("Failed to load the image ... %s\n!", argv[1]);
        return -1;
    }

    uint16_t Thresh; // Threshold for HLS
    float Th;
    if (FILTER_WIDTH == 3) {
        Th = 30532960.00;
        Thresh = 442;
    } else if (FILTER_WIDTH == 5) {
        Th = 902753878016.0;
        Thresh = 3109;
    } else if (FILTER_WIDTH == 7) {
        Th = 41151168289701888.000000;
        Thresh = 566;
    }


    //------------------------------------------------------
    //-- STEP-1.1 : CREATE MEMORY FOR OUTPUT IMAGES
    //------------------------------------------------------
    //	cvtColor(in_img, img_gray, CV_BGR2GRAY);
    // Convert rgb into grayscale
    hls_out_img.create(in_img.rows, in_img.cols, CV_8U); // create memory for hls output image
    ocv_out_img.create(in_img.rows, in_img.cols, CV_8U); // create memory for opencv output image

    #if NO

    static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC1> imgInput(in_img.rows, in_img.cols);
    static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC1> imgOutput(in_img.rows, in_img.cols);
    static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC1> imgOutputTb(in_img.rows, in_img.cols);

    imgInput.copyTo(in_img.data);
    //	imgInput = xf::cv::imread<XF_8UC1, HEIGHT, WIDTH, XF_NPPC1>(argv[1], 0);
	
    ap_uint<INPUT_PTR_WIDTH> imgInputArray[in_img.rows * in_img.cols];
    ap_uint<OUTPUT_PTR_WIDTH> imgOutputArrayTb[in_img.rows * in_img.cols];
    ap_uint<OUTPUT_PTR_WIDTH> imgOutputArray[in_img.rows * in_img.cols];
  
    xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgInput, imgInputArray);
	
    if (!dumpImgToFile(imgInput, "imgInput.txt")) {
      nrErr++;
    }

    #endif

    #if RO

    static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC8> imgInput(in_img.rows, in_img.cols);
    static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC8> imgOutput(in_img.rows, in_img.cols);
    static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC1> imgOutputTb(in_img.rows, in_img.cols);

    // imgInput.copyTo(img_gray.data);
    imgInput = xf::cv::imread<XF_8UC1, HEIGHT, WIDTH, XF_NPPC8>(argv[1], 0);

    #endif
    
    
    
    //------------------------------------------------------
    //-- STEP-1.2 : RUN HARRIS DETECTOR FROM OpenCV LIBRARY
    //------------------------------------------------------
    ocv_ref(in_img, ocv_out_img, Th);


    //------------------------------------------------------
    //-- STEP-2.1 : CREATE TRAFFIC AS INPUT STREAMS
    //------------------------------------------------------
    if (nrErr == 0) {
        if (!setInputDataStream(sSHL_Uaf_Data, "sSHL_Uaf_Data", "imgInput.txt")) {
            printf("### ERROR : Failed to set input data stream \"sSHL_Uaf_Data\". \n");
            nrErr++;
        }

        //there are 2 streams from the the App to the Role
        NetworkMeta tmp_meta = NetworkMeta(1,DEFAULT_RX_PORT,0,DEFAULT_RX_PORT,0);
        siUdp_meta.write(NetworkMetaStream(tmp_meta));
        siUdp_meta.write(NetworkMetaStream(tmp_meta));
        //set correct node_rank and cluster_size
        node_rank = 1;
        cluster_size = 2;
    }

    //------------------------------------------------------
    //-- STEP-2.2 : SET THE PASS-THROUGH MODE
    //------------------------------------------------------
    //piSHL_This_MmioEchoCtrl.write(ECHO_PATH_THRU);
    //[TODO] piSHL_This_MmioPostPktEn.write(DISABLED);
    //[TODO] piSHL_This_MmioCaptPktEn.write(DISABLED);

    //------------------------------------------------------
    //-- STEP-3 : MAIN TRAFFIC LOOP
    //------------------------------------------------------
    while (!nrErr) {

        if (simCnt < IMG_PACKETS*2+10)
        {
            stepDut();

            if(simCnt > 2)
            {
              assert(s_udp_rx_ports == 0x1);
            }

            //if( !soUdp_meta.empty())
            //{
            //  NetworkMetaStream tmp_meta = soUdp_meta.read();
            //  printf("NRC received NRCmeta stream from node_rank %d.\n", (int) tmp_meta.tdata.src_rank);
            //}


        } else {
            printf("## End of simulation at cycle=%3d. \n", simCnt);
            break;
        }

    }  // End: while()

    //-------------------------------------------------------
    //-- STEP-4 : DRAIN AND WRITE OUTPUT FILE STREAMS
    //-------------------------------------------------------
    //---- UAF-->SHELL Data ----
    if (!getOutputDataStream(sUAF_Shl_Data, "sUAF_Shl_Data", "ofsUAF_Shl_Data.dat"))
    {
        nrErr++;
    }
    //---- UAF-->SHELL META ----
    if( !soUdp_meta.empty())
    {
      int i = 0;
      while( !soUdp_meta.empty())
      {
        i++;
        NetworkMetaStream tmp_meta = soUdp_meta.read();
        printf("NRC received NRCmeta stream from rank %d to rank %d.\n", (int) tmp_meta.tdata.src_rank, (int) tmp_meta.tdata.dst_rank);
        assert(tmp_meta.tdata.src_rank == node_rank);
        //ensure forwarding behavior
        assert(tmp_meta.tdata.dst_rank == ((tmp_meta.tdata.src_rank + 1) % cluster_size));
      }
      assert(i == 1);
    }
    else {
      printf("Error No metadata received...\n");
      nrErr++;
    }
    
    //-------------------------------------------------------
    //-- STEP-5 : FROM THE OUTPUT FILE CREATE AN ARRAY
    //-------------------------------------------------------    
    if (!setInputFileToArray("ofsUAF_Shl_Data.dat", imgOutputArray)) {
      printf("### ERROR : Failed to set input array from file \"ofsUAF_Shl_Data.dat\". \n");
      nrErr++;
    }
    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgOutputArray, imgOutput);


    //------------------------------------------------------
    //-- STEP-6 : COMPARE INPUT AND OUTPUT FILE STREAMS
    //------------------------------------------------------
    int rc1 = system("diff --brief -w -i -y ../../../../test/ofsUAF_Shl_Data.dat \
                                            ../../../../test/verify_UAF_Shl_Data.dat");
    if (rc1)
    {
        printf("## Error : File \'ofsUAF_Shl_Data.dat\' does not match \'verify_UAF_Shl_Data.dat\'.\n");
    } else {
      printf("Output data in file \'ofsUAF_Shl_Data.dat\' verified.\n");
    }

    nrErr += rc1;

    printf("#####################################################\n");
    if (nrErr) 
    {
        printf("## ERROR - TESTBENCH FAILED (RC=%d) !!!             ##\n", nrErr);
    } else {
        printf("## SUCCESSFULL END OF TESTBENCH (RC=0)             ##\n");
    }
    printf("#####################################################\n");













    /**************		HLS Function	  *****************/
    float K = 0.04;
    uint16_t k = K * (1 << 16); // Convert to Q0.16 format

    #if NO

    // L2 Vitis Harris
    my_cornerHarris_accel(imgInputArray, imgOutputArrayTb, in_img.rows, in_img.cols, Thresh, k);
    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, XF_8UC1, HEIGHT, WIDTH, NPIX>(imgOutputArrayTb, imgOutputTb);
        
    // L1 Vitis Harris 
    //harris_accel(imgInput, imgOutput, Thresh, k);
	
    #endif

    #if RO

    harris_accel(imgInput, imgOutputTb, Thresh, k);

    #endif

        /// hls_out_img.data = (unsigned char *)imgOutput.copyFrom();
        xf::cv::imwrite("hls_out_tb.jpg", imgOutputTb);
        xf::cv::imwrite("hls_out.jpg", imgOutput);

        unsigned int val;
        unsigned short int row, col;

        cv::Mat out_img;
        out_img = in_img.clone();

        std::vector<cv::Point> hls_points;
        std::vector<cv::Point> ocv_points;
        std::vector<cv::Point> common_pts;

	
	xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, NPIX>* select_imgOutput;
	
	
	// Select which output you want to process for image outputs and corners comparisons:
	// &imgOutput   : The processed image by Harris IP inside the ROLE (i.e. I/O traffic is passing through SHELL)
	// &imgOutputTb : The processed image by Harris IP in this testbench (i.e. I/O traffic is done in testbench)
	select_imgOutput = &imgOutput;
	
	/* Mark HLS points on the image */
	markPointsOnImage(*select_imgOutput, in_img, out_img, hls_points);
 
	/* Write HLS and Opencv corners into a file */
	nrErr += writeCornersIntoFile(in_img, ocv_out_img, out_img, hls_points, ocv_points, common_pts);
	
	


    return(nrErr);
}
