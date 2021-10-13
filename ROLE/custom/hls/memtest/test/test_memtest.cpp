/*****************************************************************************
 * @file       : test_memtest.cpp
 * @brief      : Testbench for Memtest.
 *
 * System:     : cloudFPGA
 * Component   : Role
 * Language    : Vivado HLS
 *
 * Created: September 2021
 * Authors: FAB, WEI, NGL, DID, DCO
 * 
 * Copyright 2009-2015 - Xilinx Inc.  - All rights reserved.
 * Copyright 2015-2021 - IBM Research - All Rights Reserved.
 *
 * @ingroup MemtestTB
 * @addtogroup MemtestTB
 * \{
 *****************************************************************************/

#include "../../common/src/common.cpp"
#include <math.h>

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
#define DEBUG_MULTI_RUNS True
#define TB_MULTI_RUNS_ITERATIONS 5
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
stream<UdpWord>             image_stream_from_memtest ("image_stream_from_memtest");

ap_uint<32>                 s_udp_rx_ports = 0x0;
stream<NetworkMetaStream>   siUdp_meta          ("siUdp_meta");
stream<NetworkMetaStream>   soUdp_meta          ("soUdp_meta");
ap_uint<32>                 node_rank;
ap_uint<32>                 cluster_size;

//------------------------------------------------------
//-- TESTBENCH GLOBAL VARIABLES
//------------------------------------------------------
unsigned int         simCnt;


/*****************************************************************************
 * @brief Run a single iteration of the DUT model.
 * @return Nothing.
 ******************************************************************************/
