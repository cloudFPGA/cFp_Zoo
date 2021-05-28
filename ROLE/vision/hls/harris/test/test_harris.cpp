/*****************************************************************************
 * @file       : test_harris.cpp
 * @brief      : Testbench for Harris.
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
 * @ingroup HarrisTB
 * @addtogroup HarrisTB
 * \{
 *****************************************************************************/

#include "../../common/src/common.cpp"

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
//ap_uint<2>                piSHL_This_MmioEchoCtrl;
ap_uint<1>                  piSHL_This_MmioPostPktEn;
ap_uint<1>                  piSHL_This_MmioCaptPktEn;

//-- SHELL / Uaf / Udp Interfaces
stream<UdpWord>             sSHL_Uaf_Data ("sSHL_Uaf_Data");
stream<UdpWord>             sUAF_Shl_Data ("sUAF_Shl_Data");
stream<UdpWord>             image_stream_from_harris ("image_stream_from_harris");

ap_uint<32>                 s_udp_rx_ports = 0x0;
stream<NetworkMetaStream>   siUdp_meta          ("siUdp_meta");
stream<NetworkMetaStream>   soUdp_meta          ("soUdp_meta");
ap_uint<32>                 node_rank;
ap_uint<32>                 cluster_size;

#ifdef ENABLE_DDR

//------------------------------------------------------
//-- SHELL / Role / Mem / Mp0 Interface
//------------------------------------------------------
//---- Read Path (MM2S) ------------
stream<DmCmd>               sROL_Shl_Mem_RdCmdP0("sROL_Shl_Mem_RdCmdP0");
stream<DmSts>               sSHL_Rol_Mem_RdStsP0("sSHL_Rol_Mem_RdStsP0");
stream<Axis<MEMDW_512> >    sSHL_Rol_Mem_ReadP0 ("sSHL_Rol_Mem_ReadP0");
//---- Write Path (S2MM) -----------
stream<DmCmd>               sROL_Shl_Mem_WrCmdP0("sROL_Shl_Mem_WrCmdP0");
stream<DmSts>               sSHL_Rol_Mem_WrStsP0("sSHL_Rol_Mem_WrStsP0");
stream<Axis<MEMDW_512> >    sROL_Shl_Mem_WriteP0("sROL_Shl_Mem_WriteP0");

//------------------------------------------------------
//-- SHELL / Role / Mem / Mp1 Interface
//------------------------------------------------------
#define MEMORY_LINES_512  TOTMEMDW_512 /* 64 KiB */
membus_t   lcl_mem0[MEMORY_LINES_512];
membus_t   lcl_mem1[MEMORY_LINES_512];
#endif

//------------------------------------------------------
//-- TESTBENCH GLOBAL VARIABLES
//------------------------------------------------------
int         simCnt;


/*****************************************************************************
 * @brief Run a single iteration of the DUT model.
 * @return Nothing.
 ******************************************************************************/
void stepDut() {
    harris(
        &node_rank, &cluster_size,
        sSHL_Uaf_Data, sUAF_Shl_Data,
        siUdp_meta, soUdp_meta,
        &s_udp_rx_ports
        #ifdef ENABLE_DDR
                        ,
        // sROL_Shl_Mem_RdCmdP0,
        // sSHL_Rol_Mem_RdStsP0,
        // sSHL_Rol_Mem_ReadP0,
        sROL_Shl_Mem_WrCmdP0,
        sSHL_Rol_Mem_WrStsP0,
        sROL_Shl_Mem_WriteP0,
        lcl_mem0,
        lcl_mem1
      #endif
	  );
    simCnt++;
    printf("[%4.4d] STEP DUT \n", simCnt);
}




