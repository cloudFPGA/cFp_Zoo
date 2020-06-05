
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
#include "../../../hls/harris_app/include/xf_ocv_ref.hpp"  // For SW reference Harris from OpenCV

using namespace std;
using namespace cv;


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



  /**
   *   Main testbench and user-application for Harris on host. Client
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {
    if ((argc < 3) || (argc > 4)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <Server> <Server Port> <optional input image>\n";
        exit(1);
    }

    
    //------------------------------------------------------
    //-- STEP-1 : Socket and variables definition
    //------------------------------------------------------
    string servAddress = argv[1]; // First arg: server address
    unsigned short servPort = Socket::resolveService(argv[2], "udp");
    char buffer[BUF_LEN]; // Buffer for echo string
    unsigned int recvMsgSize; // Size of received message
    unsigned int num_frame = 0;
    float Th;
    if (FILTER_WIDTH == 3) {
        Th = 30532960.00;
    } else if (FILTER_WIDTH == 5) {
        Th = 902753878016.0;
    } else if (FILTER_WIDTH == 7) {
        Th = 41151168289701888.000000;
    }    
    print_cFpVitis();
    
    try {
          
        //------------------------------------------------------
        //-- STEP-2 : Initialize socket connection
        //------------------------------------------------------      
        #ifndef TB_SIM_CFP_VITIS
        UDPSocket sock(servPort); // NOTE: It is very important to set port here in order to call 
	                          // bind() in the UDPSocket constructor
	#else
        UDPSocket sock; // NOTE: In HOST TB the port is already binded by harris_host_fwd_tb.cpp
        #endif
        
        //---------------------------------------------------------------------------
        //-- STEP-3 : Initialize an OpenCV Mat either from image or from video/camera
        //---------------------------------------------------------------------------
        Mat frame, send, ocv_out_img;
        vector < uchar > encoded;
		
#ifdef INPUT_FROM_VIDEO
        VideoCapture cap(VIDEO_SOURCE); // Grab the camera
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
#ifdef INPUT_FROM_VIDEO	
        while (1) {
            cout << " ___________________________________________________________________ " << endl;
            cout << "/                                                                   \\" << endl;
	    cout << "INFO: Frame # " << num_frame++ << endl;
            clock_t start_cycle_main = clock();
            cap >> frame;
            if(frame.size().width==0)continue;//simple integrity check; skip erroneous data...
#endif
	    if ((frame.cols != FRAME_WIDTH) || (frame.rows != FRAME_HEIGHT)) {
		resize(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
	        cout << "WARNING: Input frame was resized from " << frame.cols << "x" 
		<< frame.rows << " to " << send.cols << "x" << send.rows << endl;
	    }
	    assert(send.total() == FRAME_WIDTH * FRAME_HEIGHT);
	    namedWindow("host_send", CV_WINDOW_NORMAL);
            imshow("host_send", send);
	    // Ensure that the send Mat is in continuous memory space. Typically, imread or resize 
	    // will return such a continuous Mat, but we should check it.
	    assert(send.isContinuous());
	    
            unsigned int total_pack  = 1 + (send.total() - 1) / PACK_SIZE;
            unsigned int total_bytes = total_pack * PACK_SIZE;
            unsigned int bytes_in_last_pack = send.total() - (total_pack - 1) * PACK_SIZE;
	    assert(total_pack == TOT_TRANSFERS);

	    cout << "INFO: Total packets to send/receive = " << total_pack << endl;
            cout << "INFO: Total bytes to send/receive   = " << send.total() << endl;
	    cout << "INFO: Total bytes in " << total_pack << " packets = "  << total_bytes << endl;
	    cout << "INFO: Bytes in last packet          = " << bytes_in_last_pack << endl;
	    cout << "INFO: Packet size (custom MTU)      = " << PACK_SIZE << endl;
	    
            //--------------------------------------------------------
            //-- STEP-4 : RUN HARRIS DETECTOR FROM OpenCV LIBRARY (SW)
            //--------------------------------------------------------
            clock_t start_cycle_harris_sw = clock();
	    ocv_out_img.create(send.rows, send.cols, CV_8U); // create memory for opencv output image
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
            clock_t start_cycle_harris_hw = clock();
	    
	    //------------------------------------------------------
            //-- STEP-5.1 : TX Loop
            //------------------------------------------------------
            clock_t last_cycle_tx = clock();
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
            cout << "INFO: Effective FPS TX:" << (1 / duration_tx) << " \tkbps:" << (PACK_SIZE * 
                 total_pack / duration_tx / 1024 * 8) << endl;
            last_cycle_tx = next_cycle_tx;
	    
	    
	    //------------------------------------------------------
            //-- STEP-5.2 : RX Loop
            //------------------------------------------------------
	    clock_t last_cycle_rx = clock();
	    unsigned int receiving_now = PACK_SIZE;
            cout << "INFO: Expecting length of packs:" << total_pack << endl;
            char * longbuf = new char[PACK_SIZE * total_pack];
            for (unsigned int i = 0; i < total_pack; i++) {
                if ( i == total_pack - 1 ) {
                    receiving_now = bytes_in_last_pack;
                }
                recvMsgSize = sock.recvFrom(buffer, BUF_LEN, servAddress, servPort);
                if (recvMsgSize != receiving_now) {
                    cerr << "Received unexpected size pack:" << recvMsgSize << ". Expected: " << 
                            receiving_now << endl;
#ifdef INPUT_FROM_VIDEO
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
#ifdef INPUT_FROM_VIDEO
                continue;
#else
		exit(1);
#endif
            }
            namedWindow("host_recv", CV_WINDOW_NORMAL);
            imshow("host_recv", frame);
            
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
	    namedWindow(windowName, CV_WINDOW_NORMAL);
	    imshow(windowName, out_img);
	    //moveWindow(windowName, 0, 0);
#ifndef INPUT_FROM_VIDEO	    
	    string out_img_file, out_points_file;
	    out_img_file.assign(argv[3]);
	    out_img_file += "_fpga_img_out.png";
	    out_points_file.assign(argv[3]);
	    out_points_file += "_fpga_points_out.png";
	    cout << "INFO: The output image file is stored at  : " << out_img_file << endl; 
	    cout << "INFO: The output points file is stored at : " << out_points_file << endl; 
	    // We save the image received from network after being processed by Harris HW or HOST TB
	    imwrite(out_img_file, out_img);
	    imwrite(out_points_file, frame);
#endif	    
	    waitKey(FRAME_INTERVAL);
	    clock_t next_cycle_main = clock();
            double duration_main = (next_cycle_main - start_cycle_main) / (double) CLOCKS_PER_SEC;
            cout << "INFO: Effective FPS E2E:" << (1 / duration_main) << endl;
            cout << "\\___________________________________________________________________/" << endl;
#ifdef INPUT_FROM_VIDEO
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