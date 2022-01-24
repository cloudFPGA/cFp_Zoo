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
 * @file       warp_transform_api.hpp
 * @brief      WarpTransform API for cF based on warp_transform_host.cpp
 *
 * @date       Jan 2022
 * @author     DCO
 * 
 * @note       Copyright 2015-2020 - IBM Research - All Rights Reserved.
 * @note       http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 * 
 * @ingroup WarpTransform
 * @addtogroup WarpTransform
 * \{
 *****************************************************************************/


#include <stdio.h>
#include <iostream>                     // For cout and cerr
#include <cstdlib>                      // For atoi()
#include <assert.h>                     // For assert()
#include <string>                       // For to_string
#include <string.h>                     // For memcpy()
#include "PracticalSockets.h" // For UDPSocket and SocketException
#include "config.h"
#include "util.hpp"

#if TRANSFORM_TYPE == 1
#define TRMAT_DIM2 3
#define TRMAT_DIM1 3
#else
#define TRMAT_DIM2 3
#define TRMAT_DIM1 2
#endif

#include "opencv2/opencv.hpp"
#include "../../../../../../ROLE/vision/hls/warp_transform/include/xf_ocv_ref.hpp"  // For SW reference WarpTransform from OpenCV
using namespace cv;
using namespace std;



void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}


/*****************************************************************************
 * @brief Resize an image and crop if necessary in order to keep a rectangle 
 * area in the middle of the image
 *
 * @param[in]  input          A pointer to the cv::Mat input image
 * @param[out] output         A pointer to the cv::Mat output image
 * @param[in]  Size           A pointer to the cv::Size of the output image (width, height)
 * @param[in]  interpolation  Enumerator for interpolation algorithm (imgproc.hpp)
 *
 * @return Nothing.
 ******************************************************************************/
void resizeCropSquare(const cv::Mat &input, const cv::Mat &output, const cv::Size &dstSize, int interpolation = INTER_LINEAR)
{
    int h = input.rows;
    int w = input.cols;
    int min_size = min(h, w);
    int x = w/2-min_size/2;
    int y = h/2-min_size/2;
    // printf("w=%d, h=%d, min_size=%d, x=%d, y=%d, width=%d, height=%d\n", w, h, min_size, x, y, width, height);
    cv::Mat crop_img = input(Rect(x, y, min_size, min_size));
    resize(crop_img, output, Size(dstSize.width, dstSize.height), 0, 0, interpolation);
}

string prepareWarpTransformCommand(unsigned int rows, unsigned int cols, unsigned int channels, float * transform_matrix){
    string out;
    unsigned int bytes_per_line = 8;
    char tx_cmd [bytes_per_line];
    char img_cmd [bytes_per_line];
    char value_cmd[bytes_per_line];

    //init tx and img cmd
    for (unsigned int k = 0; k < bytes_per_line; k++) {
       value_cmd[k]    = (char)0;
        if (k != 0) {
            tx_cmd[k] = (char)0;
            img_cmd[k] = (char)0;
        }
        else {
            tx_cmd[k] = (char)1; 
            img_cmd[k] = (char)2;
        }
     }
    out = out.append(tx_cmd,bytes_per_line);

    //dump the even elements of the tx matrix
    int off = 4;
    for (int i = 0; i < 8; i++)
    {
        memcpy(value_cmd+off, (float*)transform_matrix+i, 4);
        off += 4;
        off = off % bytes_per_line;
        if (i%2 && i!=0)
        {
            //dump matrix
            out = out.append(value_cmd,bytes_per_line);
        }
    }
    //dump last value
    unsigned int zero_constant = 0;
    memcpy(value_cmd, (char*)transform_matrix+8, 4);
    memcpy(value_cmd, (char*)&zero_constant, 4);
    out = out.append(value_cmd,bytes_per_line);
    //creating img mat cmd
    memcpy(img_cmd+6, (char*)&rows, 2);
    memcpy(img_cmd+4, (char*)&cols, 2);
    img_cmd[1]=channels;
    out = out.append(img_cmd,bytes_per_line);
    return string(out);
}


std::string get_inImgName(std::string inStr, std::string delimiter){
    size_t pos = 0;
    std::string token;
    while ((pos = inStr.find(delimiter)) != std::string::npos) {
        token = inStr.substr(0, pos);
        inStr.erase(0, pos + delimiter.length());
    }
    return inStr;
}

// template <typename T=UDPSocket >
// int  open_connection(std::string s_servAddress, std::string s_servPort, T& sock){
//     //------------------------------------------------------
//     //-- STEP-1 : Socket and variables definition
//     //------------------------------------------------------

//     string servAddress = s_servAddress;
//     unsigned short servPort;
//     bool net_type = NET_TYPE;
//     if (net_type == udp) {
//         servPort = Socket::resolveService(s_servPort, "udp");
//     }
//     else if (net_type == tcp) {
//         servPort = atoi(s_servPort.c_str());
//     }
//     else {
//      cout << "ERROR: Invalid type of socket type provided: " << net_type  << " Choosed one of (tcp=0 or udp=1)" << endl;
//     }    
    