/*****************************************************************************
 * @brief Main testbench of Hrris.
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
        printf("ERROR: Failed to load the image ... %s\n!", argv[1]);
        return -1;
    }
    else {
      printf("INFO: Succesfully loaded image ... %s\n", argv[1]);
      if (in_img.total() != FRAME_WIDTH * FRAME_HEIGHT) {
	 printf("WARNING: Resizing input image %s from [%u x %u] to  [%u x %u] !\n", argv[1], in_img.rows, in_img.cols, FRAME_WIDTH, FRAME_HEIGHT);
	 cv::resize(in_img, in_img, cv::Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, cv::INTER_LINEAR);
      }
      // Ensure that the selection of MTU is a multiple of 8 (Bytes per transaction)
      assert(PACK_SIZE % 8 == 0);
    }

    #ifdef ENABLE_DDR
    memset(lcl_mem0,  0x0, sizeof(lcl_mem0));
    memset(lcl_mem1,  0x0, sizeof(lcl_mem1));
    
    DmCmd           dmCmd_MemCmdP0;
    DmSts           dmSts_MemWrStsP0;
    DmSts           dmSts_MemRdStsP0;
    Axis<MEMDW_512> memP0;
    ap_uint<64>     currentMemPattern = 0;
  
    unsigned int ddr_addr_in = 0x0;
    #endif
    
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

    static xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, XF_NPPC1> imgInput(in_img.rows, in_img.cols);
    static xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, XF_NPPC1> imgOutput(in_img.rows, in_img.cols);
    static xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, XF_NPPC1> imgOutputTb(in_img.rows, in_img.cols);

    imgInput.copyTo(in_img.data);
    //	imgInput = xf::cv::imread<IN_TYPE, HEIGHT, WIDTH, XF_NPPC1>(argv[1], 0);
	
    ap_uint<INPUT_PTR_WIDTH>  *imgInputArray    = (ap_uint<INPUT_PTR_WIDTH>*)  malloc(in_img.rows * in_img.cols * sizeof(ap_uint<INPUT_PTR_WIDTH>));
    ap_uint<OUTPUT_PTR_WIDTH> *imgOutputArrayTb = (ap_uint<OUTPUT_PTR_WIDTH>*) malloc(in_img.rows * in_img.cols * sizeof(ap_uint<OUTPUT_PTR_WIDTH>));
    ap_uint<OUTPUT_PTR_WIDTH> *imgOutputArray   = (ap_uint<OUTPUT_PTR_WIDTH>*) malloc(in_img.rows * in_img.cols * sizeof(ap_uint<OUTPUT_PTR_WIDTH>));

    xf::cv::xfMat2Array<INPUT_PTR_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPIX>(imgInput, imgInputArray);
	
    if (!dumpImgToFile(imgInput, "ifsSHL_Uaf_Data.dat", simCnt)) {
      nrErr++;
    }

    #endif

    #if RO

    static xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, XF_NPPC8> imgInput(in_img.rows, in_img.cols);
    static xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, XF_NPPC8> imgOutput(in_img.rows, in_img.cols);
    static xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, XF_NPPC1> imgOutputTb(in_img.rows, in_img.cols);

    // imgInput.copyTo(img_gray.data);
    imgInput = xf::cv::imread<IN_TYPE, HEIGHT, WIDTH, XF_NPPC8>(argv[1], 0);

    #endif
    
    
    
    //------------------------------------------------------
    //-- STEP-1.2 : RUN HARRIS DETECTOR FROM OpenCV LIBRARY
    //------------------------------------------------------
    ocv_ref(in_img, ocv_out_img, Th);


    //------------------------------------------------------
    //-- STEP-2.1 : CREATE TRAFFIC AS INPUT STREAMS
    //------------------------------------------------------
    if (nrErr == 0) {
        if (!setInputDataStream(sSHL_Uaf_Data, "sSHL_Uaf_Data", "ifsSHL_Uaf_Data.dat", simCnt)) { 
            printf("### ERROR : Failed to set input data stream \"sSHL_Uaf_Data\". \n");
            nrErr++;
        }

        //there are TOT_TRANSFERS streams from the the App to the Role
        NetworkMeta tmp_meta = NetworkMeta(1,DEFAULT_RX_PORT,0,DEFAULT_RX_PORT,0);
	for (int i=0; i<TOT_TRANSFERS; i++) {
	  siUdp_meta.write(NetworkMetaStream(tmp_meta));
	}        
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
	
        // Keep enough simulation time for sequntially executing the FSMs of the main 3 functions 
        // (Rx-Proc-Tx)
        if (simCnt < MIN_RX_LOOPS + MIN_RX_LOOPS + MIN_TX_LOOPS + 10) 
        {
            stepDut();

            if(simCnt > 2)
            {
              assert(s_udp_rx_ports == 0x1);
            }

            
#ifdef ENABLE_DDR

            if (!sROL_Shl_Mem_WrCmdP0.empty()) {
                printf("DEBUG: Read a memory write command from SHELL/Mem/Mp0 \n");
                //-- Read a memory write command from SHELL/Mem/Mp0
                sROL_Shl_Mem_WrCmdP0.read(dmCmd_MemCmdP0);
                assert(dmCmd_MemCmdP0.btt == CHECK_CHUNK_SIZE); 
                assert(dmCmd_MemCmdP0.type == 1 && dmCmd_MemCmdP0.dsa == 0 && dmCmd_MemCmdP0.eof == 1 && dmCmd_MemCmdP0.drr == 0 && dmCmd_MemCmdP0.tag == 0x7);
            }
            
            if (!sROL_Shl_Mem_WriteP0.empty()) {
                sROL_Shl_Mem_WriteP0.read(memP0);
                printf("DEBUG: Write a memory line from SHELL/Mem/Mp0 \n");

                assert(memP0.tkeep == 0xffffffffffffffff);
                
                /* Read from AXI stream DDR interface and write to the memory mapped interface of 
                 * DDR channel P0. In the real HW, this is enabled by the AXI interconnect and AXI 
                 * Datamover, being instantiated in VHDL.
                 * */
                lcl_mem0[ddr_addr_in++] = memP0.tdata;
                
                // When we have emulated the writting to lcl_mem0, we acknowledge with a P0 status 
                dmSts_MemWrStsP0.tag = 7;
                dmSts_MemWrStsP0.okay = 1;
                dmSts_MemWrStsP0.interr = 0;
                dmSts_MemWrStsP0.slverr = 0;
                dmSts_MemWrStsP0.decerr = 0;
                if (!sSHL_Rol_Mem_WrStsP0.full()) {
                    printf("DEBUG: Write a memory status command to SHELL/Mem/Mp0 \n");
                    sSHL_Rol_Mem_WrStsP0.write(dmSts_MemWrStsP0);
                }
            }

            

