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

## @file   custom.py
## @author DID
## @date   October 2022
## @brief  A python script for testing the cF uppercase kernel. The script takes as argument the fpga 
##         ip, the port and the numpi array of a string to be processed. This array should be an 1-D 
##         array, containing all characters of a string. The kernel will rerurn a numpi
##         array which is the array with converted characters.

import sys
import os
import numpy as np
import socket
import logging

# Setting SO_RCVBUF
# Sets or gets the maximum socket receive buffer in bytes.  The kernel doubles
# this value (to allow space for bookkeeping overhead) when it is set using
# setsockopt(2), and this doubled  value  is  returned  by  getsockopt(2).
# The default value is set by the /proc/sys/net/core/rmem_default file, and
# the maximum allowed value is set by the /proc/sys/net/core/rmem_max file.
# The minimum (doubled) value for this option is 256.
recvBufSize = 0x1000000
real_buffer_size = 0

def uppercase(input_array, total_size, fpga_ip, fpga_port, debug_level):
    logging.basicConfig(level=debug_level)
    bytesToSend = input_array.tostring()
    # Create a UDP socket at client side
    UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    previous_buffer_size = UDPClientSocket.getsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF)
    UDPClientSocket.setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF, recvBufSize)
    real_buffer_size = UDPClientSocket.getsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF)
    if(real_buffer_size/2 != recvBufSize):
        logging.warning("set SO_RCVBUF failed! got only: " +str(real_buffer_size/2) + "; trying to continue...")
    BUFF_SIZE = 1024#65536
    serverAddressPort   = (fpga_ip, fpga_port)
    cnt = 0;
    while True:
        logging.debug("Sending bytes: " + str(cnt*BUFF_SIZE) + " : " + str((cnt+1)*BUFF_SIZE-1))
        UDPClientSocket.sendto(bytesToSend[cnt*BUFF_SIZE:(cnt+1)*BUFF_SIZE], serverAddressPort)
        if ((cnt+1)*BUFF_SIZE >= total_size):
            logging.debug("INFO: Reached size to sent")
            break;
        else:
            cnt = cnt + 1
    cnt = 0;
    output_array = np.zeros((total_size,), dtype=input_array.dtype)
    while True:
        logging.debug("Receiving bytes: " + str(cnt*BUFF_SIZE) + " : " + str((cnt+1)*BUFF_SIZE-1))
        msgFromServer = UDPClientSocket.recvfrom(BUFF_SIZE)
        y = np.frombuffer(msgFromServer[0], dtype=input_array.dtype)
        logging.debug(output_array[cnt*BUFF_SIZE:(cnt+1)*BUFF_SIZE-1].size)
        output_array[cnt*BUFF_SIZE:(cnt+1)*BUFF_SIZE] = y
        if ((cnt+1)*BUFF_SIZE >= total_size):
            logging.debug("Reached size to receive")
            break;
        else:
            cnt = cnt + 1
    return output_array

if __name__ == '__main__':
    main(args)
