# *****************************************************************************
# *                            cloudFPGA
# *                Copyright 2016 -- 2022 IBM Corporation
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *     http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *----------------------------------------------------------------------------

## @file   trieres.py
## @author DID
## @date   February 2022
## @brief  A python script for testing the cF median_blur kernel. The script takes as argument the fpga 
##         ip, the port and the numpi array of an image to be processed. This array should be an 1-D 
##         array, containing all pixels of a CV MAT in CV_8UC1. The kernel will rerurn a numpi
##         array which is the array with only the detected points.

import sys
import os

cpwd = os.path.abspath(__file__)
vision_path = os.path.dirname(cpwd)+"/../vision"
sys.path.append(vision_path)

import vision

#def main(argv):
    
#    if 1:
#        quantitative_finance.median_blur(input_array, total_size, fpga_ip, fpga_port)


#if __name__ == '__main__':
#    if not (hasattr(sys, 'real_prefix') or (hasattr(sys, 'base_prefix') and sys.base_prefix != sys.prefix)):
#        # This works with virtualenv for Python 3 and 2 and also for the venv module in Python 3
#        print("ERROR: It looks like this trieres isn't running in a virtual environment. Aborting.")
#        sys.exit(1)
        
#    main(sys.argv)
#    exit(0)