#endif // ENABLE_DDR
            
            
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
    if (!getOutputDataStream(sUAF_Shl_Data, "sUAF_Shl_Data", "ofsUAF_Shl_Data.dat", simCnt))
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
      //printf("DEBUG: i=%u\tTOT_TRANSFERS=%u\n", i, TOT_TRANSFERS);
      assert(i == TOT_TRANSFERS);
    }
    else {
      printf("Error No metadata received...\n");
      nrErr++;
    }
    
    //-------------------------------------------------------
    //-- STEP-5 : FROM THE OUTPUT FILE CREATE AN ARRAY
    //-------------------------------------------------------    
    if (!setInputFileToArray("ofsUAF_Shl_Data.dat", imgOutputArray, simCnt)) {
      printf("### ERROR : Failed to set input array from file \"ofsUAF_Shl_Data.dat\". \n");
      nrErr++;
    }
    xf::cv::Array2xfMat<OUTPUT_PTR_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPIX>(imgOutputArray, imgOutput);


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
    cornerHarrisAccelArray(imgInputArray, imgOutputArrayTb, in_img.rows, in_img.cols, Thresh, k);
    xf::cv::Array2xfMat<OUTPUT_PTR_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPIX>(imgOutputArrayTb, imgOutputTb);
        
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

    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPIX>* select_imgOutput;

    // Select which output you want to process for image outputs and corners comparisons:
    // &imgOutput   : The processed image by Harris IP inside the ROLE (i.e. I/O traffic is passing through SHELL)
    // &imgOutputTb : The processed image by Harris IP in this testbench (i.e. I/O traffic is done in testbench)
    select_imgOutput = &imgOutput;
 
    // Mark HLS points on the image 
    markPointsOnImage(*select_imgOutput, in_img, out_img, hls_points);
 
    // Write HLS and Opencv corners into a file 
    //nrErr += 
    writeCornersIntoFile(in_img, ocv_out_img, out_img, hls_points, ocv_points, common_pts);

    // Clear memory
    free(imgOutputArrayTb);
    free(imgOutputArray);
    free(imgInputArray);
    
    
    return(nrErr);
}




/*! \} */
