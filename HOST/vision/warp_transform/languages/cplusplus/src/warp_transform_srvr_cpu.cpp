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
 * @file       warp_transform_host_srvr_cpu.cpp
 * @brief      Host server cpu for WarpTransform userspace application for cF (x86, ppc64).
 *
 * @date       Feb 2022
 * @author     DCO
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup WarpTransform
 * @addtogroup WarpTransform
 * \{
 *****************************************************************************/

#include <cstdlib>           // For atoi()
#include <iostream>          // For cout and cerr
#include "PracticalSockets.h"
#include "config.h"
#include "cv_warp_transform_config.hpp"

#include <stdio.h>
#include <iostream>                     // For cout and cerr
#include <cstdlib>                      // For atoi()
#include <assert.h>                     // For assert()
#include <string>                       // For to_string
#include <string.h>                     // For memcpy()
#include "warp_transform_api.hpp"

#define RESET 0
#define PROCESSING_PACKET 1
#define PROCESSING_PACKET_TXMAT 2
#define OUT_STATE 3

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

    if ((argc < 2) || (argc > 2)) { // Test for correct number of parameters
        cerr << "Usage: " << argv[0] << " <Server Port>" << endl;
        exit(1);
    }

    unsigned short servPort = atoi(argv[1]); // First arg:  local port
    unsigned int num_frame = 0;
    

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
        char buffer4Commands[CMD_OVERHEAD_BYTES];
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
	img_meta_t img_rows, img_cols,img_chan, img_pixels;
	float tx_matrix[TRANSFORM_MATRIX_DIM];
	size_t buff_ptr;
	int min_pack_to_recevie = 1;
    	char init_buff [PACK_SIZE*min_pack_to_recevie];
	int receiving_now = PACK_SIZE;

        // RX Step
        while (1) {
        std::cout << " ___________________________________________________________________ " << std::endl;
		std::cout << "/                                                                   \\" << std::endl;
		std::cout << "INFO: Proxy tb Frame # " << ++num_frame << std::endl;	    
            // Block until receive message from a client
        //init
        img_rows =FRAME_HEIGHT;
        img_cols = FRAME_WIDTH;
        img_chan = 1;
        memset(tx_matrix,  0x0, sizeof(tx_matrix));
        memset(init_buff,  0x0, sizeof(init_buff));
        memset(buffer4Commands,  0x0, sizeof(buffer4Commands));
        buff_ptr = 0;
        //variables
        receiving_now = PACK_SIZE;
        #if NET_TYPE == udp
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, sourceAddress, servPort);
		#else
		recvMsgSize = servsock->recv(buffer, receiving_now);
		#endif
        memcpy(init_buff, buffer, receiving_now);

            // cout << "INFO: Received packet from " << sourceAddress << ":" << servPort << endl;
			// std::cout << "INFO: recevied the CMD=";// << std::endl;
			// printBits(8, init_buff);
			// std::cout << "INFO: tx matrix0-1=    ";//<< std::endl;
			// printBits(8, init_buff+(1 * 8));
			// std::cout << "INFO: tx matrix2-3=    ";//<< std::endl;
			// printBits(8, init_buff+(2 * 8));
			// std::cout << "INFO: tx matrix4-5=    ";//<< std::endl;
			// printBits(8, init_buff+(3 * 8));
			// std::cout << "INFO: tx matrix6-7=    ";//<< std::endl;
			// printBits(8, init_buff+(4 * 8));
			// std::cout << "INFO: tx matrix8=      ";//<< std::endl;
			// printBits(4, init_buff+(5 * 8));
			// std::cout << "INFO: IMG CMD=         ";// << std::endl;
			// printBits(8, init_buff+(6 * 8));
			// std::cout << std::endl;


        char readWord[1];
        unsigned int tx_mat_idx = 0;
        CPUPacketFsmType parseFSM = PROCESSING_PACKET;
        unsigned int img_pixels=FRAME_TOTAL;
        switch(parseFSM)
        {
        case PROCESSING_PACKET:
	{
            printf("DEBUG in parseRXData: parseFSM - PROCESSING_PACKET\n");
            //-- Read incoming data chunk
            memcpy(readWord, init_buff+buff_ptr, sizeof(char));
            printf("Read some data %s\n",readWord);
            printBits(sizeof(char), readWord);
	    buff_ptr+=sizeof(char);
            switch(*readWord)//the command is in the first 8 bits
            {
            case(WRPTX_TXMAT_CMD):{
                printf("TX MAT CMD\n");
                parseFSM = PROCESSING_PACKET_TXMAT;
                tx_mat_idx = 0;
                break;
                }
            case(WRPTX_IMG_CMD):{
                printf("IMG CMD\n");
                memcpy(&img_rows, init_buff+buff_ptr, sizeof(char)*2);
                buff_ptr+=sizeof(char)*2;
                memcpy(&img_cols, init_buff+buff_ptr, sizeof(char)*2);
                buff_ptr+=sizeof(char)*2;
                memcpy(&img_chan, init_buff+buff_ptr, sizeof(char)*1);
                buff_ptr+=sizeof(char)*1;

                std::cout << "DEBUG parseRXData - img rows =" << img_rows << " cols=" << img_cols << " chan=" << img_chan << std::endl; 
                img_pixels = img_rows * img_cols * img_chan;
                unsigned int meta_by_images = img_pixels/PACK_SIZE;
                std::cout << "DEBUG parseRXData pixels " << img_pixels << std::endl;
		parseFSM = OUT_STATE;
                }
            //TODO: fix the default case
            default:{
                cerr << "ERROR  skipping iteration" << std::endl;
                continue;
                }

            }
	 }//case
        case PROCESSING_PACKET_TXMAT:
	    {
            printf("DEBUG in parseRXData: parseFSM - PROCESSING_PACKET_TXMAT\n");
            //-- Read incoming data chunk
            for(int i =0; i<TRANSFORM_MATRIX_DIM; i++){
                float_bits_u tmp1;
                // float_bits_u tmp2;
                memcpy(&tmp1, init_buff+buff_ptr , sizeof(float));
                buff_ptr+=sizeof(float);
                tx_matrix[tx_mat_idx]=tmp1.f;
		cout<<"DEBUG: reading th value " << i << " with val=" << tx_matrix[tx_mat_idx] << endl;
                tx_mat_idx++; // it seems equal to i
            }
            buff_ptr+=sizeof(float);
            tx_mat_idx=0;
            parseFSM = PROCESSING_PACKET;
	    cout << "DEBUG: going to processing pckt" << endl;
	    }
	 case OUT_STATE:
	    {
	    cout << "INFO: exiting from the parsingFSM" << endl;
	    }
        }
        img_pixels = img_rows * img_cols * img_chan;
        
        
        // run time changeable
	int total_pack_back2Host = 1 + (img_pixels - 1) / PACK_SIZE;
    int bytes_in_last_pack_back2Host = (img_pixels) - (total_pack_back2Host - 1) * PACK_SIZE;
	int total_pack = 1 + (img_pixels+CMD_OVERHEAD_BYTES - 1) / PACK_SIZE - 1 ;
    int bytes_in_last_pack = (img_pixels+CMD_OVERHEAD_BYTES) - (total_pack - 1) * PACK_SIZE;	    
	cout << "INFO: Expecting length of packs:" << total_pack << endl;
	char * longbuf = new char[PACK_SIZE * total_pack];
	memcpy(longbuf, init_buff+buff_ptr, PACK_SIZE-buff_ptr);    
	

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
                memcpy( & longbuf[buff_ptr+(i * PACK_SIZE)], buffer, receiving_now);
            }
            //if run-time img dims, first step catching if img in receivign the compute those numbers
        
        cv::Mat frame(img_rows, img_cols, INPUT_TYPE_HOST, longbuf); // OR vec.data() instead of ptr
        cv::Mat ocv_out_img(img_rows, img_cols, INPUT_TYPE_HOST); // OR vec.data() instead of ptr
	    if (frame.size().width == 0) {
                cerr << "ERROR: receive failure!" << endl;
                continue;
            }