//     try {
          
//         //------------------------------------------------------
//         //-- STEP-2 : Initialize socket connection
//         //------------------------------------------------------      
// #if NET_TYPE == udp
// #ifndef TB_SIM_CFP_VITIS
//            sock(servPort); // NOTE: It is very important to set port here in order to call 
//                                       // bind() in the UDPSocket constructor
// #else // TB_SIM_CFP_VITIS
//             sock; // NOTE: In HOST TB the port is already binded by warp_transform_host_fwd_tb.cpp
// #endif // TB_SIM_CFP_VITIS
// #else // tcp
//            sock(servAddress, servPort);
// #endif // udp/tcp
//     // Destructor closes the socket
//     } catch (SocketException & e) {
//         cerr << e.what() << endl;
//         exit(1);
//     }
//     return servPort;
// }

// template <typename T=UDPSocket >
void cF_host_warp_transform(std::string s_servAddress, std::string s_servPort, cv::Mat input_im, float* transformation_matrix_float, cv::Mat &output_im)//, T sock, unsigned short servPort)
{
    std::cout << "Recevied addr= " << s_servAddress << " port= " << s_servPort << endl;
    //std::cout << " Inpt matrix " << input_im << endl << " tx mat[0] " << transformation_matrix_float[0] << endl;
    // cout <<  " Out img " << output_im <<endl;
    //------------------------------------------------------
    //-- STEP-1 : Socket and variables definition
    //------------------------------------------------------

    string servAddress = s_servAddress;
    unsigned short servPort;
    bool net_type = NET_TYPE;
    if (net_type == udp) {
    servPort = Socket::resolveService(s_servPort, "udp");
    }
    else if (net_type == tcp) {
    servPort = atoi(s_servPort.c_str());
    }
    else {
    cout << "ERROR: Invalid type of socket type provided: " << net_type  << " Choosed one of (tcp=0 or udp=1)" << endl;
    }    
    
    unsigned char buffer[BUF_LEN]; // Buffer for echo string
    unsigned int recvMsgSize; // Size of received message

    
    try {
          
        //------------------------------------------------------
        //-- STEP-2 : Initialize socket connection
        //------------------------------------------------------      
#if NET_TYPE == udp
#ifndef TB_SIM_CFP_VITIS
            UDPSocket sock(servPort); // NOTE: It is very important to set port here in order to call 
                                      // bind() in the UDPSocket constructor
#else // TB_SIM_CFP_VITIS
            UDPSocket sock; // NOTE: In HOST TB the port is already binded by warp_transform_host_fwd_tb.cpp
#endif // TB_SIM_CFP_VITIS
#else // tcp
            TCPSocket sock(servAddress, servPort);
#endif // udp/tcp
 
    cv::Mat transformation_matrix(TRMAT_DIM1, TRMAT_DIM2, CV_32FC1, transformation_matrix_float);
    //------------------------------------------------------------------------------------
    //-- STEP-3 : Initialize a Greyscale OpenCV Mat either from image or from video/camera
    //------------------------------------------------------------------------------------
    Mat frame, send(FRAME_WIDTH, FRAME_HEIGHT, INPUT_TYPE_HOST, Scalar(0)), ocv_out_img;

    std::string warptx_cmd = prepareWarpTransformCommand(FRAME_WIDTH, FRAME_HEIGHT, send.channels(), transformation_matrix_float);
    unsigned int num_frame=0;
    // cout << "INFO: Frame # " << 
    ++num_frame;
    // << endl;
        
#if CV_MAJOR_VERSION < 4
            cv::cvtColor(input_im,frame,CV_BGR2GRAY);
#else
            cv::cvtColor(input_im,frame,cv::COLOR_BGR2GRAY);
#endif
            resizeCropSquare(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), INTER_LINEAR);
            if ((frame.cols != FRAME_WIDTH) || (frame.rows != FRAME_HEIGHT)) {
                cout << "WARNING: Input frame was resized from " << frame.cols << "x" 
                        << frame.rows << " to " << send.cols << "x" << send.rows << endl;
            }

            assert(send.total() == FRAME_WIDTH * FRAME_HEIGHT);
            // Ensure that the selection of MTU is a multiple of 8 (Bytes per transaction)
            assert(PACK_SIZE % 8 == 0);
    
            // Ensure that the send Mat is in continuous memory space. Typically, imread or resize 
            // will return such a continuous Mat, but we should check it.
            assert(send.isContinuous());

            unsigned int send_total = send.total();
            unsigned int send_channels = send.channels();
            unsigned int warptx_cmd_size = warptx_cmd.length();
            std::cout << "Stuffs to send tot-chan-waxcmd " << send_total << " " << send_channels << " " << warptx_cmd_size <<endl;

            unsigned int total_pack  = 1 + (send_total * send_channels - 1 +  warptx_cmd_size) / PACK_SIZE;
            unsigned int total_bytes = total_pack * PACK_SIZE;
            unsigned int bytes_in_last_pack = send_total * send_channels +  warptx_cmd_size - (total_pack - 1) * PACK_SIZE;
            assert(total_pack == TOT_TRANSFERS_TX);

            unsigned int total_pack_rx  = 1 + (send_total * send_channels - 1) / PACK_SIZE;
            unsigned int total_bytes_rx = total_pack_rx * PACK_SIZE;
            unsigned int bytes_in_last_pack_rx = send_total * send_channels - (total_pack_rx- 1) * PACK_SIZE;
            assert(total_pack_rx == TOT_TRANSFERS_RX);

	        unsigned char * longbuf = (unsigned char *) malloc (PACK_SIZE * total_pack_rx * sizeof (unsigned char));

            // cout << "INFO: FPGA destination : " << servAddress << ":" << servPort << endl;
            // cout << "INFO: Network socket   : " << ((NET_TYPE == tcp) ? "TCP" : "UDP") << endl;
            // cout << "INFO: Total packets to send= " << total_pack << endl;
            // cout << "INFO: Total packets to receive = " << total_pack_rx << endl;
            // cout << "INFO: Total bytes to send   = " << send_total * send_channels +  warptx_cmd_size << endl;
            // cout << "INFO: Total bytes to receive   = " << send_total * send_channels << endl;
            // cout << "INFO: Total bytes in " << total_pack << " packets = "  << total_bytes << endl;
            // cout << "INFO: Total bytes in " << total_pack_rx << " packets = "  << total_bytes_rx << endl;
            // cout << "INFO: Bytes in last packet          = " << bytes_in_last_pack << endl;
            // cout << "INFO: Bytes in last packet to receive    = " << bytes_in_last_pack_rx << endl;
            // cout << "INFO: Packet size (custom MTU)      = " << PACK_SIZE << endl;

            //--------------------------------------------------------
            //-- STEP-4 : RUN WARPTRANSFORM DETECTOR FROM OpenCV LIBRARY (SW)
            //--------------------------------------------------------
            // ocv_out_img.create(send.rows, send.cols, INPUT_TYPE_HOST); // create memory for opencv output image
            //------------------------------------------------------
            //-- STEP-5.1 : Preparation
            //------------------------------------------------------

            // Anchor a pointer on cvMat raw data
            unsigned char * sendarr_img = send.isContinuous()? send.data: send.clone().data;
	        unsigned char * sendarr = (unsigned char *) malloc (send_total * send_channels +  warptx_cmd_size);
            memcpy(sendarr,warptx_cmd.c_str(), warptx_cmd_size);
            memcpy(sendarr+warptx_cmd_size,sendarr_img, send_total * send_channels);
            
    
            cout << "INFO: setup everything for sending " << total_pack << " packs with total bytes of " << to_string(send_total * send_channels +  warptx_cmd_size) << endl;
            //------------------------------------------------------
            //-- STEP-5.2 : TX Loop
            //------------------------------------------------------
            unsigned int sending_now = PACK_SIZE;
            for (unsigned int i = 0; i < total_pack; i++) {
                if ( i == total_pack - 1 ) {
                    sending_now = bytes_in_last_pack;
                }
                #if NET_TYPE == udp
                sock.sendTo( & sendarr[i * PACK_SIZE], sending_now, servAddress, servPort);
                #else
                sock.send( & sendarr[i * PACK_SIZE], sending_now);
                #endif
                // cout << "DEBUG: iteration " << i << " sending " << sending_now << endl;
                //delay(50);  
            }

            //------------------------------------------------------
            //-- STEP-5.3 : RX Loop
            //------------------------------------------------------    
            unsigned int loopi=0;
            unsigned int receiving_now = PACK_SIZE;
            cout << "INFO: Expecting length of packs:" << total_pack_rx << " from " <<  servAddress << ":" << servPort << endl;
            for (unsigned int i = 0; i < send_total; ) {
                #if NET_TYPE == udp                
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, servAddress, servPort);
                #else
                recvMsgSize = sock.recv(buffer, BUF_LEN);
                #endif
                if (recvMsgSize != receiving_now) {
                    cerr << "WARNING: at i=" << i << " received unexpected size pack:" << recvMsgSize << ". Expected: " << 
                            receiving_now << endl;
                    //continue;
                }
                memcpy( & longbuf[i], buffer, recvMsgSize);
                i += recvMsgSize;
                // cout << "DEBUG: iteration " << i << " receiving " << receiving_now << endl;
                //delay(50);
            }
            // cout << "INFO: Received packet from " << servAddress << ":" << servPort << endl;

            frame = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, INPUT_TYPE_HOST, longbuf); // OR vec.data() instead of ptr
            if (frame.size().width == 0) {
                cerr << "receive failure!" << endl;
            }

        output_im =  frame.clone();
	    free (sendarr);
	    free (longbuf);
        //delete &sock;
        cout << "here hsould be deleted the socket" << endl;
    // Destructor closes the socket
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }
    cout<< "Exiting" << endl;
    
}
/*! \} */