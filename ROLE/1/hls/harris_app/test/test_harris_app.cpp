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
#define MAX_SIM_CYCLES  500
#define OK          true
#define KO          false
#define VALID       true
#define UNVALID     false
#define DEBUG_TRACE true

#define ENABLED     (ap_uint<1>)1
#define DISABLED    (ap_uint<1>)0

//---------------------------------------------------------
//-- TESTBENCH GLOBAL VARIABLES
//--  These variables might be updated/overwritten by the
//--  content of a test-vector file.
//---------------------------------------------------------
unsigned int    gSimCycCnt    = 0;
bool            gTraceEvent   = false;
bool            gFatalError   = false;
unsigned int    gMaxSimCycles = MAX_SIM_CYCLES;

//------------------------------------------------------
//-- DUT INTERFACES AS GLOBAL VARIABLES
//------------------------------------------------------

//-- SHELL / MMIO / Configuration Interfaces
ap_uint<2>          sSHL_UAF_MmioEchoCtrl;
ap_uint<1>          sSHL_UAF_MmioPostPktEn;
ap_uint<1>          sSHL_UAF_MmioCaptPktEn;

//-- SHELL / UDP Interfaces
stream<UdpWord>     ssSHL_UAF_Data      ("ssSHL_UAF_Data");
stream<UdpWord>     ssUAF_SHL_Data      ("ssUAF_SHL_Data");

//------------------------------------------------------
//-- TESTBENCH GLOBAL VARIABLES
//------------------------------------------------------
int         simCnt;


/*****************************************************************************
 * @brief Run a single iteration of the DUT model.
 * @ingroup harris_app
 * @return Nothing.
 ******************************************************************************/
void stepDut() {
    harris_app(
            sSHL_UAF_MmioEchoCtrl,
            //[TODO] sSHL_UAF_MmioPostPktEn,
            //[TODO] sSHL_UAF_MmioCaptPktEn,
            ssSHL_UAF_Data,
            ssUAF_SHL_Data);

    simCnt++;
    printf("[%4.4d] STEP DUT \n", simCnt);
}

/*****************************************************************************
 * @brief Initialize an input data stream from a file.
 * @ingroup harris_app
 *
 * @param[in] sDataStream, the input data stream to set.
 * @param[in] dataStreamName, the name of the data stream.
 * @param[in] inpFileName, the name of the input file to read from.
 * @return OK if successful, otherwise KO.
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
 * @brief Read data from a stream.
 * @ingroup harris_app
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
 * @brief Dump a data word to a file.
 * @ingroup harris_app
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
 * @ingroup harris_app
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
    cv::Mat ocvpnts, hlspnts;

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


    //------------------------------------------------------
    //-- STEP-1.2 : RUN HARRIS DETECTOR FROM OpenCV LIBRARY
    //------------------------------------------------------
    ocv_ref(in_img, ocv_out_img, Th);


    //------------------------------------------------------
    //-- STEP-2.1 : CREATE TRAFFIC AS INPUT STREAMS
    //------------------------------------------------------
    if (nrErr == 0) {
        if (!setInputDataStream(ssSHL_UAF_Data, "ssSHL_UAF_Data", "ifsSHL_Uaf_Data.dat")) {
            printf("### ERROR : Failed to set input data stream \"sSHL_Uaf_Data\". \n");
            nrErr++;
        }
    }

    //------------------------------------------------------
    //-- STEP-2.2 : SET THE PASS-THROUGH MODE
    //------------------------------------------------------
    sSHL_UAF_MmioEchoCtrl.write(ECHO_PATH_THRU);
    //[TODO] sSHL_UAF_MmioPostPktEn.write(DISABLED);
    //[TODO] sSHL_UAF_MmioCaptPktEn.write(DISABLED);

    //------------------------------------------------------
    //-- STEP-3 : MAIN TRAFFIC LOOP
    //------------------------------------------------------
    while (!nrErr) {

        if (simCnt < 25)
            stepDut();
        else {
            printf("## End of simulation at cycle=%3d. \n", simCnt);
            break;
        }

    }  // End: while()

    //-------------------------------------------------------
    //-- STEP-4 : DRAIN AND WRITE OUTPUT FILE STREAMS
    //-------------------------------------------------------
    //---- UAF-->SHELL Data ----
    if (!getOutputDataStream(ssUAF_SHL_Data, "ssUAF_SHL_Data", "ofsUAF_Shl_Data.dat"))
    {
        nrErr++;
    }

    //------------------------------------------------------
    //-- STEP-5 : COMPARE INPUT AND OUTPUT FILE STREAMS
    //------------------------------------------------------
    int rc1 = system("diff --brief -w -i -y ../../../../test/ofsUAF_Shl_Data.dat \
                                            ../../../../test/ifsSHL_Uaf_Data.dat");
    if (rc1)
        printf("## Error : File \'ofsUAF_Shl_Data.dat\' does not match \'ifsSHL_Uaf_Data.dat\'.\n");

    nrErr += rc1;

    printf("#####################################################\n");
    if (nrErr)
        printf("## ERROR - TESTBENCH FAILED (RC=%d) !!!             ##\n", nrErr);
    else
        printf("## SUCCESSFULL END OF TESTBENCH (RC=0)             ##\n");

    printf("#####################################################\n");

















    /**************		HLS Function	  *****************/
    float K = 0.04;
    uint16_t k = K * (1 << 16); // Convert to Q0.16 format
    uint32_t nCorners = 0;
    uint16_t imgwidth = in_img.cols;
    uint16_t imgheight = in_img.rows;

    #if NO

        static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC1> imgInput(in_img.rows, in_img.cols);
        static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC1> imgOutput(in_img.rows, in_img.cols);

        imgInput.copyTo(in_img.data);
        //	imgInput = xf::cv::imread<XF_8UC1, HEIGHT, WIDTH, XF_NPPC1>(argv[1], 0);
        harris_accel(imgInput, imgOutput, Thresh, k);
        //ap_uint<INPUT_PTR_WIDTH> imgInput_tb[128*128];
        //ap_uint<INPUT_PTR_WIDTH> imgOutput_tb[128*128];
        //cornerHarris_accel(imgInput_tb, imgOutput_tb, in_img.rows, in_img.cols, Thresh, k);

    #endif

    #if RO

        static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC8> imgInput(in_img.rows, in_img.cols);
        static xf::cv::Mat<XF_8UC1, HEIGHT, WIDTH, XF_NPPC8> imgOutput(in_img.rows, in_img.cols);

        // imgInput.copyTo(img_gray.data);
        imgInput = xf::cv::imread<XF_8UC1, HEIGHT, WIDTH, XF_NPPC8>(argv[1], 0);
        harris_accel(imgInput, imgOutput, Thresh, k);

    #endif

        /// hls_out_img.data = (unsigned char *)imgOutput.copyFrom();
        xf::cv::imwrite("hls_out.jpg", imgOutput);

        unsigned int val;
        unsigned short int row, col;

        cv::Mat out_img;
        out_img = in_img.clone();

        std::vector<cv::Point> hls_points;
        std::vector<cv::Point> ocv_points;
        std::vector<cv::Point> common_pts;
        /*						Mark HLS points on the image 				*/

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

        /*						End of marking HLS points on the image 				*/
        /*						Write HLS and Opencv corners into a file			*/

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

        if (persuccess < 60 || totalhls == 0) return 1;

        //return 0; // of original Harris

    return(nrErr);
}
