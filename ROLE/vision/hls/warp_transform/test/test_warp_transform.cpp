/*****************************************************************************
 * @file       : test_warp_transform.cpp
 * @brief      : Testbench for WarpTransform.
 *
 * System:     : cloudFPGA
 * Component   : Role
 * Language    : Vivado HLS
 *
 * Created: Nov 2021
 * Authors: FAB, WEI, NGL, DID, DCO
 * 
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2020 - IBM Research - All Rights Reserved.
 *
 * @ingroup WarpTransformTB
 * @addtogroup WarpTransformTB
 * \{
 *****************************************************************************/

#include "../include/warp_transform.hpp"
#include "../../common/src/common.cpp"
#include "../include/xf_warp_transform_config.h"

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

// The number of sequential testbench executions
#define TB_TRIALS   3

// Enable delay in the response channel of DDR AXI controller
#define ENABLE_DDR_EMULATE_DELAY_IN_TB

#define ENABLED     (ap_uint<1>)1
#define DISABLED    (ap_uint<1>)0


#if TRANSFORM_TYPE == 1
#define TRMAT_DIM2 3
#define TRMAT_DIM1 3
#else
#define TRMAT_DIM2 3
#define TRMAT_DIM1 2
#endif

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
stream<UdpWord>             image_stream_from_warp_transform ("image_stream_from_warp_transform");

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
#define MEMORY_LINES_512 TOTMEMDW_512 /* 64 KiB */
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
    warp_transform(
        &node_rank, &cluster_size,
        sSHL_Uaf_Data, sUAF_Shl_Data,
        siUdp_meta, soUdp_meta,
        &s_udp_rx_ports
        #ifdef ENABLE_DDR
                        ,
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
    int nrErr;
    unsigned int tb_trials = 0;

    printf("#####################################################\n");
    printf("## MAIN TESTBENCH STARTS HERE                      ##\n");
    printf("#####################################################\n");

    
    //------------------------------------------------------
    //-- TESTBENCH LOCAL VARIABLES FOR WARPTRANSFORM
    //------------------------------------------------------
    cv::Mat in_img;
    // float identity_tx_mat [9] = {1,0,0,0,1,0,0,0,0};
    // float xtranslation_tx_mat [9] = {1,0,2,0,1,0,0,0,0};// 1 0 vx 0 1 vy 000
    // float ytranslation_tx_mat [9] = {1,0,0,0,1,2,0,0,0}; 
    // float reflection_tx_mat [9] = {-1,0,0,0,1,0,0,0,0};
    // float yscale_tx_mat [9] = {2,0,0,0,1,0,0,0,0}; //cx  0 0 0 cy 0 000
    // float xscale_tx_mat [9] = {1,0,0,0,2,0,0,0,0};
    // float rotation_30degree_tx_mat [9] = {0.87,-0.5,0,0.5,0.87,0,0,0,0}; //cos -sin 0 sin cos 0 000
    // float shearing_tx_mat [9] = {1,0.5,0,0,1,0,0,0,0}; //1 cx 0 cy 1 0 000
    //float transformation_matrix_float [9] = {1,0,0,0,-1,256,0,0,0};// reflect wrt x
    float transformation_matrix_float [9] = {1.5,0,0,0,1.8,0,0,0,0};
    cv::Mat transformation_matrix(TRMAT_DIM1, TRMAT_DIM2, CV_32FC1, transformation_matrix_float);
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
    unsigned int ddr_write_req_iter = 0;
    unsigned int wait_cycles_to_ack_ddr_status = 0;
    unsigned int count_cycles_to_ack_ddr_status = 0;
    bool ddr_write_sts_req = false;
    #endif
    

    //------------------------------------------------------
    //-- STEP-1.1 : CREATE MEMORY FOR OUTPUT IMAGES
    //------------------------------------------------------
    //	cvtColor(in_img, img_gray, CV_BGR2GRAY);
    // Convert rgb into grayscale
    hls_out_img.create(in_img.rows, in_img.cols, CV_8U); // create memory for hls output image
    ocv_out_img.create(in_img.rows, in_img.cols, CV_8U); // create memory for opencv output image

    #if NO
    static xf::cv::Mat<TYPE, HEIGHT, WIDTH, XF_NPPC1> imgInput(in_img.rows, in_img.cols);
    static xf::cv::Mat<TYPE, HEIGHT, WIDTH, XF_NPPC1> imgOutput(in_img.rows, in_img.cols);
    static xf::cv::Mat<TYPE, HEIGHT, WIDTH, XF_NPPC1> imgOutputTb(in_img.rows, in_img.cols);
    imgInput.copyTo(in_img.data);
    ap_uint<INPUT_PTR_WIDTH>  *imgInputArray    = (ap_uint<INPUT_PTR_WIDTH>*)  malloc(in_img.rows * in_img.cols * sizeof(ap_uint<INPUT_PTR_WIDTH>));
    ap_uint<OUTPUT_PTR_WIDTH> *imgOutputArrayTb = (ap_uint<OUTPUT_PTR_WIDTH>*) malloc(in_img.rows * in_img.cols * sizeof(ap_uint<OUTPUT_PTR_WIDTH>));
    ap_uint<OUTPUT_PTR_WIDTH> *imgOutputArray   = (ap_uint<OUTPUT_PTR_WIDTH>*) malloc(in_img.rows * in_img.cols * sizeof(ap_uint<OUTPUT_PTR_WIDTH>));
    xf::cv::xfMat2Array<INPUT_PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPIX>(imgInput, imgInputArray);
    #endif

    #if RO
    static xf::cv::Mat<TYPE, HEIGHT, WIDTH, XF_NPPC8> imgInput(in_img.rows, in_img.cols);
    static xf::cv::Mat<TYPE, HEIGHT, WIDTH, XF_NPPC8> imgOutput(in_img.rows, in_img.cols);
    static xf::cv::Mat<TYPE, HEIGHT, WIDTH, XF_NPPC1> imgOutputTb(in_img.rows, in_img.cols);
    #endif

    // L2 Vitis WarpTransform
    warptTransformAccelArray(imgInputArray, transformation_matrix_float,imgOutputArrayTb, in_img.rows, in_img.cols);
    xf::cv::Array2xfMat<OUTPUT_PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPIX>(imgOutputArrayTb, imgOutputTb);
    if ( !dumpImgToFile ( imgOutputTb, "verify_UAF_Shl_Data.dat", simCnt) ) {
        nrErr++;
    }
    
    while (tb_trials++ < TB_TRIALS) {
   

        printf ( "##################################################### \n" );
        printf ( "## TESTBENCH #%u STARTS HERE                        ##\n", tb_trials );
        printf ( "##################################################### \n" );


        simCnt = 0;
        nrErr  = 0;

#if NO
        if ( !dumpImgToFileWarpTransform ( imgInput, "ifsSHL_Uaf_Data.dat", simCnt, transformation_matrix_float ) ) {
            nrErr++;
        }
#endif

#if RO
        // imgInput.copyTo(img_gray.data);
        imgInput = xf::cv::imread<TYPE, HEIGHT, WIDTH, XF_NPPC8> ( argv[1], 0 );
#endif



        //------------------------------------------------------
        //-- STEP-1.2 : RUN WARPTRANSFORM DETECTOR FROM OpenCV LIBRARY
        //------------------------------------------------------
        // ksize: aperture linear size; it must be odd and greater than 1, for example: 3, 5, 7 ...
        //int ksize = WINDOW_SIZE ;
        ocv_ref ( in_img, ocv_out_img, transformation_matrix);



        //------------------------------------------------------
        //-- STEP-2.1 : CREATE TRAFFIC AS INPUT STREAMS
        //------------------------------------------------------
        if ( nrErr == 0 ) {
            if ( !setInputDataStream ( sSHL_Uaf_Data, "sSHL_Uaf_Data", "ifsSHL_Uaf_Data.dat", simCnt ) ) {
                printf ( "### ERROR : Failed to set input data stream \"sSHL_Uaf_Data\". \n" );
                nrErr++;
            }

            //there are TOT_TRANSFERS streams from the the App to the Role
            NetworkMeta tmp_meta = NetworkMeta ( 1,DEFAULT_RX_PORT,0,DEFAULT_RX_PORT,0 );
            for ( int i=0; i<TOT_TRANSFERS; i++ ) {
                siUdp_meta.write ( NetworkMetaStream ( tmp_meta ) );
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
        while ( !nrErr ) {

            // Keep enough simulation time for sequntially executing the FSMs of the main 3 functions
            // (Rx-Proc-Tx)
            if ( simCnt < MIN_RX_LOOPS + MIN_RX_LOOPS + MIN_TX_LOOPS + 10 + 2 + 4
#ifdef ENABLE_DDR
#ifdef ENABLE_DDR_EMULATE_DELAY_IN_TB 
                + (TYPICAL_DDR_LATENCY + EXTRA_DDR_LATENCY_DUE_II + DDR_LATENCY) * MEMORY_LINES_512
#endif
#endif
               ) {
                stepDut();

                if ( simCnt > 2 ) {
                    assert ( s_udp_rx_ports == PORTS_OPENED );
                }

if (simCnt < 0)
    exit(0);
#ifdef ENABLE_DDR

                if ( !sROL_Shl_Mem_WrCmdP0.empty() ) {
                    printf ( "DEBUG tb: Read a memory write command from SHELL/Mem/Mp0 \n" );
                    //-- Read a memory write command from SHELL/Mem/Mp0
                    sROL_Shl_Mem_WrCmdP0.read ( dmCmd_MemCmdP0 );
                    //assert ( dmCmd_MemCmdP0.bbt == CHECK_CHUNK_SIZE );
                    assert ( dmCmd_MemCmdP0.type == 1 && dmCmd_MemCmdP0.dsa == 0 && dmCmd_MemCmdP0.eof == 1 && dmCmd_MemCmdP0.drr == 1 && dmCmd_MemCmdP0.tag == 0x0 );
                    ddr_addr_in = (unsigned int)dmCmd_MemCmdP0.saddr / BPERMDW_512; // Convert the byte-aligned address to local mem of stack tb.
                    printf ( "DEBUG tb: Requesting writting to address %u (max depth = %u) an amount of %u bytes (%u memory lines), ddr_write_req_iter=%u\n", ddr_addr_in,  MEMORY_LINES_512-1, (unsigned int)dmCmd_MemCmdP0.bbt, (unsigned int)(1 + (dmCmd_MemCmdP0.bbt - 1) / BPERMDW_512), ddr_write_req_iter);
                    assert (ddr_addr_in <= MEMORY_LINES_512-1);
                    //ddr_write_req_iter++;
                    //printf ( "DEBUG tb: (ddr_write_req_iter)%(MEMORY_LINES_512-1)=%u\n", (ddr_write_req_iter)%(MEMORY_LINES_512-1));
                    if ((++ddr_write_req_iter)%(MEMORY_LINES_512) == 0) {
                        ddr_write_req_iter = 0;
                    }
                    printf ( "DEBUG tb: ddr_write_req_iter=%u\n", ddr_write_req_iter);

#ifdef ENABLE_DDR_EMULATE_DELAY_IN_TB
                    /*
                     * 16                   ->  emulate a response in 16 cycles
                     * 1                    ->  emulate immediate response
                     * CYCLES_UNTIL_TIMEOUT ->  on purpose timeout
                     */ 
                    if (ddr_write_req_iter == 1) {
                        wait_cycles_to_ack_ddr_status = TYPICAL_DDR_LATENCY;
                    }
                    else if (ddr_write_req_iter == 2) {
                        wait_cycles_to_ack_ddr_status = TYPICAL_DDR_LATENCY;
                    }
                    else {
                        wait_cycles_to_ack_ddr_status = TYPICAL_DDR_LATENCY;
                    }
                    if (!sSHL_Rol_Mem_WrStsP0.empty()) {
                        printf("WARNING: Emptying sSHL_Rol_Mem_WrStsP0 fifo.\n");
                        dmSts_MemWrStsP0  = sSHL_Rol_Mem_WrStsP0.read();
                    }
#else
                    wait_cycles_to_ack_ddr_status = 0;
#endif
                    count_cycles_to_ack_ddr_status = 0;
                    ddr_write_sts_req = false;
                }

                if ( !sROL_Shl_Mem_WriteP0.empty() ) {
                    sROL_Shl_Mem_WriteP0.read ( memP0 );
                    printf ( "DEBUG tb: Write a memory line from SHELL/Mem/Mp0 \n" );

                    assert ( memP0.tkeep == 0xffffffffffffffff );

                    /* Read from AXI stream DDR interface and write to the memory mapped interface of
                     * DDR channel P0. In the real HW, this is enabled by the AXI interconnect and AXI
                     * Datamover, being instantiated in VHDL.
                     * */
                    // printf ( "DEBUG tb: Writting to address 0x%x : %u an amount of %u bytes\n", ddr_addr_in, memP0.tdata.to_long(), BPERMDW_512);
                    std::cout << "DEBUG tb: Writting to address 0x" << std::hex << ddr_addr_in << " : " << memP0.tdata << " an amount of " << std::dec << BPERMDW_512 << " bytes" << std::endl;
                    //lcl_mem0[ddr_addr_in++] = memP0.tdata;
                    memcpy(&lcl_mem0[ddr_addr_in++], &memP0.tdata, BPERMDW_512);
                    ddr_write_sts_req = true;
                }
                // When we have emulated the writting to lcl_mem0, we acknowledge with a P0 status
                if ((ddr_write_sts_req == true) && !sSHL_Rol_Mem_WrStsP0.full() && (memP0.tlast == true)) {
                    if (count_cycles_to_ack_ddr_status++ == wait_cycles_to_ack_ddr_status) {
                        dmSts_MemWrStsP0.tag = 0;
                        dmSts_MemWrStsP0.okay = 1;
                        dmSts_MemWrStsP0.interr = 0;
                        dmSts_MemWrStsP0.slverr = 0;
                        dmSts_MemWrStsP0.decerr = 0;
                        printf ( "DEBUG tb: Write a memory status command to SHELL/Mem/Mp0 \n" );
                            sSHL_Rol_Mem_WrStsP0.write ( dmSts_MemWrStsP0 );
                            ddr_write_sts_req = false;
                    }
                    else{
                        printf ( "DEBUG tb: Waiting to write a memory status command to SHELL/Mem/Mp0 [%u out of %u] cycles\n", count_cycles_to_ack_ddr_status, wait_cycles_to_ack_ddr_status);
                    }
                }



#endif // ENABLE_DDR


                //if( !soUdp_meta.empty())
                //{
                //  NetworkMetaStream tmp_meta = soUdp_meta.read();
                //  printf("NRC received NRCmeta stream from node_rank %d.\n", (int) tmp_meta.tdata.src_rank);
                //}


            } else {
#ifdef ENABLE_DDR
                if (!sSHL_Rol_Mem_WrStsP0.empty()) {
                    printf("WARNING: Emptying sSHL_Rol_Mem_WrStsP0 fifo.\n");
                    dmSts_MemWrStsP0  = sSHL_Rol_Mem_WrStsP0.read();
                }
#endif
                printf ( "## End of simulation at cycle=%3d. \n", simCnt );
                break;
            }

        }  // End: while()

        //-------------------------------------------------------
        //-- STEP-4 : DRAIN AND WRITE OUTPUT FILE STREAMS
        //-------------------------------------------------------
        //---- UAF-->SHELL Data ----
        if ( !getOutputDataStream ( sUAF_Shl_Data, "sUAF_Shl_Data", "ofsUAF_Shl_Data.dat", simCnt ) ) {
            nrErr++;
        }
        //---- UAF-->SHELL META ----
        if ( !soUdp_meta.empty() ) {
            int i = 0;
            while ( !soUdp_meta.empty() ) {
                i++;
                NetworkMetaStream tmp_meta = soUdp_meta.read();
                printf ( "NRC received NRCmeta stream from rank %d to rank %d.\n", ( int ) tmp_meta.tdata.src_rank, ( int ) tmp_meta.tdata.dst_rank );
                assert ( tmp_meta.tdata.src_rank == node_rank );
                //ensure forwarding behavior
                assert ( tmp_meta.tdata.dst_rank == ( ( tmp_meta.tdata.src_rank + 1 ) % cluster_size ) );
            }
            //printf("DEBUG: i=%u\tTOT_TRANSFERS=%u\n", i, TOT_TRANSFERS);
            assert ( i == TOT_TRANSFERS );
        } else {
            printf ( "Error No metadata received...\n" );
            nrErr++;
        }

        //-------------------------------------------------------
        //-- STEP-5 : FROM THE OUTPUT FILE CREATE AN ARRAY
        //-------------------------------------------------------
        if ( !setInputFileToArray ( "ofsUAF_Shl_Data.dat", imgOutputArray, simCnt ) ) {
            printf ( "### ERROR : Failed to set input array from file \"ofsUAF_Shl_Data.dat\". \n" );
            nrErr++;
        }
        xf::cv::Array2xfMat<OUTPUT_PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPIX> ( imgOutputArray, imgOutput );


        //------------------------------------------------------
        //-- STEP-6 : COMPARE INPUT AND OUTPUT FILE STREAMS
        //------------------------------------------------------
        int rc1 = system ( "diff --brief -w -i -y ../../../../test/ofsUAF_Shl_Data.dat \
                                            ../../../../test/verify_UAF_Shl_Data.dat" );
        if ( rc1 ) {
            printf ( "## Error : File \'ofsUAF_Shl_Data.dat\' does not match \'verify_UAF_Shl_Data.dat\'.\n" );
        } else {
            printf ( "Output data in file \'ofsUAF_Shl_Data.dat\' verified.\n" );
        }
        const string outfilename = "hls_out-"+std::to_string(tb_trials)+".jpg";
        xf::cv::imwrite(outfilename.c_str(), imgOutput);

        nrErr += rc1;

        printf ( "#####################################################\n" );
        if ( nrErr ) {
            printf ( "## ERROR - TESTBENCH #%u FAILED (RC=%d) !!!        ##\n", tb_trials, nrErr );
        } else {
            printf ( "## SUCCESSFULL END OF TESTBENCH #%u (RC=0)          ##\n", tb_trials );
        }
        printf ( "#####################################################\n" );





    } // End tb trials while loop







    /**************		HLS Function	  *****************/

    // #if NO

    // // L2 Vitis WarpTransform
    // warptTransformAccelArray(imgInputArray, transformation_matrix_float,imgOutputArrayTb, in_img.rows, in_img.cols);
    // xf::cv::Array2xfMat<OUTPUT_PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPIX>(imgOutputArrayTb, imgOutputTb);
        
    // // L1 Vitis WarpTransform 
    // //warp_transform_accel(imgInput, imgOutput, Thresh, k);
	
    // #endif

    // #if RO

    // warp_transform_accel(imgInput, imgOutputTb);

    // #endif

    /// hls_out_img.data = (unsigned char *)imgOutput.copyFrom();
    xf::cv::imwrite("hls_out_tb.jpg", imgOutputTb);
    xf::cv::imwrite("hls_out.jpg", imgOutput);
    cv::imwrite("ocv_ref_out.jpg", ocv_out_img);

    unsigned int val;
    unsigned short int row, col;

    cv::Mat out_img;
    out_img = in_img.clone();

    std::vector<cv::Point> hls_points;
    std::vector<cv::Point> ocv_points;
    std::vector<cv::Point> common_pts;

    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPIX>* select_imgOutput;

    // Select which output you want to process for image outputs and corners comparisons:
    // &imgOutput   : The processed image by WarpTransform IP inside the ROLE (i.e. I/O traffic is passing through SHELL)
    // &imgOutputTb : The processed image by WarpTransform IP in this testbench (i.e. I/O traffic is done in testbench)
    select_imgOutput = &imgOutput;
 
    // Clear memory
    free(imgOutputArrayTb);
    free(imgOutputArray);
    free(imgInputArray);
    
    
    return(nrErr);
}




/*! \} */
