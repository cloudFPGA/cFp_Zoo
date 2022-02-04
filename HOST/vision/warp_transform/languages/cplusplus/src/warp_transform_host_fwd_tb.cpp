/*******************************************************************************
 * Copyright 2016 -- 2022 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*******************************************************************************/

/*****************************************************************************
 * @file       warp_transform_host_fwd_tb.cpp
 * @brief      Testbench for WarpTransform userspace application for cF (x86, ppc64).
 *
 * @date       Nov 2021
 * @author     DID,DCO
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup WarpTransformTB
 * @addtogroup WarpTransformTB
 * \{
 *****************************************************************************/

#include <cstdlib>           // For atoi()
#include <iostream>          // For cout and cerr
#include "PracticalSockets.h"
#include "config.h"
#include "opencv2/opencv.hpp"

using namespace cv;

/*****************************************************************************
 * @brief print the binary representation of a target pointer buffer of a given size.
 *      Assumes little endian.
 * @param[in]  size the bytesize to print from ptr.
 * @param[in] ptr the buffer pointer.
 * @return nothing, print to stdout.
 ******************************************************************************/
void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}

  /**
   *   Main testbench for the user-application for WarpTransform on host. Server
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

#ifdef SHOW_WINDOWS
    namedWindow("tb_recv", WINDOW_AUTOSIZE);
#endif // SHOW_WINDOWS

    try {
      	#if NET_TYPE == udp
        UDPSocket sock(servPort);
	#else
	TCPServerSocket servSock(servPort);     // Server Socket object
	TCPSocket *servsock = servSock.accept();     // Wait for a client to connect
	#endif
        char buffer[BUF_LEN]; // Buffer for echo string
        int recvMsgSize; // Size of received message
        string sourceAddress; // Address of datagram source
	    
	#if NET_TYPE == tcp
	// TCP client handling
	cout << "Handling client ";
	try {
	  cout << servsock->getForeignAddress() << ":";
	} catch (SocketException e) {
	    cerr << "Unable to get foreign address" << endl;
	  }
	try {
	  cout << servsock->getForeignPort();
	} catch (SocketException e) {
	    cerr << "Unable to get foreign port" << endl;
	  }
	cout << endl;
	#endif
	    

        // RX Step
        clock_t last_cycle_rx = clock();
        while (1) {
            // Block until receive message from a client
    
	    int total_pack_back2Host = 1 + (FRAME_TOTAL - 1) / PACK_SIZE;
        int bytes_in_last_pack_back2Host = (FRAME_TOTAL) - (total_pack_back2Host - 1) * PACK_SIZE;
		int total_pack = 1 + (WARP_TRANSFORM_TOTAL - 1) / PACK_SIZE;
        int bytes_in_last_pack = (WARP_TRANSFORM_TOTAL) - (total_pack - 1) * PACK_SIZE;	    
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
		#if NET_TYPE == udp
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, servPort);
		#else
		recvMsgSize = servsock->recv(buffer, receiving_now);
		#endif
                if (recvMsgSize != receiving_now) {
                    cerr << "ERROR: Received unexpected size pack:" << recvMsgSize << endl;
                    continue;
                }
                memcpy( & longbuf[i * PACK_SIZE], buffer, receiving_now);
            }

            cout << "INFO: Received packet from " << sourceAddress << ":" << servPort << endl;
			std::cout << "INFO: recevied the CMD=";// << std::endl;
			printBits(8, longbuf);
			std::cout << "INFO: tx matrix0-1=    ";//<< std::endl;
			printBits(8, longbuf+(1 * 8));
			std::cout << "INFO: tx matrix2-3=    ";//<< std::endl;
			printBits(8, longbuf+(2 * 8));
			std::cout << "INFO: tx matrix4-5=    ";//<< std::endl;
			printBits(8, longbuf+(3 * 8));
			std::cout << "INFO: tx matrix6-7=    ";//<< std::endl;
			printBits(8, longbuf+(4 * 8));
			std::cout << "INFO: tx matrix8=      ";//<< std::endl;
			printBits(4, longbuf+(5 * 8));
			std::cout << "INFO: IMG CMD=         ";// << std::endl;
			printBits(8, longbuf+(6 * 8));
			std::cout << std::endl;
            cv::Mat frame = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, INPUT_TYPE_HOST, longbuf+CMD_OVERHEAD_BYTES); // OR vec.data() instead of ptr
	    if (frame.size().width == 0) {
                cerr << "ERROR: receive failure!" << endl;
                continue;
            }
#ifdef SHOW_WINDOWS
            imshow("tb_recv", frame);
#endif // SHOW_WINDOWS

	    // We save the image received from network in order to process it with the warp_transform HLS TB
	    imwrite("../../../../../../ROLE/vision/hls/warp_transform/test/input_from_udp_to_fpga.png", frame);
	    
	    // Select simulation mode, default fcsim
	    string synth_cmd = " ";
	    string exec_cmd = "make fcsim -j 4";
	    string ouf_file = "../../../../../../ROLE/vision/hls/warp_transform/warp_transform_prj/solution1/fcsim/build/hls_out.jpg";
	    if (argc == 3) {
	      if (atoi(argv[2]) == 2) {
		exec_cmd = "make csim";
		ouf_file = "../../../../../../ROLE/vision/hls/warp_transform/warp_transform_prj/solution1/csim/build/hls_out.jpg";
	      }
	      else if (atoi(argv[2]) == 3) {
		synth_cmd = "faketime '2021-12-12' make csynth && ";
		exec_cmd = "make cosim";
		ouf_file = "../../../../../../ROLE/vision/hls/warp_transform/warp_transform_prj/solution1/sim/wrapc_pc/hls_out.jpg";
	      }
	      else if (atoi(argv[2]) == 4) {
		exec_cmd = "make kcachegrind";
		ouf_file = "../../../../../../ROLE/vision/hls/warp_transform/warp_transform_prj/solution1/fcsim/build/hls_out.jpg";
	      }
	    }
	    // Calling the actual TB over its typical makefile procedure, but passing the save file
	    // Skip the rebuilding phase on the 2nd run. However ensure that it's a clean recompile
	    // the first time.
	    clean_cmd = " ";
	    if (num_frame == 1) {
	      //clean_cmd = "";
	      clean_cmd = "make clean && ";
	    }
	    string str_command = "cd ../../../../../../ROLE/vision/hls/warp_transform/ && " + clean_cmd + synth_cmd + "\
				  INPUT_IMAGE=./test/input_from_udp_to_fpga.png " + exec_cmd + " && \
				  cd ../../../../HOST/vision/warp_transform/languages/cplusplus/build/ "; 
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
	#ifdef SHOW_WINDOWS
		imshow("tb_send", frame);
	#endif //SHOW_WINDOWS
	    
	    // Ensure that the send Mat is in continuous memory space. Typically, imread or resize 
	    // will return such a continuous Mat, but we should check it.
	    assert(frame.isContinuous());
	    
	    // TX Loop
	    unsigned int sending_now = PACK_SIZE;
	    clock_t last_cycle_tx = clock();
            for (int i = 0; i < total_pack_back2Host; i++) {
                if ( i == total_pack_back2Host - 1 ) {
                    sending_now = bytes_in_last_pack_back2Host;
		}
		#if NET_TYPE == udp
		sock.sendTo( & frame.data[i * PACK_SIZE], sending_now, sourceAddress, servPort);
		#else
		//sock.send( & frame.data[i * PACK_SIZE], sending_now);
		servsock->send( & frame.data[i * PACK_SIZE], sending_now);
		#endif
	    }
            
            clock_t next_cycle_tx = clock();
            double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
                    total_pack / duration_tx / 1024 * 8) << endl;
            last_cycle_tx = next_cycle_tx; 
            cout << "\\___________________________________________________________________/" << endl;
        break;
		} // while loop
        #if NET_TYPE == tcp
        delete servsock;
	#endif
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }
    return 0;
}




/*! \} */
