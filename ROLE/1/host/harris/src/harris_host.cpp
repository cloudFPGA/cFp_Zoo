/*
 *   C++ UDP socket client for live image upstreaming
 *   Modified from http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 *   Copyright (C) 2015
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../include/PracticalSocket.h"      // For UDPSocket and SocketException
#include <iostream>               // For cout and cerr
#include <cstdlib>                // For atoi()
#include <stdio.h>

using namespace std;

#include "opencv2/opencv.hpp"
using namespace cv;
#include "../include/config.h"


void matToVector (Mat *mat, std::vector<uchar> *array) {
  if (mat->isContinuous()) {
    // array.assign(mat.datastart, mat.dataend); // <- has problems for sub-matrix like mat = big_mat.row(i)
    array->assign(mat->data, mat->data + mat->total());
  } else {
    for (int i = 0; i < mat->rows; ++i) {
      array->insert(array->end(), mat->ptr<uchar>(i), mat->ptr<uchar>(i)+mat->cols);
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

    try {
        UDPSocket sock;
        int jpegqual =  ENCODE_QUALITY; // Compression Parameter

        Mat frame, send;
        vector < uchar > encoded;
	
#ifdef INPUT_FROM_CAMERA
        VideoCapture cap(0); // Grab the camera
        namedWindow("send", WINDOW_AUTOSIZE);
        if (!cap.isOpened()) {
            cerr << "OpenCV Failed to open camera";
            exit(1);
        }
#else
	frame = cv::imread(argv[3], 0); // reading in the color image

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
            resize(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
            vector < int > compression_params;
            compression_params.push_back(IMWRITE_JPEG_QUALITY);
            compression_params.push_back(jpegqual);

            //imencode(".jpg", send, encoded, compression_params);
            //imshow("send1", send);
            //int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;
	    cout << "\ttotal_pack=" << total_pack << endl;
	    matToVector(&send, &encoded);
            imshow("send2", send);
            int total_pack2 = 1 + (encoded.size() - 1) / PACK_SIZE;
	    cout << "\ttotal_pack2=" << total_pack2 << endl;
	    
            int ibuf[1];
            ibuf[0] = total_pack;
            sock.sendTo(ibuf, sizeof(int), servAddress, servPort);

            for (int i = 0; i < total_pack; i++)
                sock.sendTo( & encoded[i * PACK_SIZE], PACK_SIZE, servAddress, servPort);

            waitKey(1000*FRAME_INTERVAL);

            clock_t next_cycle = clock();
            double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
            cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << endl;

            cout << next_cycle - last_cycle;
            last_cycle = next_cycle;
        
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
