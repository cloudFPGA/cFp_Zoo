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
 * @file       warp_transform_host_sw_pure.cpp
 * @brief      WarpTransform software for benchamrking
 *
 * @date       Jan 2022
 * @author     DCO
 * 
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
#include "config.h"
#include "warp_transform_api.hpp"
#include "PracticalSockets.h" // For UDPSocket and SocketException
#include "util.hpp"
#include <omp.h>
#include <chrono>
#include <thread>         // std::thread

//decide wether use thread or openmp runtimes
// #define USE_OPENMP
#include "cv_warp_transform_config.hpp"

using namespace cv;
using namespace std;

#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED 
//#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
//#undef BOOST_NO_CXX11_SCOPED_ENUMS


namespace fs = boost::filesystem;

/**
 * \brief   Return the filenames of all files that have the specified extension
 *          in the specified directory and all subdirectories.
 */
std::vector<fs::path> get_all(fs::path const & root, std::string const & ext)
{
    std::vector<fs::path> paths;

    if (fs::exists(root) && fs::is_directory(root))
    {
        for (auto const & entry : fs::recursive_directory_iterator(root))
        {
            if (fs::is_regular_file(entry) && entry.path().extension() == ext)
                paths.emplace_back(entry.path().filename());
        }
    }
    return paths;
}  

//TODO: template
std::vector<fs::path> extract_subvector(std::vector<fs::path> myVec,int start, int end)
{
    std::vector<fs::path>::const_iterator first = myVec.begin() + start;
    std::vector<fs::path>::const_iterator last = myVec.begin() + end;
    std::vector<fs::path> subVec(first, last);
    // std::cout << start << " " << end << std::endl;
    return subVec;
}

//TODO: template
std::vector<std::vector<fs::path>> split_images(int thr_nr, std::vector<fs::path> dataset_images){
    std::vector<fs::path> tmp;
    std::vector<std::vector<fs::path>> out;
    int start,end = 0;
    int img_per_threads = dataset_images.size() / thr_nr;
    for (size_t i = 0; i < thr_nr; i++)
    {
        tmp.clear();
        start = img_per_threads*i;
        end = i <  thr_nr - 1 ? img_per_threads* ( i + 1 ) : dataset_images.size() ;
        tmp = extract_subvector(dataset_images, start, end);
        out.push_back(tmp);
    }
    return out;   
}

void splitString(string s, vector<string> &outVect, char separator){
	string temp = "";
	for(int i=0;i<s.length();i++){
		if(s[i]==separator){
			outVect.push_back(temp);
			temp = "";
		}
		else{
			temp.push_back(s[i]);
		}
		
	}
	outVect.push_back(temp);
}

//TODO: write here the images or wait to write em after the execution?
// as well as the reading phase.
// TBD
// template <typename T>
void wax_on_vec_imgs( std::string strInFldr, std::vector<fs::path> input_imgs, float* transformation_matrix_float, std::string strOutFldr, int start_cntr){
    Mat frame, send(FRAME_WIDTH, FRAME_HEIGHT, INPUT_TYPE_HOST, Scalar(0)), ocv_out_img;
    cv::Mat transformation_matrix(TRMAT_DIM1, TRMAT_DIM2, CV_32FC1, transformation_matrix_float);

    int cntr=start_cntr;
    for(std::vector<fs::path>::const_iterator it = input_imgs.begin(); it != input_imgs.end(); ++it, cntr++){
        //if vec of images this will change
        // std::cout << "Start cntr= "<< start_cntr << " img " << (*it).string() << std::endl;
        frame = cv::imread(strInFldr+(*it).string()); //, cv::IMREAD_GRAYSCALE); // reading in the image in grey scale
#if CV_MAJOR_VERSION < 4
            cv::cvtColor(frame,frame,CV_BGR2GRAY);
#else
            cv::cvtColor(frame,frame,cv::COLOR_BGR2GRAY);
#endif
            resizeCropSquare(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), INTER_LINEAR);
            ocv_out_img.create(send.rows, send.cols, INPUT_TYPE_HOST); // create memory for opencv output image
            ocv_ref(send, ocv_out_img, transformation_matrix);
            const string outfilename = strOutFldr + "wax-out-"+std::to_string(cntr)+".jpg";
            imwrite(outfilename, ocv_out_img);
    }
}


