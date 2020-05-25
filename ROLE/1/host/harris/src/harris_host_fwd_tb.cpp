/*****************************************************************************
 * @file       harris_host_fwd_tb.cpp
 * @brief      Testbench for Harris userspace application for cF (x86, ppc64).
 *
 * @date       May 2020
 * @author     DID
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup HarrisTB
 * @addtogroup HarrisTB
 * \{
 *****************************************************************************/

#include <cstdlib>           // For atoi()
#include <iostream>          // For cout and cerr
#include "../include/PracticalSocket.h" // For UDPSocket and SocketException
#include "../include/config.h"
#include "opencv2/opencv.hpp"

using namespace cv;


  /**
   *   Main testbench for the user-application for Harris on host. Server
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {

    if ((argc < 2) || (argc > 3)) { // Test for correct number of parameters
        cerr << "Usage: " << argv[0] << " <Server Port> <optional simulation mode>" << endl;
        exit(1);
    }

    unsigned short servPort = atoi(argv[1]); // First arg:  local port

    namedWindow("tb_recv", WINDOW_AUTOSIZE);
    try {
        UDPSocket sock(servPort);

        char buffer[BUF_LEN]; // Buffer for echo string
        int recvMsgSize; // Size of received message
        string sourceAddress; // Address of datagram source
        unsigned short sourcePort; // Port of datagram source

        // RX Step
        clock_t last_cycle_rx = clock();
#ifdef INPUT_FROM_CAMERA
        while (1) {
            // Block until receive message from a client
#endif
    
	    int total_pack = 1 + (FRAME_WIDTH * FRAME_HEIGHT - 1) / PACK_SIZE;
            cout << "INFO: Expecting length of packs:" << total_pack << endl;
            char * longbuf = new char[PACK_SIZE * total_pack];
	    
	    // RX Loop
            for (int i = 0; i < total_pack; i++) {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
                if (recvMsgSize != PACK_SIZE) {
                    cerr << "ERROR: Received unexpected size pack:" << recvMsgSize << endl;
#ifdef INPUT_FROM_CAMERA
                    continue;
#else
		    exit(1);
#endif
                }
                memcpy( & longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
            }

            cout << "INFO: Received packet from " << sourceAddress << ":" << sourcePort << endl;
 
            cv::Mat frame = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC1, longbuf); // OR vec.data() instead of ptr
	    if (frame.size().width == 0) {
                cerr << "ERROR: receive failure!" << endl;
#ifdef INPUT_FROM_CAMERA
                continue;
#else
		exit(1);
#endif
            }
            imshow("tb_recv", frame);
	    
	    // We save the image received from network in order to process it with the harris HLS TB
	    imwrite("../../../hls/harris_app/test/input_from_udp_to_fpga.png", frame);
	    
	    // Select simulation mode, default fcsim
	    string exec_cmd = "make fcsim -j 4";
	    string ouf_file = "../../../hls/harris_app/harris_app_prj/solution1/fcsim/build/output_hls.png";
	    if (argc == 3) {
	      if (atoi(argv[2]) == 2) {
		exec_cmd = "make csim";
		ouf_file = "../../../hls/harris_app/harris_app_prj/solution1/csim/build/output_hls.png";
	      }
	      else if (atoi(argv[2]) == 3) {
		exec_cmd = "make cosim";
		ouf_file = "../../../hls/harris_app/harris_app_prj/solution1/cosim/build/output_hls.png";
	      }
	      else if (atoi(argv[2]) == 4) {
		exec_cmd = "make kcachegrind";
		ouf_file = "../../../hls/harris_app/harris_app_prj/solution1/fcsim/build/output_hls.png";
	      }
	    }
	    // Calling the actual TB over its typical makefile procedure, but passing the save file
	    string str_command = "cd ../../../hls/harris_app && make clean && \
				  INPUT_IMAGE=./test/input_from_udp_to_fpga.png " + exec_cmd + " && \
				  cd ../../host/harris/build/ "; 
	    const char *command = str_command.c_str(); 
  	    cout << "Calling TB with command:" << command << endl; 
	    system(command); 
	    
            free(longbuf);
	    	    	    	    
	    clock_t next_cycle_rx = clock();
            double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * total_pack / duration_rx / 1024 * 8) << endl;
            last_cycle_rx = next_cycle_rx;
    

	    // TX step
	    frame = cv::imread(ouf_file, cv::IMREAD_GRAYSCALE); // reading in the image in grey scale
	    if (!frame.data) {
	      cerr << "ERROR: Failed to load the image " << ouf_file << endl; 
	      return -1;
	    }
	    else {
	      cout << "INFO: Succesfully loaded image " << ouf_file << endl; 
	    }
	        
	    assert(frame.total() == FRAME_WIDTH * FRAME_HEIGHT);
	    
            imshow("tb_send", frame);
	    
	    // Ensure that the send Mat is in continuous memory space. Typically, imread or resize 
	    // will return such a continuous Mat, but we should check it.
	    assert(frame.isContinuous());
	    
	    // TX Loop
	    clock_t last_cycle_tx = clock();
            for (int i = 0; i < total_pack; i++) {
		sock.sendTo( & frame.data[i * PACK_SIZE], PACK_SIZE, sourceAddress, sourcePort);
		//sock.sendTo( & flat.data[i * PACK_SIZE], PACK_SIZE, sourceAddress, sourcePort);
	    }
            
            clock_t next_cycle_tx = clock();
            double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * total_pack / duration_tx / 1024 * 8) << endl;
            last_cycle_tx = next_cycle_tx;
	    
	    
#ifdef INPUT_FROM_CAMERA	    
        }
#endif
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }
    return 0;
}




/*! \} */