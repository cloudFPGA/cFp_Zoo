
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
#include <iostream>                     // For cout and cerr
#include <cstdlib>                      // For atoi()
#include <assert.h>                     // For assert()
#include "../include/PracticalSocket.h" // For UDPSocket and SocketException
#include "../include/config.h"
#include "opencv2/opencv.hpp"

// For HOST TB uncomment the following
 #define TB_SIM_CFP_VITIS


using namespace std;
using namespace cv;


void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}


/*****************************************************************************
 * @brief Mark the points found by Harris into the image.
 * 
 * @return Nothing
 ******************************************************************************/
void markPointsOnImage(cv::Mat& imgOutput, cv::Mat& in_img, cv::Mat& out_img, std::vector<cv::Point>& hls_points) {

   for (int j = 0; j < imgOutput.rows; j++) {
      for (int i = 0; i < (imgOutput.cols); i++) {
	  //for CV_8UC1
          unsigned char pix = imgOutput.at<unsigned char>(j,i);  //.read(j * (imgOutput.cols) + i);
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
    char buffer[BUF_LEN]; // Buffer for echo string
    unsigned int recvMsgSize; // Size of received message
    
    try {
        #ifndef TB_SIM_CFP_VITIS
        UDPSocket sock(servPort); // NOTE: It is very important to set port here in order to call 
	                          // bind() in the UDPSocket constructor
	#else
        UDPSocket sock; // NOTE: In HOST TB the port is already binded by harris_host_fwd_tb.cpp
        #endif
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
	frame = cv::imread(argv[3], cv::IMREAD_GRAYSCALE); // reading in the image in grey scale
	

	if (!frame.data) {
	  printf("ERROR: Failed to load the image ... %s!\n", argv[3]);
	  return -1;
	}
	else {
	  printf("INFO: Succesfully loaded image ... %s!\n", argv[3]);
	}
#endif
        clock_t last_cycle_tx = clock();
#ifdef INPUT_FROM_CAMERA	
        while (1) {
            cap >> frame;
            if(frame.size().width==0)continue;//simple integrity check; skip erroneous data...
#endif
            resize(frame, send, cv::Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
	    
	    assert(send.total() == FRAME_WIDTH * FRAME_HEIGHT);
	    
            imshow("host_send", send);
	    
	    // Ensure that the send Mat is in continuous memory space. Typically, imread or resize 
	    // will return such a continuous Mat, but we should check it.
	    assert(send.isContinuous());
	    
            unsigned int total_pack  = 1 + (send.total() - 1) / PACK_SIZE;
            unsigned int total_bytes = total_pack * PACK_SIZE;
            unsigned int bytes_in_last_pack = send.total() - (total_pack - 1) * PACK_SIZE;

	    cout << "INFO: Total packets to send = " << total_pack << endl;
            cout << "INFO: Total bytes to send = " << send.total() << endl;
	    cout << "INFO: Total bytes in " << total_pack << " packets = "  << total_bytes << endl;
	    cout << "INFO: Bytes in last packet  = " << bytes_in_last_pack << endl;
	    
	    assert(total_pack == TOT_TRANSFERS);
	    
	    
	    // TX Loop
	    unsigned int sending_now = PACK_SIZE;
            for (unsigned int i = 0; i < total_pack; i++) {
                if ( i == total_pack - 1 ) {
                    sending_now = bytes_in_last_pack;
                }
		sock.sendTo( & send.data[i * PACK_SIZE], sending_now, servAddress, servPort);
		delay(100);  
	    }
            
            clock_t next_cycle_tx = clock();
            double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * total_pack / duration_tx / 1024 * 8) << endl;
            last_cycle_tx = next_cycle_tx;
	    
	    clock_t last_cycle_rx = clock();
	    
	    // RX Loop
	    unsigned int receiving_now = PACK_SIZE;
            cout << "INFO: Expecting length of packs:" << total_pack << endl;
            char * longbuf = new char[PACK_SIZE * total_pack];
            for (unsigned int i = 0; i < total_pack; i++) {
                if ( i == total_pack - 1 ) {
                    receiving_now = bytes_in_last_pack;
                }
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, servAddress, servPort);
                if (recvMsgSize != receiving_now) {
                    cerr << "Received unexpected size pack:" << recvMsgSize << ". Expected: " << receiving_now << endl;
#ifdef INPUT_FROM_CAMERA
                    continue;
#else
		    exit(1);
#endif
                }
                memcpy( & longbuf[i * PACK_SIZE], buffer, receiving_now);
            }

            cout << "INFO: Received packet from " << servAddress << ":" << servPort << endl;
 
            frame = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC1, longbuf); // OR vec.data() instead of ptr
	    if (frame.size().width == 0) {
                cerr << "receive failure!" << endl;
#ifdef INPUT_FROM_CAMERA
                continue;
#else
		exit(1);
#endif
            }
            namedWindow("host_recv", WINDOW_AUTOSIZE);
            imshow("host_recv", frame);
            
            clock_t next_cycle_rx = clock();
            double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * total_pack / duration_rx / 1024 * 8) << endl;
            last_cycle_rx = next_cycle_rx;
	    
	    string out_file;
	    out_file.assign(argv[3]);
	    out_file += "_fpga_out.png";
	    cout << "INFO: The output file is stored at: " << out_file << endl; 
	    
	    // We save the image received from network after being processed by harris HLS TB
	    imwrite(out_file, frame);
	      
            cv::Mat out_img;
            out_img = send.clone();

            std::vector<cv::Point> hls_points;
	    
	    /* Mark HLS points on the image */
	    markPointsOnImage(frame, send, out_img, hls_points);
	    
	    
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