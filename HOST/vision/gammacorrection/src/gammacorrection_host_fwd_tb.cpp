/*****************************************************************************
 * @file       gammacorrection_host_fwd_tb.cpp
 * @brief      Testbench for Gammacorrection userspace application for cF (x86, ppc64).
 *
 * @date       May 2020
 * @author     DID
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup GammacorrectionTB
 * @addtogroup GammacorrectionTB
 * \{
 *****************************************************************************/

#include <cstdlib>           // For atoi()
#include <iostream>          // For cout and cerr
#include "../../../PracticalSockets/src/PracticalSockets.h"
#include "../include/config.h"
#include "opencv2/opencv.hpp"

using namespace cv;


  /**
   *   Main testbench for the user-application for Gammacorrection on host. Server
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {

    if ((argc < 2) || (argc > 3)) { // Test for correct number of parameters
        cerr << "Usage: " << argv[0] << " <Server Port> <optional simulation mode>" << endl;
        exit(1);
    }

    unsigned short servPort = atoi(argv[1]); // First arg:  local port
    unsigned int num_frame = 0;
    string clean_cmd;
	    
    namedWindow("tb_recv", WINDOW_AUTOSIZE);
    try {
        UDPSocket sock(servPort);

        char buffer[BUF_LEN]; // Buffer for echo string
        int recvMsgSize; // Size of received message
        string sourceAddress; // Address of datagram source
        unsigned short sourcePort; // Port of datagram source

        // RX Step
        clock_t last_cycle_rx = clock();
        while (1) {
            // Block until receive message from a client
    
	    int total_pack = 1 + (FRAME_TOTAL - 1) / PACK_SIZE;
            int bytes_in_last_pack = (FRAME_TOTAL) - (total_pack - 1) * PACK_SIZE;	    
	    int receiving_now = PACK_SIZE;
            cout << " ___________________________________________________________________ " << endl;
            cout << "/                                                                   \\" << endl;
	    cout << "INFO: Proxy tb Frame # " << ++num_frame << endl;	    
            cout << "INFO: Expecting length of packs:" << total_pack << endl;
            char * longbuf = new char[PACK_SIZE * total_pack];
	    
	    // RX Loop
            for (int i = 0; i < total_pack; i++) {
	        if ( i == total_pack - 1 ) {
                    receiving_now = bytes_in_last_pack;
                }
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
                if (recvMsgSize != receiving_now) {
                    cerr << "ERROR: Received unexpected size pack:" << recvMsgSize << endl;
                    continue;
                }
                memcpy( & longbuf[i * PACK_SIZE], buffer, receiving_now);
            }

            cout << "INFO: Received packet from " << sourceAddress << ":" << sourcePort << endl;
 
            cv::Mat frame = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, INPUT_TYPE_HOST, longbuf); // OR vec.data() instead of ptr
	    if (frame.size().width == 0) {
                cerr << "ERROR: receive failure!" << endl;
                continue;
            }
            imshow("tb_recv", frame);
	    
	    // We save the image received from network in order to process it with the gammacorrection HLS TB
	    imwrite("../../../../ROLE/vision/hls/gammacorrection/test/input_from_udp_to_fpga.png", frame);
	    
	    // Select simulation mode, default fcsim
	    string exec_cmd = "make fcsim -j 4";
	    string ouf_file = "../../../../ROLE/vision/hls/gammacorrection/gammacorrection_prj/solution1/fcsim/build/hls_out.jpg";
	    if (argc == 3) {
	      if (atoi(argv[2]) == 2) {
		exec_cmd = "make csim";
		ouf_file = "../../../../ROLE/vision/hls/gammacorrection/gammacorrection_prj/solution1/csim/build/hls_out.jpg";
	      }
	      else if (atoi(argv[2]) == 3) {
		exec_cmd = "make cosim";
		ouf_file = "../../../../ROLE/vision/hls/gammacorrection/gammacorrection_prj/solution1/cosim/build/hls_out.jpg";
	      }
	      else if (atoi(argv[2]) == 4) {
		exec_cmd = "make kcachegrind";
		ouf_file = "../../../../ROLE/vision/hls/gammacorrection/gammacorrection_prj/solution1/fcsim/build/hls_out.jpg";
	      }
	    }
	    // Calling the actual TB over its typical makefile procedure, but passing the save file
	    // Skip the rebuilding phase on the 2nd run. However ensure that it's a clean recompile
	    // the first time.
	    clean_cmd = " ";
	    if (num_frame == 1) {
	      clean_cmd = "make clean && ";
	    }
	    string str_command = "cd ../../../../ROLE/vision/hls/gammacorrection/ && " + clean_cmd + "\
				  INPUT_IMAGE=./test/input_from_udp_to_fpga.png " + exec_cmd + " && \
				  cd ../../../../HOST/vision/gammacorrection/build/ "; 
	    const char *command = str_command.c_str(); 
  	    cout << "Calling TB with command:" << command << endl; 
	    system(command); 
	    
            free(longbuf);
	    	    	    	    
	    clock_t next_cycle_rx = clock();
            double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * 
                    total_pack / duration_rx / 1024 * 8) << endl;
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
	    unsigned int sending_now = PACK_SIZE;
	    clock_t last_cycle_tx = clock();
            for (int i = 0; i < total_pack; i++) {
                if ( i == total_pack - 1 ) {
                    sending_now = bytes_in_last_pack;
		}
		sock.sendTo( & frame.data[i * PACK_SIZE], sending_now, sourceAddress, sourcePort);
	    }
            
            clock_t next_cycle_tx = clock();
            double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
                    total_pack / duration_tx / 1024 * 8) << endl;
            last_cycle_tx = next_cycle_tx; 
            cout << "\\___________________________________________________________________/" << endl;
        } // while loop

    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }
    return 0;
}




/*! \} */