void stepDut() {
    memtest(
        &node_rank, &cluster_size,
      sSHL_Uaf_Data, sUAF_Shl_Data,
      siUdp_meta, soUdp_meta,
      &s_udp_rx_ports);
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


    if (argc < 3 || argc > 4) {
        printf("Usage : %s <number of address to test> , <testing times> , <command string ready2go> ;provided %d\n", argv[0], argc);
        return -1;
    }

    string strInput_memaddrUT = argv[1];
    string strInput_nmbrTest = argv[2];
    string strInput_commandstring= "";
    cout << argc << " argc was, and argv "<< argv << endl;
    if(argc > 3)
    {
      strInput_commandstring.assign(argv[3]);
    }
    //printStringHex(strInput_commandstring, strInput_commandstring.length());
    unsigned int memory_addr_under_test=0;
    unsigned int testingNumber = 1;
    // string tmp_string= argv[1];
    // string strInput;

    // //clean the corners if make or other utilities insert this weird ticks at the beginning of the string
    // if(isCornerPresent(tmp_string,"'") or isCornerPresent(tmp_string,"`")){
	  //   tmp_string = tmp_string.substr(1,tmp_string.length()-2);
    // }
    // cout << hex << tmp_string << dec << endl;
    // //perform hex2ascii conversion so that we can codify our command as we wish, no restriction to null ascii characters
    // hex2ascii(tmp_string, strInput);
    // if(isCornerPresent(strInput,"'") or isCornerPresent(strInput,"`")){
	  //   strInput = strInput.substr(1,strInput.length()-2);
    // }
     //cout << hex << strInput_memaddrUT << dec << endl;
     //cout << hex << strInput_nmbrTest << dec << endl;
    if (!strInput_memaddrUT.length() || !strInput_nmbrTest.length()) {
        printf("ERROR: Empty string provided. Aborting...\n");
        return -1;
    }
    else {
      try
      {
       memory_addr_under_test = stoul(strInput_memaddrUT);
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
        memory_addr_under_test = 65; //at least a fault
      }
      try
      {     
        testingNumber = stoul(strInput_nmbrTest);
      }
      catch(const std::exception& e)
      {
        std::cerr << e.what() << '\n';
        testingNumber = 3; //at least see the fault
      }
      

      printf("Succesfully loaded string ... %s\n", argv[1]);
      printf("Succesfully loaded the address number %u and the number of testings %u\n", memory_addr_under_test, testingNumber);
      // Ensure that the selection of MTU is a multiple of 8 (Bytes per transaction)
      assert(PACK_SIZE % 8 == 0);
    }
    //------------------------------------------------------
    //-- TESTBENCH LOCAL VARIABLES FOR MEMTEST
    //------------------------------------------------------
    unsigned int sim_time = testingNumber * ((2 * (memory_addr_under_test+2)) + 4) + 2 + 10; // # of tests*((2*(rd/wr addresses + 1 state update))+start+out) + 10 random cycles
    size_t charInputSize = 8; //a single tdata
    size_t charOutputSize = 8*1+((8 * (2 + 1 + 1)) * testingNumber); //stop, 4 for each test, potential stop?

    unsigned int tot_input_transfers = CEIL(( 1 ) * 8,PACK_SIZE);//(CEIL( ((testingNumber * (2 * (memory_addr_under_test+1)) + 2) + 2 )* 8, PACK_SIZE)); // only a single tx
    unsigned int tot_output_transfers =  1;// (CEIL(8 * (2 + 1 + 1) * testingNumber, PACK_SIZE)); //  only 3 rx packets of 8 bytes each
    if(charOutputSize>PACK_SIZE){
      tot_output_transfers = charOutputSize%PACK_SIZE==0 ? charOutputSize/PACK_SIZE : charOutputSize/PACK_SIZE + 1;

      //tot_output_transfers = ceil(charOutputSize/PACK_SIZE);
    }
    cout << tot_output_transfers << " tx, outsize " << charOutputSize <<endl;


    //char *charOutput = (char*)malloc((charOutputSize+1)* sizeof(char)); // reading two 32 ints + others?
    char * charOutput = new char[(charOutputSize)* sizeof(char)]; // reading two 32 ints + others?
    //char *charInput = (char*)malloc(charInputSize* sizeof(char)); // at least print the inputs
    char * charInput = new char[(charInputSize)* sizeof(char)]; // at least print the inputs
    //char * longbuf= new char[PACK_SIZE+1];
    char tmp_char_cmd [1];
    tmp_char_cmd[1] = (char)0;
    unsigned int tmp_int_cmd = 0;

    if (!charOutput || !charInput) {
        printf("ERROR: Cannot allocate memory for output string. Aborting...\n");
        return -1;
    }
    std::vector<MemoryTestResult> testResults_vector;
    
    //------------------------------------------------------
    //-- STEP-1.1 : CREATE MEMORY FOR OUTPUT IMAGES
    //------------------------------------------------------
  unsigned int bytes_per_line = 8;
  string strInput;
  strInput.reserve(charInputSize+1);//="";
  string strGold;//="";
  strGold.reserve(charOutputSize+1);
  //="";
  //string longbuf_string;//="";
  //out_string.reserve(charOutputSize+1);

  
#ifdef SIM_STOP_COMPUTATION // stop the comptuation with a stop command after some CCs
  
  std::string strStop; 
	char stop_cmd [bytes_per_line];
	for (unsigned int k = 0; k < bytes_per_line; k++) {
		if (k != 0) {
			stop_cmd[k] = (char)0;
	    }
	    else {
			stop_cmd[k] = (char)2;
	    }
	 }
  strStop.append(stop_cmd,8);
#endif //SIM_STOP_COMPUTATION


    simCnt = 0;
    nrErr  = 0;
// Assumption: if the user knows how to format the command stream she/he does by itself (or it is because of the emulation flow :D)
// otheriwse the TB takes care and create the commands based on the inputs
    if(!strInput_commandstring.length()){
      strInput=createMemTestCommands(memory_addr_under_test, testingNumber);
    }else{
      ////////////////////////////////////////////////////////////
      //////TODO: this parsing is not working
      ////////////////////////////////////////////////////////////
      char * char_command = new char[strInput_commandstring.length()+1];
      char_command[0]='\0';//initi just for the sake
      //char_command = new char[strInput_commandstring.length()];
    //  cout << strInput_commandstring << endl;
    //   for (int i = 0; i < strInput_commandstring.length(); i++)
    //   {
    //     tmp_char_cmd[1] = strInput_commandstring[i];
    //     tmp_int_cmd = (unsigned int)atoi(tmp_char_cmd);
    //     memcpy(char_command+i,(char*)&tmp_int_cmd, sizeof(char));
    //     tmp_char_cmd[1] = (char)0;
    //     tmp_int_cmd = 0;
    //   }
    //   strInput.append(char_command,strInput_commandstring.length());
    //   printStringHex(strInput_commandstring, strInput_commandstring.length());
    //   printCharBuffHex(char_command, strInput_commandstring.length());
    //   printStringHex(strInput, strInput_commandstring.length());
    strInput=createMemTestCommands(memory_addr_under_test, testingNumber);
        if(char_command != NULL){
          cout << "Clearing the char command" << endl;
          delete[] char_command;
          char_command = NULL;
        }
    }
    //strInput[strInput.length()]='\0';
    strGold = createMemTestGoldenOutput(memory_addr_under_test, testingNumber, true);

    if (!dumpStringToFile(strInput, "ifsSHL_Uaf_Data.dat", simCnt)) {
      nrErr++;
    }
    //return 0;
    //the three is for setting the tlast to 1 every 3 commands to respect the current memtest pattern
    if (!dumpStringToFile(strGold, "verify_UAF_Shl_Data.dat", simCnt)){ 
      //, 3)) {
      nrErr++;
    }
#ifdef SIM_STOP_COMPUTATION // stop the comptuation with a stop command after some CCs

    if (!dumpStringToFile(strStop, "ifsSHL_Uaf_STOPData.dat", simCnt)){ 
      nrErr++;
    }
#endif // SIM_STOP_COMPUTATION
#ifdef DEBUG_MULTI_RUNS
for(int iterations=0; iterations < TB_MULTI_RUNS_ITERATIONS; iterations++){
#endif //DEBUG_MULTI_RUNS
    //------------------------------------------------------
    //-- STEP-2.1 : CREATE TRAFFIC AS INPUT STREAMS
    //------------------------------------------------------
    if (nrErr == 0) {
        if (!setInputDataStream(sSHL_Uaf_Data, "sSHL_Uaf_Data", "ifsSHL_Uaf_Data.dat", simCnt)) { 
            printf("### ERROR : Failed to set input data stream \"sSHL_Uaf_Data\". \n");
            nrErr++;
        }

        //there are tot_input_transfers streams from the the App to the Role
        NetworkMeta tmp_meta = NetworkMeta(1,DEFAULT_RX_PORT,0,DEFAULT_RX_PORT,0);
	for (unsigned int i=0; i<tot_input_transfers; i++) {
	  siUdp_meta.write(NetworkMetaStream(tmp_meta));
	}        
	//set correct node_rank and cluster_size
        node_rank = 1;
        cluster_size = 2;
    }
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
              //cout << "Verifying the assert, curr value of udp port: " << s_udp_rx_ports << endl;
              assert(s_udp_rx_ports == 0x1);
            }
#ifdef SIM_STOP_COMPUTATION // stop the comptuation with a stop command after some CCs
            if(simCnt == testingNumber * ((2 * (memory_addr_under_test+1))) + 2){
              if (!setInputDataStream(sSHL_Uaf_Data, "sSHL_Uaf_Data", "ifsSHL_Uaf_STOPData.dat", simCnt)) { 
              printf("### ERROR : Failed to set input data stream \"sSHL_Uaf_Data\". \n");
              nrErr++;
              }
              //there are tot_input_transfers streams from the the App to the Role
              NetworkMeta tmp_meta = NetworkMeta(1,DEFAULT_RX_PORT,0,DEFAULT_RX_PORT,0);
              for (unsigned int i=0; i<tot_input_transfers; i++) {
                siUdp_meta.write(NetworkMetaStream(tmp_meta));
              }        
              //set correct node_rank and cluster_size
                    node_rank = 1;
                    cluster_size = 2;
              }
#endif //SIM_STOP_COMPUTATION
              
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
      //printf("DEBUG %d received against %d predicted\n", i, tot_output_transfers);
      assert(i == tot_output_transfers);
    }
    else {
      printf("Error No metadata received...\n");
      nrErr++;
    }
    
    //-------------------------------------------------------
    //-- STEP-5 : FROM THE OUTPUT FILE CREATE AN ARRAY
    //------------------------------------------------------- 
    if (!dumpFileToString("ifsSHL_Uaf_Data.dat", charInput, simCnt)) {
      printf("### ERROR : Failed to set string from file \"ofsUAF_Shl_Data.dat\". \n");
      nrErr++;
    }
    printf("Input string : ");
    for (unsigned int i = 0; i <  charInputSize; i++)
       printf("%x", charInput[i]); 
    printf("\n");    
    if (!dumpFileToString("ofsUAF_Shl_Data.dat", charOutput, simCnt)) {
      printf("### ERROR : Failed to set string from file \"ofsUAF_Shl_Data.dat\". \n");
      nrErr++;
    }
    //__file_write("./hls_out.txt", charOutput, charOutputSize);
    //charOutput[charOutputSize+1]='\0';
    string out_string;
    out_string.reserve(charOutputSize);
    string tmpToDebug = string(charOutput,charOutputSize);
    out_string.append(tmpToDebug,0, charOutputSize);
		//printStringHex(out_string, charOutputSize);
    dumpStringToFileOnlyRawData(out_string, "./hls_out.txt", simCnt, charOutputSize);
    printf("Output string: ");
    for (unsigned int i = 0; i < charOutputSize; i++)
       printf("%x", charOutput[i]); 
    printf("\n");
  out_string.clear();
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
///////////////////////
////TODO: create an output parsing stuff
///////////////////////

  //longbuf = new char[PACK_SIZE];
  string ouf_file ="./hls_out.txt";
  int rawdatalines=0;
  string longbuf_string;
  longbuf_string.reserve(charOutputSize);
  longbuf_string = dumpFileToStringRawDataString(ouf_file, &rawdatalines, charOutputSize);
  //printCharBuffHex(longbuf, charOutputSize);
  //
  //printStringHex(longbuf_string, charOutputSize);
  //
  //cout << longbuf << endl;
  //longbuf[charOutputSize+1]='\0';
  //string longbuf_string(longbuf);
  testResults_vector=parseMemoryTestOutput(longbuf_string,charOutputSize,rawdatalines);
  longbuf_string.clear();
  testResults_vector.clear();
  cout<< endl << "  Going to close the TB with iteration " << iterations << endl << endl;

  nrErr += rc1;

  printf("#####################################################\n");
  if (nrErr) 
  {
      printf("## ERROR - TESTBENCH FAILED (RC=%d) !!!             ##\n", nrErr);
  } else {
      printf("## SUCCESSFULL END OF TESTBENCH (RC=0)             ##\n");
  }
  printf("#####################################################\n");
  //strGold="";
  //strInput="";
  simCnt = 0;

#ifdef DEBUG_MULTI_RUNS
}
 #endif//DEBUG_MULTI_RUNS
//strInput.clear();
//strGold.clear();
 //safe deletion
// if(longbuf != NULL){
//  cout << "Clearing the longbuf" << endl;
//  delete[] longbuf;
//  longbuf = NULL;
// }

if(charOutput != NULL){
 cout << "Clearing the char output" << endl;
 delete[] charOutput;
 charOutput = NULL;
}
if(charInput != NULL){
 cout << "Clearing the char input" << endl;
  delete[] charInput;
  charInput = NULL;
}
  return(nrErr);
}



/*! \} */