#ifdef SHOW_WINDOWS
            imshow("tb_recv", frame);
#endif // SHOW_WINDOWS

	    // We save the image received from network in order to process it with the warp_transform HLS TB
	    imwrite("../../../../../../ROLE/vision/hls/warp_transform/test/input_from_udp_to_fpga.png", frame);
	imwrite("./input_from_udp_to_fpga"+to_string(num_frame)+".png", frame);
        cv::Mat transformation_matrix(TRMAT_DIM1, TRMAT_DIM2, CV_32FC1, tx_matrix);
        ocv_ref(frame, ocv_out_img, transformation_matrix);
	imwrite("./out_to_fpga"+to_string(num_frame)+".png", ocv_out_img);
        //free(longbuf);
 	delete longbuf;  
	        
	    assert(ocv_out_img.total() == FRAME_WIDTH * FRAME_HEIGHT);
	#ifdef SHOW_WINDOWS
		imshow("tb_send", ocv_out_img);
	#endif //SHOW_WINDOWS
	    
	    // Ensure that the send Mat is in continuous memory space. Typically, imread or resize 
	    // will return such a continuous Mat, but we should check it.
	    assert(ocv_out_img.isContinuous());
	    
	    // TX Loop
	    unsigned int sending_now = PACK_SIZE;
	    clock_t last_cycle_tx = clock();
            for (int i = 0; i < total_pack_back2Host; i++) {
                if ( i == total_pack_back2Host - 1 ) {
                    sending_now = bytes_in_last_pack_back2Host;
		}
		#if NET_TYPE == udp
		sock.sendTo( & ocv_out_img.data[i * PACK_SIZE], sending_now, sourceAddress, servPort);
		#else
		servsock->send( & ocv_out_img.data[i * PACK_SIZE], sending_now);
		#endif
	    }
            clock_t next_cycle_tx = clock();
            double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
                    total_pack / duration_tx / 1024 * 8) << endl;
            last_cycle_tx = next_cycle_tx; 
            cout << "\\___________________________________________________________________/" << endl;
        // break;
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