//TODO: write here the images or wait to write em after the execution?
// as well as the reading phase.
// TBD
// template <typename T>
void cf_wax_on_vec_imgs( std::string strInFldr, std::vector<fs::path> input_imgs,
float* transformation_matrix_float, std::string strOutFldr, int start_cntr, unsigned int  wax_mode,
std::string cf_ip, std::string cf_port){
    Mat frame, send(FRAME_WIDTH, FRAME_HEIGHT, INPUT_TYPE_HOST, Scalar(0)), ocv_out_img;

    int cntr=start_cntr;
    for(std::vector<fs::path>::const_iterator it = input_imgs.begin(); it != input_imgs.end(); ++it, cntr++){
        //if vec of images this will change
        //std::string str_command = "nohup ./warp_transform_host " + cf_ip + " " + cf_port +  " " + strInFldr+(*it).string() +  " " +  strOutFldr + " " + to_string(wax_mode)  + " </dev/null >/dev/null 2>&1 &";
        std::string str_command = "./warp_transform_host_lightweight " + cf_ip + " " + cf_port +  " " + strInFldr+(*it).string() +  " " +  strOutFldr + " " + to_string(wax_mode)  + "";
        //std::string str_command = "nohup ./warp_transform_host " + cf_ip + " " + cf_port +  " " + strInFldr+(*it).string() +  " " +  strOutFldr + " " + to_string(wax_mode)  + " &>/dev/null & > /dev/null 2>&1";
        const char *command = str_command.c_str(); 
        //cout << "Calling CF with command:" << command << endl; 
	system(command); 
     }
}


//TODO: write here the images or wait to write em after the execution?
// as well as the reading phase.
// TBD
// template <typename T>
void cf_wax_on_vec_imgs_apis( std::string strInFldr, std::vector<fs::path> input_imgs,
float* transformation_matrix_float, std::string strOutFldr, int start_cntr,
std::string cf_ip, std::string cf_port){
    // unsigned short servPort;
    // #if NET_TYPE == udp
    // UDPSocket my_socket;
    // servPort= open_connection<UDPSocket>(cf_ip, cf_port,my_socket);
    // #else // tcp
    // TCPSocket my_socket;
    // servPort= open_connection<TCPSocket>(cf_ip, cf_port,my_socket);
    // #endif 
    Mat frame, send(FRAME_WIDTH, FRAME_HEIGHT, INPUT_TYPE_HOST, Scalar(0)), ocv_out_img;
    int cntr=start_cntr;
    for(std::vector<fs::path>::const_iterator it = input_imgs.begin(); it != input_imgs.end(); ++it, cntr++){
        //if vec of images this will change
        std::cout << "Thread " << cf_ip << " proc " << cntr << std::endl;
        frame = cv::imread(strInFldr+(*it).string()); //, cv::IMREAD_GRAYSCALE); // reading in the image in grey scale
        ocv_out_img.create(FRAME_WIDTH, FRAME_HEIGHT, INPUT_TYPE_HOST); // create memory for opencv output image
        // cF_host_warp_transform(cf_ip, cf_port, frame, transformation_matrix_float, ocv_out_img, std::move(my_socket) ,servPort);
        cF_host_warp_transform(cf_ip, cf_port, frame, transformation_matrix_float, ocv_out_img);//, my_socket ,servPort);
        const string outfilename = strOutFldr + "wax-cfout-"+std::to_string(cntr)+".jpg";
        imwrite(outfilename, ocv_out_img);
        std::cout << "Thread " << cf_ip << " done processing " << std::endl;
        
    }
}

