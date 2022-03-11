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
import numpy as np
import cv2
import socket



def median_blur(input_array, total_size, fpga_ip, fpga_port):
    bytesToSend = input_array.tostring()
    #input_array = np.ones((1, 60))
    # Create a UDP socket at client side
    UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    BUFF_SIZE = 1024#65536
    #UDPClientSocket.setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF,BUFF_SIZE)
    serverAddressPort   = (fpga_ip, fpga_port)

    cnt = 0;
    while True:
        print("INFO: Sending bytes: " + str(cnt*BUFF_SIZE) + " : " + str((cnt+1)*BUFF_SIZE-1))
       
        UDPClientSocket.sendto(bytesToSend[cnt*BUFF_SIZE:(cnt+1)*BUFF_SIZE], serverAddressPort)
        if ((cnt+1)*BUFF_SIZE >= total_size):
            print("INFO: Reached size to sent")
            break;
        else:
            cnt = cnt + 1

    cnt = 0;
    output_array = np.zeros((total_size,))
    while True:
        print("INFO: Receiving bytes: " + str(cnt*BUFF_SIZE) + " : " + str((cnt+1)*BUFF_SIZE-1))
       
        msgFromServer = UDPClientSocket.recvfrom(BUFF_SIZE)
        print(input_array.dtype)
        y = np.frombuffer(msgFromServer[0], dtype=input_array.dtype)
        
        print(output_array[cnt*BUFF_SIZE:(cnt+1)*BUFF_SIZE-1].size)
        output_array[cnt*BUFF_SIZE:(cnt+1)*BUFF_SIZE] = y

        if ((cnt+1)*BUFF_SIZE >= total_size):
            print("INFO: Reached size to receive")
            break;
        else:
            cnt = cnt + 1

    return output_array




if __name__ == '__main__':
    main(args)
