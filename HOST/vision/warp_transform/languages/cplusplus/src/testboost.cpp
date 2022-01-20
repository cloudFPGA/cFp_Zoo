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
#include <omp.h>

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
    if ((argc < 3) || (argc > 5)) { // Test for correct number of arguments
        cerr << "Usage: " << argv[0] << " <input folder> <output folder> <optional number of threads> <optional warp-transform mode>\n";
        //TODO: maybe not having optional but let the code read always the tx matrix
        exit(1);
    }
    //------------------------------------------------------
    //-- STEP-1 : Init
    //------------------------------------------------------
    
    assert ((argc == 3) || (argc == 4) || (argc == 5));
    printf("%d\n",argc);

    string strInFldr, strOutFldr, strNrThrd="", strWaxMode ="";
    printf("gno\n");
    strInFldr.assign(argv[1]);
    printf("gno\n");
    strOutFldr.assign(argv[2]);
    if(argc>=4){
    printf("gno\n");
    strNrThrd.assign(argv[3]);
    }
    if(argc==5){
    printf("gno\n");
    strWaxMode.assign(argv[4]);
    }
    printf("gno\n");
    unsigned int thread_number = 4;
    unsigned int wax_mode = 1;
//Setup   
    printf("setuop\n");
	try{
		thread_number = stoul(strNrThrd);
	} catch  (const std::exception& e) {
		std::cerr << e.what() << '\n';
		cout << "WARNING something bad happened in the thread insertion, hence default used" << endl;
		thread_number = 4;
	}
    printf("setuop\n");

    try{
		wax_mode = stoul(strWaxMode);
	} catch  (const std::exception& e) {
		std::cerr << e.what() << '\n';
		cout << "WARNING something bad happened in the thread insertion, hence default used" << endl;
		thread_number = 1;
	}
    printf("setuop\n");

    print_cFpZoo();
  
    /////////////////
    std::vector<fs::path> dataset_imgs = get_all(strInFldr, ".png");
  for(std::vector<fs::path>::const_iterator it = dataset_imgs.begin(); it != dataset_imgs.end(); ++it)
  {
      std::cout << (*it).string() << std::endl;
  }
    
    return 0;
}
/*! \} */