void setupTxMatrix(float transformation_matrix_float [9],unsigned int wax_mode)
{
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
        // std::copy(std::begin(square_reduction), std::end(square_reduction), std::begin(transformation_matrix_float));
        memcpy(transformation_matrix_float,square_reduction, sizeof(float)*9);
        break;
    case 2:
    //yscale_tx_mat
        // std::copy(std::begin(yscale_tx_mat), std::end(yscale_tx_mat), std::begin(transformation_matrix_float));
        memcpy(transformation_matrix_float,square_reduction, sizeof(float)*9);
        break;
    case 3:
    //xscale_tx_mat
        // std::copy(std::begin(xscale_tx_mat), std::end(xscale_tx_mat), std::begin(transformation_matrix_float));
        memcpy(transformation_matrix_float,square_reduction, sizeof(float)*9);
        break;
    case 4:
    //rotation_30degree_tx_mat
        // std::copy(std::begin(rotation_30degree_tx_mat), std::end(rotation_30degree_tx_mat), std::begin(transformation_matrix_float));
        memcpy(transformation_matrix_float,square_reduction, sizeof(float)*9);
        break;
    case 5:
    //xtranslation_tx_mat
        // std::copy(std::begin(xtranslation_tx_mat), std::end(xtranslation_tx_mat), std::begin(transformation_matrix_float));
        memcpy(transformation_matrix_float,square_reduction, sizeof(float)*9);
        break;
    case 6:
    //ytranslation_tx_mat
        // std::copy(std::begin(ytranslation_tx_mat), std::end(ytranslation_tx_mat), std::begin(transformation_matrix_float));
        memcpy(transformation_matrix_float,square_reduction, sizeof(float)*9);
        break;
    case 7:
    //shearing_tx_mat
        // std::copy(std::begin(shearing_tx_mat), std::end(shearing_tx_mat), std::begin(transformation_matrix_float));
        memcpy(transformation_matrix_float,square_reduction, sizeof(float)*9);
        break;                
    case 8:
    //reflection_tx_mat
        // std::copy(std::begin(reflection_tx_mat), std::end(reflection_tx_mat), std::begin(transformation_matrix_float));
        memcpy(transformation_matrix_float,square_reduction, sizeof(float)*9);
        break;     
    default:
    //identity
        // std::copy(std::begin(identity), std::end(identity), std::begin(transformation_matrix_float));
        memcpy(transformation_matrix_float,square_reduction, sizeof(float)*9);
        break;
    }
    // cv::Mat transformation_matrix(TRMAT_DIM1, TRMAT_DIM2, CV_32FC1, transformation_matrix_float);
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
	cout <<  "Warp transform pure software version                      " << endl;
}


  /**
   *   Main testbench for WarpTransform software version
   *   @return O on success, 1 on fail 
   */
