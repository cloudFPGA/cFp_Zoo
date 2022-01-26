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
 * @file       warp_transform_host.cpp
 * @brief      WarpTransform userspace application for cF (x86, ppc64).
 *
 * @date       Nov 2021
 * @author     DID,DCO
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


#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_NUMPI)
#include "opencv2/opencv.hpp"
#include "../../../../../../ROLE/vision/hls/warp_transform/include/xf_ocv_ref.hpp"  // For SW reference WarpTransform from OpenCV
using namespace cv;
#endif

using namespace std;

void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

void print_cFpZoo(void)
{
        cout <<  "                                                          " << endl;
	cout <<  "...build with:                                            " << endl;
	cout <<  " ██████╗███████╗██████╗    ███████╗ ██████╗  ██████╗      " << endl;
	cout <<  "██╔════╝██╔════╝██╔══██╗   ╚══███╔╝██╔═══██╗██╔═══██╗     " << endl;
	cout <<  "██║     █████╗  ██████╔╝     ███╔╝ ██║   ██║██║   ██║     " << endl;
	cout <<  "██║     ██╔══╝  ██╔═══╝     ███╔╝  ██║   ██║██║   ██║     " << endl;
	cout <<  "╚██████╗██║     ██║███████╗███████╗╚██████╔╝╚██████╔╝     " << endl;
	cout <<  " ╚═════╝╚═╝     ╚═╝╚══════╝╚══════╝ ╚═════╝  ╚═════╝      " << endl;
	cout <<  "A cloudFPGA project from IBM ZRL                    v1.0  " << endl;
	cout <<  "Warp transform kernel                                     " << endl;
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

string prepareWarpTransformCommand(unsigned int rows, unsigned int cols, unsigned int channels,float * transform_matrix){
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


#ifdef PY_WRAP
#if PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME
void warp_transform(char *s_servAddress, char *s_servPort, char *input_str, char *output_img_str, char *output_points_str)
#elif PY_WRAP == PY_WRAP_WARPTRANSFORM_NUMPI
void warp_transform(int total_size, unsigned char *input_img, int total_size2, unsigned char *output_img, char *s_servAddress, char *s_servPort)
#endif // PY_WRAP value
{
#else // !PY_WRAP
  /**
   *   Main testbench and user-application for WarpTransform on host. Client
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {
    if ((argc < 3) || (argc > 6)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port> <optional input image> <optional output folder> <optional wax mode> \n";
        exit(1);
    }
#endif // PY_WRAP

    //------------------------------------------------------
    //-- STEP-1 : Socket and variables definition
    //------------------------------------------------------
    
    #ifndef PY_WRAP
    assert ((argc == 3) || (argc == 4) || (argc == 5) || (argc == 6));
    string s_servAddress = argv[1]; // First arg: server address
    char *s_servPort = argv[2];
    #endif

    string servAddress = s_servAddress;
    unsigned short servPort;
    bool net_type = NET_TYPE;
    if (net_type == udp) {
    servPort = Socket::resolveService(s_servPort, "udp");
    }
    else if (net_type == tcp) {
    servPort = atoi(s_servPort);
    }
    else {
    cout << "ERROR: Invalid type of socket type provided: " << net_type  << " Choosed one of (tcp=0 or udp=1)" << endl;
    }    
    
    unsigned char buffer[BUF_LEN]; // Buffer for echo string
    unsigned int recvMsgSize; // Size of received message
    string input_string, out_folder_string, out_filename,strWaxMode;
#ifdef INPUT_FROM_CAMERA
    int input_num;
#ifdef PY_WRAP
#if PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME
    input_num = atoi(input_str);
    input_string = "./cam"+to_string(input_num);
#endif // PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME
#else // !PY_WRAP
    if (argc == 3) {
        input_num = 0;
    }
    else if (argc == 4) {
        input_num = atoi(argv[3]);
    }
    input_string = "./cam"+to_string(input_num);
#endif // PY_WRAP    
#else // !INPUT_FROM_CAMERA
#ifdef PY_WRAP
#if PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME
    input_string.assign(input_str);
#endif // PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME    
#else // !PY_WRAP
    if (argc == 3) {
        // Give a default image
        input_string.assign("../../../../../../ROLE/vision/hls/warp_transform/test/8x8.png");
    }
    else if (argc >= 4) {
        input_string.assign(argv[3]);
    }
    if(argc >= 5) {
       out_folder_string.assign(argv[4]);
       //reasonably (hopefully :) assuming no / in the filename)
       out_filename = out_folder_string + get_inImgName(input_string, "/");
        if(argc>=6){
            strWaxMode.assign(argv[5]);
        }
    }else{
        out_folder_string.assign(input_string);
        out_filename.assign(input_string);
    }
#endif // PY_WRAP
#endif // INPUT_FROM_CAMERA
    unsigned int wax_mode = 2;
    try{
		wax_mode = stoul(strWaxMode);
	} catch  (const std::exception& e) {
		std::cerr << e.what() << '\n';
		cout << "WARNING something bad happened in the wax mode insertion, hence default used" << endl;
		wax_mode = 2;
	}
   
#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)
   
    float transformation_matrix_float [9]= {1,0,0,0,1,0,0,0,0};
    float square_reduction [9] = {1.5,0,0,0,1.8,0,0,0,0};
    float yscale_tx_mat [9] = {2,0,0,0,1,0,0,0,0};////cx  0 0 0 cy 0 000
    float xscale_tx_mat [9] = {1,0,0,0,2,0,0,0,0};////cx  0 0 0 cy 0 000
    float rotation_30degree_tx_mat [9] = {0.87,-0.5,0,0.5,0.87,0,0,0,0}; //cos -sin 0 sin cos 0 000
    float xtranslation_tx_mat [9] = {1,0,2,0,1,0,0,0,0};// 1 0 vx 0 1 vy 000
    float ytranslation_tx_mat [9] = {1,0,0,0,1,2,0,0,0};// 1 0 vx 0 1 vy 000
    float shearing_tx_mat [9] = {1,0.5,0,0,1,0,0,0,0}; //1 cx 0 cy 1 0 000
    float reflection_tx_mat [9] = {-1,0,0,0,1,0,0,0,0};
    float identity [9] = {1,0,0,0,1,0,0,0,0};

// on the TX have a look of a visual comparison to opencv results. the same will be applied to this kernel
// moreover, opencv matrixes seems column-wise format.
    switch (wax_mode)
    {
    case 1:
    //square_reduction
        std::copy(std::begin(square_reduction), std::end(square_reduction), std::begin(transformation_matrix_float));
        break;
    case 2:
    //yscale_tx_mat
        std::copy(std::begin(yscale_tx_mat), std::end(yscale_tx_mat), std::begin(transformation_matrix_float));
        break;
    case 3:
    //xscale_tx_mat
        std::copy(std::begin(xscale_tx_mat), std::end(xscale_tx_mat), std::begin(transformation_matrix_float));
        break;
    case 4:
    //rotation_30degree_tx_mat
        std::copy(std::begin(rotation_30degree_tx_mat), std::end(rotation_30degree_tx_mat), std::begin(transformation_matrix_float));
        break;
    case 5:
    //xtranslation_tx_mat
        std::copy(std::begin(xtranslation_tx_mat), std::end(xtranslation_tx_mat), std::begin(transformation_matrix_float));
        break;
    case 6:
    //ytranslation_tx_mat
        std::copy(std::begin(ytranslation_tx_mat), std::end(ytranslation_tx_mat), std::begin(transformation_matrix_float));
        break;
    case 7:
    //shearing_tx_mat
        std::copy(std::begin(shearing_tx_mat), std::end(shearing_tx_mat), std::begin(transformation_matrix_float));
        break;                
    case 8:
    //reflection_tx_mat
        std::copy(std::begin(reflection_tx_mat), std::end(reflection_tx_mat), std::begin(transformation_matrix_float));
        break;     
    default:
    //identity
        std::copy(std::begin(identity), std::end(identity), std::begin(transformation_matrix_float));
        break;
    }
    cv::Mat transformation_matrix(TRMAT_DIM1, TRMAT_DIM2, CV_32FC1, transformation_matrix_float);
    /////////////////
    string out_img_file;
    string out_video_file;
    // Define the codec and create VideoWriter object.The output is stored in 'outcpp.avi' file. 
    //#ifdef PY_WRAP
    //out_video_file.assign(output_str);
    //#else // !PY_WRAP
    //out_video_file.assign(input_string);
    out_video_file.assign(out_img_file);
    out_video_file += "_fpga_video_out.avi";
    //#endif // PY_WRAP
#if CV_MAJOR_VERSION < 4
    VideoWriter video(out_video_file,CV_FOURCC('M','J','P','G'),10, Size(FRAME_WIDTH,FRAME_HEIGHT));
#else
    VideoWriter video(out_video_file,cv::VideoWriter::fourcc('M','J','P','G'),10, Size(FRAME_WIDTH,FRAME_HEIGHT));
#endif

#endif // #if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME) 

    print_cFpZoo();
    
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
 

#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)


            //------------------------------------------------------------------------------------
            //-- STEP-3 : Initialize a Greyscale OpenCV Mat either from image or from video/camera
            //------------------------------------------------------------------------------------
            Mat frame, send(FRAME_WIDTH, FRAME_HEIGHT, INPUT_TYPE_HOST, Scalar(0)), ocv_out_img;
            vector < uchar > encoded;

#ifdef INPUT_FROM_CAMERA

        VideoCapture cap(input_num); // Grab the camera
        if (!cap.isOpened()) {
            cerr << "OpenCV Failed to open camera " + input_num << endl;
            exit(1);
        }
        
#else // INPUT_FROM_CAMERA

        VideoCapture cap(input_string); // Grab the image
        if (!cap.isOpened()) {
            cerr << "OpenCV Failed to open file " + input_string << endl;
            exit(1);
        }
        
#endif // INPUT_FROM_CAMERA
        std::string warptx_cmd = prepareWarpTransformCommand(FRAME_WIDTH, FRAME_HEIGHT, send.channels(), transformation_matrix_float);
        
        //frame = cv::imread(argv[3], cv::IMREAD_GRAYSCALE); // reading in the image in grey scale
        unsigned int num_frame = 0;

        while (1) {
            clock_t start_cycle_main = clock();
            cap >> frame;
            if (frame.empty()) break; // if input is an image, the loop will be executed once
            if(frame.size().width==0) continue; //simple integrity check; skip erroneous data...
            cout << " ___________________________________________________________________ " << endl;
            cout << "/                                                                   \\" << endl;
            cout << "INFO: Frame # " << ++num_frame << endl;
#if CV_MAJOR_VERSION < 4
            cv::cvtColor(frame,frame,CV_BGR2GRAY);
#else
            cv::cvtColor(frame,frame,cv::COLOR_BGR2GRAY);
#endif
            resizeCropSquare(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), INTER_LINEAR);
            if ((frame.cols != FRAME_WIDTH) || (frame.rows != FRAME_HEIGHT)) {
                cout << "WARNING: Input frame was resized from " << frame.cols << "x" 
                        << frame.rows << " to " << send.cols << "x" << send.rows << endl;
            }
                if(argc < 5){
                imwrite("testimg.png", frame);
                }

            assert(send.total() == FRAME_WIDTH * FRAME_HEIGHT);
            // Ensure that the selection of MTU is a multiple of 8 (Bytes per transaction)
            assert(PACK_SIZE % 8 == 0);

#ifdef SHOW_WINDOWS	    
            namedWindow("host_send", CV_WINDOW_NORMAL);
            imshow("host_send", send);

#endif // SHOW_WINDOWS
    
            // Ensure that the send Mat is in continuous memory space. Typically, imread or resize 
            // will return such a continuous Mat, but we should check it.
            assert(send.isContinuous());

            unsigned int send_total = send.total();
            unsigned int send_channels = send.channels();
            unsigned int warptx_cmd_size = warptx_cmd.length();
#else // PY_WRAP == PY_WRAP_WARPTRANSFORM_NUMPI
////TODO: warptx cmds
            unsigned int send_total = (unsigned int)total_size +  warptx_cmd_size; //TBC!!!!!!!!!!!!!!!!!
            unsigned int send_channels = 1; // FIXME: It is ok only for 1-d array, i.e. CV_8UC1
            unsigned char * sendarr = input_img;
#endif // #if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)

            unsigned int total_pack  = 1 + (send_total * send_channels - 1 +  warptx_cmd_size) / PACK_SIZE;
            unsigned int total_bytes = total_pack * PACK_SIZE;
            unsigned int bytes_in_last_pack = send_total * send_channels +  warptx_cmd_size - (total_pack - 1) * PACK_SIZE;
            assert(total_pack == TOT_TRANSFERS_TX);

            unsigned int total_pack_rx  = 1 + (send_total * send_channels - 1) / PACK_SIZE;
            unsigned int total_bytes_rx = total_pack_rx * PACK_SIZE;
            unsigned int bytes_in_last_pack_rx = send_total * send_channels - (total_pack_rx- 1) * PACK_SIZE;
            assert(total_pack_rx == TOT_TRANSFERS_RX);

            //unsigned char * longbuf = new unsigned char[PACK_SIZE * total_pack];
	    unsigned char * longbuf = (unsigned char *) malloc (PACK_SIZE * total_pack_rx * sizeof (unsigned char));

            cout << "INFO: FPGA destination : " << servAddress << ":" << servPort << endl;
            cout << "INFO: Network socket   : " << ((NET_TYPE == tcp) ? "TCP" : "UDP") << endl;
            cout << "INFO: Total packets to send= " << total_pack << endl;
            cout << "INFO: Total packets to receive = " << total_pack_rx << endl;
            cout << "INFO: Total bytes to send   = " << send_total * send_channels +  warptx_cmd_size << endl;
            cout << "INFO: Total bytes to receive   = " << send_total * send_channels << endl;
            cout << "INFO: Total bytes in " << total_pack << " packets = "  << total_bytes << endl;
            cout << "INFO: Total bytes in " << total_pack_rx << " packets = "  << total_bytes_rx << endl;
            cout << "INFO: Bytes in last packet          = " << bytes_in_last_pack << endl;
            cout << "INFO: Bytes in last packet to receive    = " << bytes_in_last_pack_rx << endl;
            cout << "INFO: Packet size (custom MTU)      = " << PACK_SIZE << endl;

#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)
	    
            //--------------------------------------------------------
            //-- STEP-4 : RUN WARPTRANSFORM DETECTOR FROM OpenCV LIBRARY (SW)
            //--------------------------------------------------------
            clock_t start_cycle_warp_transform_sw = clock();
            ocv_out_img.create(send.rows, send.cols, INPUT_TYPE_HOST); // create memory for opencv output image
            ocv_ref(send, ocv_out_img, transformation_matrix);
            clock_t end_cycle_warp_transform_sw = clock();
            double duration_warp_transform_sw = (end_cycle_warp_transform_sw - start_cycle_warp_transform_sw) / 
                                            (double) CLOCKS_PER_SEC;
            cout << "INFO: SW exec. time:" << duration_warp_transform_sw << " seconds" << endl;
            cout << "INFO: Effective FPS SW:" << (1 / duration_warp_transform_sw) << " \tkbps:" << 
                    (PACK_SIZE * total_pack / duration_warp_transform_sw / 1024 * 8) << endl;

            //------------------------------------------------------
            //-- STEP-5 : RUN WARPTRANSFORM DETECTOR FROM cF (HW)
            //------------------------------------------------------
    
            //------------------------------------------------------
            //-- STEP-5.1 : Preparation
            //------------------------------------------------------

            // Anchor a pointer on cvMat raw data
            unsigned char * sendarr_img = send.isContinuous()? send.data: send.clone().data;
            // unsigned char * sendarr = send.isContinuous()? send.data: send.clone().data;
            // warptx_cmd = warptx_cmd.append(sendarr_img, send_total * send_channels);
	        unsigned char * sendarr = (unsigned char *) malloc (send_total * send_channels +  warptx_cmd_size);
            memcpy(sendarr,warptx_cmd.c_str(), warptx_cmd_size);
            memcpy(sendarr+warptx_cmd_size,sendarr_img, send_total * send_channels);
            
#endif // !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)
    
            clock_t start_cycle_warp_transform_hw = clock();

            //------------------------------------------------------
            //-- STEP-5.2 : TX Loop
            //------------------------------------------------------
            clock_t last_cycle_tx = clock();
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
                //delay(5);  
            }
            
            clock_t next_cycle_tx = clock();
            double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
                 total_pack / duration_tx / 1024 * 8) << endl;
            last_cycle_tx = next_cycle_tx;
        
        
            //------------------------------------------------------
            //-- STEP-5.3 : RX Loop
            //------------------------------------------------------    
            clock_t last_cycle_rx = clock();
            unsigned int receiving_now = PACK_SIZE;
            cout << "INFO: Expecting length of packs:" << total_pack_rx << " from " <<  servAddress << ":" << servPort << endl;
            //unsigned char * longbuf = new unsigned char[PACK_SIZE * total_pack];
            unsigned int loopi=0;
            for (unsigned int i = 0; i < send_total; ) {
                //cout << "DEBUG: i=" << i << ", loopi=" << loopi++ << endl;
                //if ( i == total_pack - 1 ) {
                //    receiving_now = bytes_in_last_pack;
                //}
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
                //cout << "DEBUG: i=" << i << " recvMsgSize=" << recvMsgSize << endl;
                i += recvMsgSize;
                //delay(5);
            }
            cout << "INFO: Received packet from " << servAddress << ":" << servPort << endl;

            clock_t next_cycle_rx = clock();
            double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * 
                    total_pack_rx / duration_rx / 1024 * 8) << endl;
            last_cycle_rx = next_cycle_rx;

            clock_t end_cycle_warp_transform_hw = next_cycle_rx;

            double duration_warp_transform_hw = (end_cycle_warp_transform_hw - start_cycle_warp_transform_hw) / 
                                                (double) CLOCKS_PER_SEC;
            cout << "INFO: HW exec. time:" << duration_warp_transform_hw << " seconds" << endl;
            cout << "INFO: Effective FPS HW:" << (1 / duration_warp_transform_hw) << " \tkbps:" << 
                    (PACK_SIZE * total_pack / duration_warp_transform_hw / 1024 * 8) << endl;
                    ////TODO: do we account for the cmd or not? if not is total_pack_rx
                    
#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)

            frame = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, INPUT_TYPE_HOST, longbuf); // OR vec.data() instead of ptr
            if (frame.size().width == 0) {
                cerr << "receive failure!" << endl;
                continue;
            }
#ifdef SHOW_WINDOWS            
            namedWindow("host_recv", CV_WINDOW_NORMAL);
            imshow("host_recv", frame);
#endif

            //------------------------------------------------------
            //-- STEP-6 : Write output files and show in windows
            //------------------------------------------------------

            ostringstream oss;
//            oss << "cFp_Vitis E2E:" << "INFO: Effective FPS HW:" << (1 / duration_warp_transform_hw) << 
//                   " \tkbps:" << (PACK_SIZE * total_pack / duration_warp_transform_hw / 1024 * 8);
            string windowName = "cFp_Vitis End2End"; //oss.str();

            //moveWindow(windowName, 0, 0);
#ifdef WRITE_OUTPUT_FILE
            if (num_frame == 1) {
                out_img_file.assign(out_filename);
                out_img_file += "_fpga_out_frame_" + to_string(num_frame) + ".png";
#if defined(PY_WRAP) && (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)

                if (!strcpy(output_img_str, &out_img_file[0])) {
                    cerr << "ERROR: Cannot write to output image string." << endl;
                }
#endif // defined(PY_WRAP) && (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)
                cout << "INFO: The output image file is stored at : " << out_img_file << endl; 
                // We save the image received from network after being processed by WarpTransform HW or HOST TB
                imwrite(out_img_file, frame);
            }
            else if (num_frame > 1) {
                // If the frame is empty, break immediately
                if (frame.empty()) {
                    break;
                }
                cout << "INFO: The output video file is stored at  : " << out_video_file << endl;
                Mat tovideo;
                if (frame.channels() != 1) {
                    tovideo = frame;
                }
                else {
                    cvtColor(frame, tovideo, COLOR_GRAY2BGR);        
                }
                video.write(tovideo);
            }
#endif // WRITE_OUTPUT_FILE
            waitKey(FRAME_INTERVAL);
            double duration_main = (clock() - start_cycle_main) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS E2E:" << (1 / duration_main) << endl;
            cout << "\\___________________________________________________________________/" << endl
            << endl;
            //delete(longbuf);
	    free (sendarr);
	    free (longbuf);
        } // while loop
	
        // When everything done, release the video capture and write object
        cap.release();
        video.release();

        // Closes all the windows
        destroyAllWindows();

#else  // !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)
        //output_img = longbuf;
        memcpy( output_img, longbuf, total_size);
	free(longbuf);
#endif // !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_WARPTRANSFORM_FILENAME)

        // Destructor closes the socket
    } catch (SocketException & e) {
        cerr << e.what() << endl;
        exit(1);
    }
    
#ifndef PY_WRAP
    return 0;
#endif
}




/*! \} */
