
/*****************************************************************************
 * @file       harris_host.cpp
 * @brief      Harris userspace application for cF (x86, ppc64).
 *
 * @date       May 2020
 * @author     DID
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup Harris
 * @addtogroup Harris
 * \{
 *****************************************************************************/


#include <stdio.h>
#include <iostream>               // For cout and cerr
#include <cstdlib>                // For atoi()
#include "../include/PracticalSocket.h"      // For UDPSocket and SocketException
using namespace std;
#include "opencv2/opencv.hpp"
using namespace cv;
#include "../include/config.h"
#include <assert.h>


  /**
   *   Main testbench and user-application for Harris on host. Client
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {
    if ((argc < 3) || (argc > 4)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port> <optional input image>\n";
        exit(1);
    }

    string servAddress = argv[1]; // First arg: server address
    unsigned short servPort = Socket::resolveService(argv[2], "udp");

    try {
        UDPSocket sock;

        cv::Mat frame, send;
        vector < uchar > encoded;
		
#ifdef INPUT_FROM_CAMERA
        VideoCapture cap(0); // Grab the camera
        namedWindow("send", WINDOW_AUTOSIZE);
        if (!cap.isOpened()) {
            cerr << "OpenCV Failed to open camera";
            exit(1);
        }
#else
	frame = cv::imread(argv[3], cv::IMREAD_GRAYSCALE); // reading in the color image
	

	if (!frame.data) {
	  printf("Failed to load the image ... %s\n!", argv[3]);
	  return -1;
	}
	else {
	  printf("Succesfully loaded image ... %s\n!", argv[3]);
	}
#endif
        clock_t last_cycle = clock();
#ifdef INPUT_FROM_CAMERA	
        while (1) {
            cap >> frame;
            if(frame.size().width==0)continue;//simple integrity check; skip erroneous data...
#endif
            resize(frame, send, cv::Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
	    
	    assert(send.total() == FRAME_WIDTH * FRAME_HEIGHT);
	    
            imshow("send", send);
            int total_pack = 1 + (send.total() - 1) / PACK_SIZE;
	    cout << "\ttotal_pack=" << total_pack << endl;
	    
	    //printf("DEBUG: send.total=%u\n", send.total());
	    //printf("DEBUG: send.channels=%u\n", send.channels());
	    //printf("DEBUG: encoded.size()=%u\n", encoded.size());
	    
	    // If the image has channels we should flatten it to ensure the correct transmission
	    //uint totalElements = send.total() * send.channels(); // Note: image.total() == rows*cols.
	    //printf("DEBUG: totalElements=%u\n", totalElements);
	    //cv::Mat flat = send.reshape(1, totalElements); // 1xN mat of 1 channel, O(1) operation
	    //if(!send.isContinuous()) {
	    //  flat = flat.clone(); // O(N)
	    //}    
	    

            for (int i = 0; i < total_pack; i++) {
		sock.sendTo( & send.data[i * PACK_SIZE], PACK_SIZE, servAddress, servPort);
		//sock.sendTo( & flat.data[i * PACK_SIZE], PACK_SIZE, servAddress, servPort);
	    }
            
            clock_t next_cycle = clock();
            double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
            cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << endl;

            cout << next_cycle - last_cycle;
            last_cycle = next_cycle;
	    
	    waitKey(1000*FRAME_INTERVAL);
#ifdef INPUT_FROM_CAMERA
	}
#endif
	// Destructor closes the socket

    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }

    return 0;
}




/*! \} */