int main(int argc, char * argv[]) {
    if ((argc < 4) || (argc > 8)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <input folder> <output folder> <0|1|2 sw|cf api|cf exec> <optional number of threads> \n";
        cerr << " <optional warp-transform mode> <port list 1234:5678:...> <ip list 10.12.200.222:10.12.200.54:10.12.200.128:10.12.200.127> \n";
        //TODO: maybe not having optional but let the code read always the tx matrix
        exit(1);
    }
    //------------------------------------------------------
    //-- STEP-1 : Init
    //------------------------------------------------------
    
    assert ((argc  >= 4) ||  (argc <= 8) );

    

    string strInFldr, strOutFldr, strExeMode, strNrThrd="", strWaxMode="", strPorts="", strIps="";
    vector<string> ipsVect;
    vector<string> portsVect;
    //vector<string> ipsVect({"10.12.200.222","10.12.200.54","10.12.200.128","10.12.200.127"});
    //vector<string> ipsVect({"10.12.200.145"});
    // vector<string> ipsVect({"localhost","localhost","localhost","localhost",
    // "localhost","localhost","localhost","localhost"});
    //vector<string> portsVect({"2718", "2718", "2718", "2718"});
    //vector<string> portsVect({"1234", "5678","9101","1121",
    // "3141","5161","7181","9202"});
    strInFldr.assign(argv[1]);
    strOutFldr.assign(argv[2]);
    strExeMode.assign(argv[3]);
    if(argc>=5){
        strNrThrd.assign(argv[4]);
        if(argc>=6){
            strWaxMode.assign(argv[5]);
        }

    }
    if(argc >= 7){
        strPorts.assign(argv[6]);
        splitString(strPorts, portsVect, ':');
        if(argc == 8){
            strIps.assign(argv[7]);
            splitString(strIps, ipsVect, ':');
        }else{
            vector<string> tmp({"10.12.200.63","10.12.200.135","10.12.200.19","10.12.200.216"});
            ipsVect = tmp;
        }
    }else{
        vector<string> tmp1({"2718", "2718", "2718", "2718"});
        vector<string> tmp2({"10.12.200.63","10.12.200.135","10.12.200.19","10.12.200.216"});
        portsVect = tmp1;
        ipsVect = tmp2;
    }

    unsigned int thread_number = 4;
    unsigned int wax_mode = 2;
    unsigned int exe_mode = 0;
//Setup
	try{
		exe_mode = stoul(strExeMode);
	} catch  (const std::exception& e) {
		std::cerr << e.what() << '\n';
		cout << "WARNING something bad happened in the execution insertion, hence CPU used" << endl;
		exe_mode = 0;
	}
    assert((exe_mode == 0) || (exe_mode >= 2));
	try{
		thread_number = stoul(strNrThrd);
	} catch  (const std::exception& e) {
		std::cerr << e.what() << '\n';
		cout << "WARNING something bad happened in the thread insertion, hence default used" << endl;
		thread_number = 4;
	}

    try{
		wax_mode = stoul(strWaxMode);
	} catch  (const std::exception& e) {
		std::cerr << e.what() << '\n';
		cout << "WARNING something bad happened in the wax mode insertion, hence default used" << endl;
		wax_mode = 2;
	}
    if(exe_mode!= 0){
        assert(portsVect.size() == ipsVect.size());
        assert(ipsVect.size() >= thread_number);
    }
    print_cFpZoo();
    #ifdef USE_OPENMP
    std::cout << "Using openmp runtime" << std::endl;
    #else
    std::cout << "Using the threads runtime" << std::endl;
    #endif // USE_OPENMP
    float transformation_matrix_float [9]= {1,0,0,0,1,0,0,0,0};
    setupTxMatrix(transformation_matrix_float, wax_mode);
    
    /////////////////
    std::vector<fs::path> dataset_imgs = get_all(strInFldr, ".png");
    std::vector<std::vector<fs::path>> imgs_splitted = split_images(thread_number, dataset_imgs);

    int iam, startcntr;
    clock_t start_cycle_warp_transform_sw = clock();
    int img_per_threads = dataset_imgs.size() / thread_number;
    std::vector<fs::path> tmp;
    auto sTime = std::chrono::high_resolution_clock::now();
#ifndef USE_OPENMP
    std::vector<std::thread> vectThreads;
    for (size_t i = 0; i < thread_number; i++)
    {   
        iam = i;
        startcntr =  iam!=0 ? img_per_threads*iam: 0;
        tmp = imgs_splitted.at(iam);
        if(exe_mode == 0){
            std::thread tmpThr(wax_on_vec_imgs,strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr);
            vectThreads.push_back(std::move(tmpThr));
        }else if(exe_mode == 1){
            std::thread tmpThr(cf_wax_on_vec_imgs_apis,strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr,
            ipsVect.at(iam), portsVect.at(iam));
            vectThreads.push_back(std::move(tmpThr));
	    }else{
            // cf_wax_on_vec_imgs(strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr,
            //cf_wax_on_vec_imgs_apis(strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr,
            std::thread tmpThr(cf_wax_on_vec_imgs,strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr, wax_mode,
            ipsVect.at(iam), portsVect.at(iam));
            vectThreads.push_back(std::move(tmpThr));
        }   
    }
    for (size_t i = 0; i < thread_number; i++)
    {
        if(vectThreads.at(i).joinable()) {
            vectThreads.at(i).join();
        }
    }
#else
    omp_set_dynamic(0);
#pragma omp parallel default(shared) private(tmp,iam,startcntr) num_threads(thread_number) 
    {
        iam = omp_get_thread_num();
        // std::cout << " How many images allocated " << img_per_threads << std::endl;
        startcntr =  iam!=0 ? img_per_threads*iam: 0;
        tmp = imgs_splitted.at(iam);
        // std::cout << " Iam " << iam << " cntr " << startcntr << " size of vec " << tmp.size() <<   std::endl;
        if(exe_mode == 0){
            wax_on_vec_imgs(strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr);
        }else if(exe_mode == 1){
            cf_wax_on_vec_imgs_apis(strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr,
            ipsVect.at(iam), portsVect.at(iam));
	    }else{
            // cf_wax_on_vec_imgs(strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr,
            //cf_wax_on_vec_imgs_apis(strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr,
            cf_wax_on_vec_imgs(strInFldr, tmp, transformation_matrix_float, strOutFldr, startcntr, wax_mode,
            ipsVect.at(iam), portsVect.at(iam));

        }
	}
#endif //USE_OPENMP
    clock_t end_cycle_warp_transform_sw = clock();
    auto sfinish = std::chrono::high_resolution_clock::now();
    std::cout << "INFO: chrono time [ms] =" <<  std::chrono::duration_cast<std::chrono::milliseconds>(sfinish-sTime).count()<<"\n";
    double duration_warp_transform_sw = (end_cycle_warp_transform_sw - start_cycle_warp_transform_sw) / (double) CLOCKS_PER_SEC;
    std::cout << "INFO: SW exec. time:" << duration_warp_transform_sw << " seconds" << endl; 
    
    return 0;
}
/*! \} */