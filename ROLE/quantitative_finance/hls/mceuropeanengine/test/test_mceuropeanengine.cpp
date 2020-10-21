/*****************************************************************************
 * @file       test_mceuropeanengine.cpp
 * @brief      Testbench for MCEuropeanEngine
 *
 * @date April 2020
 * @authors    FAB, WEI, NGL, DID
 * 
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2020 - IBM Research - All Rights Reserved.
 *
 * @ingroup MCEuropeanEngineTB
 * @addtogroup MCEuropeanEngineTB
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

ap_uint<32>                 s_udp_rx_ports = 0x0;
stream<NetworkMetaStream>   siUdp_meta          ("siUdp_meta");
stream<NetworkMetaStream>   soUdp_meta          ("soUdp_meta");
ap_uint<32>                 node_rank;
ap_uint<32>                 cluster_size;

//------------------------------------------------------
//-- TESTBENCH GLOBAL VARIABLES
//------------------------------------------------------
unsigned int simCnt;


/*****************************************************************************
 * @brief Run a single iteration of the DUT model.
 * @return Nothing.
 ******************************************************************************/
void stepDut() {
    mceuropeanengine(
      &node_rank, &cluster_size,
      sSHL_Uaf_Data, sUAF_Shl_Data,
      siUdp_meta, soUdp_meta,
      &s_udp_rx_ports);
    simCnt++;
    printf("[%4.4d] STEP DUT \n", simCnt);
}



/*****************************************************************************
 * @brief Main testbench of MCEuropeanEngine.
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

    if (argc != 2) {
        printf("Usage : %s <configuration file>. Provided %d\n", argv[0], argc);
        return -1;
    }
    
    // Ensure that the selection of MTU is a multiple of 8 (Bytes per transaction)
    assert(PACK_SIZE % 8 == 0);
    
    //------------------------------------------------------
    //-- TESTBENCH LOCAL VARIABLES FOR MCEUROPEANENGINE
    //------------------------------------------------------
    varin instruct;
    const char *fname = argv[1]; //"../../../../etc/mce.conf";
    if (readFileConfigToStruct(fname, &instruct) != INSIZE) {
        printf("WARNING: Invalid read size of configration file %s. Will use default...\n", fname);
	instruct.loop_nm = OUTDEP;    
	instruct.seed = 4332 ; // 441242, 42, 13342;
	instruct.underlying = 36;
	instruct.volatility = 0.20;
	instruct.dividendYield = 0.0;
	instruct.riskFreeRate = 0.06;
	instruct.timeLength = 1;
	instruct.strike = 40;
	instruct.optionType = 1;
	instruct.requiredTolerance = 0.02;
	instruct.requiredSamples = 1; // 262144; // 48128;//0;//1024;//0;
	instruct.timeSteps = 1;
	instruct.maxSamples = 1;
    }
    
    if ((instruct.loop_nm == 0) || (instruct.loop_nm > OUTDEP)) {
	    printf("WARNING tb Invalid instruct->loop_nm = %u. Will assign %u\n", (unsigned int)instruct.loop_nm, OUTDEP);
	    instruct.loop_nm = OUTDEP;
    }
	  
    unsigned int sim_time = MIN_RX_LOOPS + MIN_TX_LOOPS + 10;
    unsigned int tot_trasnfers_in  = TOT_TRANSFERS_IN;
    unsigned int tot_trasnfers_out = TOT_TRANSFERS_OUT;
    
    DtUsed *out = (DtUsed*)malloc(instruct.loop_nm * sizeof(DtUsed));
    if (!out) {
        printf("ERROR: Cannot allocate memory for output array. Aborting...\n");
        return -1;
    }
    
    
    //------------------------------------------------------
    //-- STEP-1.1 : CREATE FILE FROM INPUT CONFIGURATION
    //------------------------------------------------------
    if (!dumpStructToFile(&instruct, "ifsSHL_Uaf_Data.dat", simCnt)) {
      nrErr++;
    }

    //------------------------------------------------------
    //-- STEP-2.1 : CREATE TRAFFIC AS INPUT STREAM
    //------------------------------------------------------
    if (nrErr == 0) {
        if (!setInputDataStream(sSHL_Uaf_Data, "sSHL_Uaf_Data", "ifsSHL_Uaf_Data.dat", simCnt)) { 
            printf("### ERROR : Failed to set input data stream \"sSHL_Uaf_Data\". \n");
            nrErr++;
        }

        //there are tot_trasnfers_in streams from the the App to the Role
        NetworkMeta tmp_meta = NetworkMeta(1,DEFAULT_RX_PORT,0,DEFAULT_RX_PORT,0);
	for (unsigned int i = 0; i < tot_trasnfers_in; i++) {
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
        // (Rx-Tx)
        if (simCnt < sim_time) 
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
    if (!getOutputDataStream(sUAF_Shl_Data, "sUAF_Shl_Data", "ofsUAF_Shl_Data.dat", simCnt))
    {
        nrErr++;
    }
    //---- UAF-->SHELL META ----
    if( !soUdp_meta.empty())
    {
      unsigned int i = 0;
      while( !soUdp_meta.empty())
      {
        i++;
        NetworkMetaStream tmp_meta = soUdp_meta.read();
        printf("NRC received NRCmeta stream from rank %d to rank %d.\n", (int) tmp_meta.tdata.src_rank, (int) tmp_meta.tdata.dst_rank);
        assert(tmp_meta.tdata.src_rank == node_rank);
        //ensure forwarding behavior
        assert(tmp_meta.tdata.dst_rank == ((tmp_meta.tdata.src_rank + 1) % cluster_size));
      }
      //printf("i=%u, tot_trasnfers_in=%u, tot_trasnfers_out=%u\n", i,tot_trasnfers_in, tot_trasnfers_out);
      assert(i == tot_trasnfers_out);
    }
    else {
      printf("Error No metadata received...\n");
      nrErr++;
    }
    
    //-------------------------------------------------------
    //-- STEP-5 : FROM THE OUTPUT FILE CREATE AN ARRAY
    //------------------------------------------------------- 
    
    if (!dumpFileToArray("ofsUAF_Shl_Data.dat", out, simCnt)) {
      printf("### ERROR : Failed to set array from file \"ofsUAF_Shl_Data.dat\". \n");
      nrErr++;
    }
    writeArrayToFile("./hls_out.txt", out);
    for (unsigned int i = 0; i < instruct.loop_nm; i++) {
	printf("Option price %u: %f\n", i, out[i]);
    }
    //------------------------------------------------------
    //-- STEP-6 : COMPARE OUTPUT AND GOLDEN FILE STREAMS
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


    return(nrErr);
}




/*! \} */