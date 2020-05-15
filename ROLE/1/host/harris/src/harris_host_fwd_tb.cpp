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

#define BUF_LEN 65540 // Larger than maximum UDP packet size

#include "opencv2/opencv.hpp"
using namespace cv;
#include "../include/config.h"




  /**
   *   Main testbench for the user-application for Harris on host. Server
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {

    if (argc != 2) { // Test for correct number of parameters
        cerr << "Usage: " << argv[0] << " <Server Port>" << endl;
        exit(1);
    }

    unsigned short servPort = atoi(argv[1]); // First arg:  local port

    namedWindow("recv", WINDOW_AUTOSIZE);
    try {
        UDPSocket sock(servPort);

        char buffer[BUF_LEN]; // Buffer for echo string
        int recvMsgSize; // Size of received message
        string sourceAddress; // Address of datagram source
        unsigned short sourcePort; // Port of datagram source

        clock_t last_cycle = clock();
#ifdef INPUT_FROM_CAMERA
        while (1) {
            // Block until receive message from a client
#endif
    
	    int total_pack = 1 + (FRAME_WIDTH * FRAME_HEIGHT - 1) / PACK_SIZE;
            cout << "expecting length of packs:" << total_pack << endl;
            char * longbuf = new char[PACK_SIZE * total_pack];
            for (int i = 0; i < total_pack; i++) {
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, sourcePort);
                if (recvMsgSize != PACK_SIZE) {
                    cerr << "Received unexpected size pack:" << recvMsgSize << endl;
#ifdef INPUT_FROM_CAMERA
                    continue;
#else
		    exit(1);
#endif
                }
                memcpy( & longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
            }

            cout << "Received packet from " << sourceAddress << ":" << sourcePort << endl;
 
            cv::Mat frame = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC1, longbuf); // OR vec.data() instead of ptr
	    if (frame.size().width == 0) {
                cerr << "decode failure!" << endl;
#ifdef INPUT_FROM_CAMERA
                continue;
#else
		exit(1);
#endif
            }
            imshow("recv", frame);
	    
	    // We save the image received from network in order to process it with the harris HLS TB
	    imwrite("../../../hls/harris_app/test/input_from_udp_to_fpga.png", frame);
	    
	    // Calling the actual TB over its typical makefile procedure, but passing the save file
	    string str_command = "cd ../../../hls/harris_app && make clean && \
				  INPUT_IMAGE=./test/input_from_udp_to_fpga.png	 make fcsim -j 4 && \
				  cd ../../host/harris/build/ "; 
	    const char *command = str_command.c_str(); 
  	    cout << "Calling TB with command:" << command << endl; 
	    system(command); 
	    
            free(longbuf);

            waitKey(1);
            clock_t next_cycle = clock();
            double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
            cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << endl;

            cout << next_cycle - last_cycle;
            last_cycle = next_cycle;
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
