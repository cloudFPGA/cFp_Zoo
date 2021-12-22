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
#include <string>                       // For to_string
#include <string.h>                     // For memcpy()
#include "../../../../../PracticalSockets/src/PracticalSockets.h" // For UDPSocket and SocketException
#include "../include/config.h"

#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_HARRIS_FILENAME)
#include "opencv2/opencv.hpp"
#include "../../../../../../ROLE/vision/hls/harris/include/xf_ocv_ref.hpp"  // For SW reference Harris from OpenCV
using namespace cv;
#endif

using namespace std;


void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

void print_cFpVitis(void)
{
    cout <<  "                                                          " << endl;
    cout <<  "...build with:                                            " << endl;
    cout <<  " ██████╗███████╗██████╗   ██╗   ██╗██╗████████╗██╗███████╗" << endl;
    cout <<  "██╔════╝██╔════╝██╔══██╗  ██║   ██║██║╚══██╔══╝██║██╔════╝" << endl;
    cout <<  "██║     █████╗  ██████╔╝  ██║   ██║██║   ██║   ██║███████╗" << endl;
    cout <<  "██║     ██╔══╝  ██╔═══╝   ╚██╗ ██╔╝██║   ██║   ██║╚════██║" << endl;
    cout <<  "╚██████╗██║     ██║███████╗╚████╔╝ ██║   ██║   ██║███████║" << endl;
    cout <<  " ╚═════╝╚═╝     ╚═╝╚══════╝ ╚═══╝  ╚═╝   ╚═╝   ╚═╝╚══════╝" << endl;
    cout <<  "A cloudFPGA project from IBM ZRL               v1.0 --did " << endl;
    cout <<  "                                                          " << endl;
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

#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_HARRIS_FILENAME)

/*****************************************************************************
 * @brief Mark the points found by Harris into the image.
 * 
 * @return Nothing
 ******************************************************************************/
void markPointsOnImage(Mat& imgOutput, 
		       Mat& in_img, 
		       Mat& out_img, 
		       vector<Point>& hw_points) 
{

   for (int j = 0; j < imgOutput.rows; j++) {
      for (int i = 0; i < (imgOutput.cols); i++) {
	  //for CV_8UC1
          unsigned char pix = imgOutput.at<unsigned char>(j,i);  //.read(j * (imgOutput.cols) + i);
          if (pix != 0) {
	      Point tmp;
              tmp.x = i;
              tmp.y = j;
              if ((tmp.x < in_img.cols) && (tmp.y < in_img.rows) && (j > 0)) {
		  hw_points.push_back(tmp);
	      }
              short int y, x;
              y = j;
              x = i;
              if (j > 0) circle(out_img, Point(x, y), 2, Scalar(0, 0, 255, 255), 1, 8, 0);
	  }
      }
   }
}

#endif

#ifdef PY_WRAP
#if PY_WRAP == PY_WRAP_HARRIS_FILENAME
void harris(char *s_servAddress, char *s_servPort, char *input_str, char *output_img_str, char *output_points_str)
#elif PY_WRAP == PY_WRAP_HARRIS_NUMPI
void harris(int total_size, unsigned char *input_img, int total_size2, unsigned char *output_img, char *s_servAddress, char *s_servPort)
#endif // PY_WRAP value
{
#else // !PY_WRAP
  /**
   *   Main testbench and user-application for Harris on host. Client
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {
    if ((argc < 3) || (argc > 4)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port> <optional input image>\n";
        exit(1);
    }
#endif // PY_WRAP
    
    //------------------------------------------------------
    //-- STEP-1 : Socket and variables definition
    //------------------------------------------------------
    
    #ifndef PY_WRAP
    assert ((argc == 3) || (argc == 4));
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
    string input_string;
#ifdef INPUT_FROM_CAMERA
    int input_num;
#ifdef PY_WRAP
#if PY_WRAP == PY_WRAP_HARRIS_FILENAME
    input_num = atoi(input_str);
    input_string = "./cam"+to_string(input_num);
#endif // PY_WRAP == PY_WRAP_HARRIS_FILENAME
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
#if PY_WRAP == PY_WRAP_HARRIS_FILENAME
    input_string.assign(input_str);
#endif // PY_WRAP == PY_WRAP_HARRIS_FILENAME    
#else // !PY_WRAP
    if (argc == 3) {
        // Give a default image
        input_string.assign("../../../../../../ROLE/vision/hls/harris/test/8x8.png");
    }
    else if (argc == 4) {
        input_string.assign(argv[3]);
    }
#endif // PY_WRAP
#endif // INPUT_FROM_CAMERA
    
   
#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_HARRIS_FILENAME)

   
    float Th;
    if (FILTER_WIDTH == 3) {
        Th = 30532960.00;
    } else if (FILTER_WIDTH == 5) {
        Th = 902753878016.0;
    } else if (FILTER_WIDTH == 7) {
        Th = 41151168289701888.000000;
    }
    string out_img_file, out_points_file;
    string out_video_file, out_video_points_file;
    // Define the codec and create VideoWriter object.The output is stored in 'outcpp.avi' file. 
    //#ifdef PY_WRAP
    //out_video_file.assign(output_str);
    //#else // !PY_WRAP
    out_video_file.assign(input_string);
    out_video_file += "_fpga_video_out.avi";
    out_video_points_file.assign(input_string);
    out_video_points_file += "_fpga_video_points_out.avi";
    //#endif // PY_WRAP
#if CV_MAJOR_VERSION < 4
    VideoWriter video(out_video_file,CV_FOURCC('M','J','P','G'),10, Size(FRAME_WIDTH,FRAME_HEIGHT));
    VideoWriter videop(out_video_points_file,CV_FOURCC('M','J','P','G'),10, Size(FRAME_WIDTH,FRAME_HEIGHT));    
#else
    VideoWriter video(out_video_file,cv::VideoWriter::fourcc('M','J','P','G'),10, Size(FRAME_WIDTH,FRAME_HEIGHT));
    VideoWriter videop(out_video_points_file,cv::VideoWriter::fourcc('M','J','P','G'),10, Size(FRAME_WIDTH,FRAME_HEIGHT));
#endif

#endif // #if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_HARRIS_FILENAME) 

    print_cFpVitis();
    
    try {
          
        //------------------------------------------------------
        //-- STEP-2 : Initialize socket connection
        //------------------------------------------------------      
	#if NET_TYPE == udp
        #ifndef TB_SIM_CFP_VITIS
        UDPSocket sock(servPort); // NOTE: It is very important to set port here in order to call 
	                          // bind() in the UDPSocket constructor
	#else // TB_SIM_CFP_VITIS
        UDPSocket sock; // NOTE: In HOST TB the port is already binded by harris_host_fwd_tb.cpp
	#endif // TB_SIM_CFP_VITIS
	#else // tcp
	TCPSocket sock(servAddress, servPort);
        #endif // udp/tcp
 

#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_HARRIS_FILENAME)


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
	    
#else // PY_WRAP == PY_WRAP_HARRIS_NUMPI
	    
	    unsigned int send_total = (unsigned int)total_size;
	    unsigned int send_channels = 1; // FIXME: It is ok only for 1-d array, i.e. CV_8UC1
	    unsigned char * sendarr = input_img;
#endif // #if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_HARRIS_FILENAME)

   
	    
            unsigned int total_pack  = 1 + (send_total * send_channels - 1) / PACK_SIZE;
            unsigned int total_bytes = total_pack * PACK_SIZE;
            unsigned int bytes_in_last_pack = send_total * send_channels - (total_pack - 1) * PACK_SIZE;
	    assert(total_pack == TOT_TRANSFERS);

            cout << "INFO: FPGA destination : " << servAddress << ":" << servPort << endl;
	    cout << "INFO: Network socket   : " << ((NET_TYPE == tcp) ? "TCP" : "UDP") << endl;
	    cout << "INFO: Total packets to send/receive = " << total_pack << endl;
            cout << "INFO: Total bytes to send/receive   = " << send_total * send_channels << endl;
	    cout << "INFO: Total bytes in " << total_pack << " packets = "  << total_bytes << endl;
	    cout << "INFO: Bytes in last packet          = " << bytes_in_last_pack << endl;
	    cout << "INFO: Packet size (custom MTU)      = " << PACK_SIZE << endl;
	    
#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_HARRIS_FILENAME)    
	    
            //--------------------------------------------------------
            //-- STEP-4 : RUN HARRIS DETECTOR FROM OpenCV LIBRARY (SW)
            //--------------------------------------------------------
            clock_t start_cycle_harris_sw = clock();
	    ocv_out_img.create(send.rows, send.cols, INPUT_TYPE_HOST); // create memory for opencv output image
	    ocv_ref(send, ocv_out_img, Th);
	    clock_t end_cycle_harris_sw = clock();
	    double duration_harris_sw = (end_cycle_harris_sw - start_cycle_harris_sw) / 
	                                (double) CLOCKS_PER_SEC;
	    cout << "INFO: SW exec. time:" << duration_harris_sw << " seconds" << endl;
            cout << "INFO: Effective FPS SW:" << (1 / duration_harris_sw) << " \tkbps:" << 
                    (PACK_SIZE * total_pack / duration_harris_sw / 1024 * 8) << endl;
	    
	    //------------------------------------------------------
            //-- STEP-5 : RUN HARRIS DETECTOR FROM cF (HW)
            //------------------------------------------------------
	    
	    //------------------------------------------------------
            //-- STEP-5.1 : Preparation
            //------------------------------------------------------

	    // Anchor a pointer on cvMat raw data
            unsigned char * sendarr = send.isContinuous()? send.data: send.clone().data;
 
            clock_t start_cycle_harris_hw = clock();

#endif // !PY_WRAP_HARRIS_NUMPI
    
	    
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
		//delay(1);  
	    }
            
            clock_t next_cycle_tx = clock();
            double duration_tx = (next_cycle_tx - last_cycle_tx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
                 total_pack / duration_tx / 1024 * 8) << endl;
            last_cycle_tx = next_cycle_tx;
	    
	    
	    //------------------------------------------------------
            //-- STEP-5.3 : RX Loop
            //------------------------------------------------------    
#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_HARRIS_FILENAME)
	    clock_t last_cycle_rx = clock();
#endif
	    unsigned int receiving_now = PACK_SIZE;
            cout << "INFO: Expecting length of packs:" << total_pack << " from " <<  servAddress << ":" << servPort << endl;
            unsigned char * longbuf = new unsigned char[PACK_SIZE * total_pack];
            for (unsigned int i = 0; i < send_total; ) {
	        //cout << "DEBUG: " << i << endl;
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
            }

            cout << "INFO: Received packet from " << servAddress << ":" << servPort << endl;

#if !defined(PY_WRAP) || (PY_WRAP == PY_WRAP_HARRIS_FILENAME)
	    
            frame = cv::Mat(FRAME_HEIGHT, FRAME_WIDTH, INPUT_TYPE_HOST, longbuf); // OR vec.data() instead of ptr
	    if (frame.size().width == 0) {
                cerr << "receive failure!" << endl;
                continue;

            }
#ifdef SHOW_WINDOWS            
            namedWindow("host_recv", CV_WINDOW_NORMAL);
            imshow("host_recv", frame);
#endif
            clock_t next_cycle_rx = clock();
            double duration_rx = (next_cycle_rx - last_cycle_rx) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS RX:" << (1 / duration_rx) << " \tkbps:" << (PACK_SIZE * 
                    total_pack / duration_rx / 1024 * 8) << endl;
            last_cycle_rx = next_cycle_rx;
	   
	    clock_t end_cycle_harris_hw = next_cycle_rx;
	    
	    double duration_harris_hw = (end_cycle_harris_hw - start_cycle_harris_hw) / 
	                                (double) CLOCKS_PER_SEC;
	    cout << "INFO: HW exec. time:" << duration_harris_hw << " seconds" << endl;
            cout << "INFO: Effective FPS HW:" << (1 / duration_harris_hw) << " \tkbps:" << 
                    (PACK_SIZE * total_pack / duration_harris_hw / 1024 * 8) << endl;	    
	    
	    //------------------------------------------------------
            //-- STEP-6 : Write output files and show in windows
            //------------------------------------------------------
            Mat out_img;
            out_img = send.clone();
            vector<Point> hw_points;
	    
	    /* Mark HLS points on the image */
	    markPointsOnImage(frame, send, out_img, hw_points);
	    
	    ostringstream oss;
            oss << "cFp_Vitis E2E:" << "INFO: Effective FPS HW:" << (1 / duration_harris_hw) << 
                   " \tkbps:" << (PACK_SIZE * total_pack / duration_harris_hw / 1024 * 8);
	    string windowName = "cFp_Vitis End2End"; //oss.str();
	    /*
	    string msg = "cFp_Vitis";
	    Scalar color(255, 0, 0);
	    int fontFace = FONT_HERSHEY_DUPLEX;
	    double fontScale = 0.5;
	    int thickness = 1;
	    putText(out_img,
            msg, //text
            Point(10, out_img.rows / 2), //top-left position
            fontFace,
            fontScale,
            color, //font color
            thickness);
	    */
#ifdef SHOW_WINDOWS
	    namedWindow(windowName, CV_WINDOW_NORMAL);
	    imshow(windowName, out_img);
#endif
	    //moveWindow(windowName, 0, 0);
#ifdef WRITE_OUTPUT_FILE
	    if (num_frame == 1) {
	      out_img_file.assign(input_string);
	      out_img_file += "_fpga_img_out_frame_" + to_string(num_frame) + ".png";
	      out_points_file.assign(input_string);
	      out_points_file += "_fpga_points_out_frame_" + to_string(num_frame) + ".png";
#if defined(PY_WRAP) && (PY_WRAP == PY_WRAP_HARRIS_FILENAME)

	      if (!strcpy(output_img_str, &out_img_file[0])) {
		  cerr << "ERROR: Cannot write to output image string." << endl;
	      }
	      if (!strcpy(output_points_str, &out_points_file[0])) {
		  cerr << "ERROR: Cannot write to output points string." << endl;
	      }
#endif // defined(PY_WRAP) && (PY_WRAP == PY_WRAP_HARRIS_FILENAME)
	      cout << "INFO: The output image file is stored at  : " << out_img_file << endl; 
	      cout << "INFO: The output points file is stored at : " << out_points_file << endl; 
	      // We save the image received from network after being processed by Harris HW or HOST TB
	      imwrite(out_img_file, out_img);
	      imwrite(out_points_file, frame);
	    }
	    else if (num_frame > 1) {
	      // If the frame is empty, break immediately
	      if (frame.empty()) {
		break;
	      }
	      cout << "INFO: The output video file is stored at  : " << out_video_file << endl;
	      cout << "INFO: The output video -only points- file is stored at  : " << out_video_points_file << endl;
	      Mat tovideo, tovideop;
	      if (frame.channels() != 1) {
		tovideo  = out_img;
		tovideop = frame;
	      }
	      else {
		cvtColor(out_img, tovideo, COLOR_GRAY2BGR);
		cvtColor(frame, tovideop, COLOR_GRAY2BGR);        
	      }
	      video.write(tovideo);
	      videop.write(tovideop);
	    }
#endif // WRITE_OUTPUT_FILE
	    waitKey(FRAME_INTERVAL);
            double duration_main = (clock() - start_cycle_main) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS E2E:" << (1 / duration_main) << endl;
            cout << "\\___________________________________________________________________/" << endl
            << endl;
	} // while loop
	
	// When everything done, release the video capture and write object
	cap.release();
	video.release();
    	videop.release();

        // Closes all the windows
	destroyAllWindows();

#else
	//output_img = longbuf;
	memcpy( output_img, longbuf, total_size);
    	delete(longbuf);
#endif // defined(PY_WRAP) && (PY_WRAP == PY_WRAP_HARRIS_FILENAME)
	
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
