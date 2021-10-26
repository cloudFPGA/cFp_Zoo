# @brief  A script for editing the files needed to select the Vitis kernel to be implemented
# @author DID
# @date Nov. 2020

import pathlib
import os
import shutil
from re import search
import sys



def edit_file(full_file, new_kernel, udp, tcp, mtu, port, ddr):
    f1 = open(full_file, 'r')
    f2 = open(full_file+"_new", 'w')

    replaced = 0
    uaf_start_detected = taf_start_detected = 0
    ddr_component_start_detected = ddr_component_end_detected = 0
    ddr_uaf_start_detected = ddr_uaf_end_detected = 0
    ddr_taf_start_detected = ddr_taf_end_detected = 0
    ddr_Mp2_dummy_start_detected = ddr_Mp2_dummy_end_detected = 0
    
    for s in f1:
        new_s = s2 = s
        if search("vhdl", full_file):
            search_str = "  component "
            if search(search_str, s):
                s2 = "  component "+new_kernel+"Application is\n"
                replaced = replaced + 1

            search_str = "  end component "
            if search(search_str, s):
                s2 = "  end component "+new_kernel+"Application;\n"
                replaced = replaced + 1
 
            #######################################################################################
            # Replacing role interface for DDR Mp0-1 in Role.vdl
            
            # Check component start
            search_str = "SHELL / Mem / Mp0 Interface / Start Component"
            if search(search_str, s):
                s2 = s
                ddr_component_start_detected = 1
            
            # Check component stop
            search_str = "SHELL / Mem / Mp1 Interface / End Component"
            if search(search_str, s):
                s2 = s
                ddr_component_end_detected = 1
                
            # Replace ddr pattern in component declaration
            if (ddr_component_start_detected):
              excluding_pattern = "-- auto excluding component Mp0-Mp1           "
              if (search(excluding_pattern, s2)):
                if (ddr):
                  s2 = s2.replace(str(excluding_pattern), str(''))
                  replaced = replaced + 1 
              else:
                if (not ddr):
                  s2 = excluding_pattern + s2
                  replaced = replaced + 1 
              if (ddr_component_end_detected):
                ddr_component_start_detected = ddr_component_end_detected = 0;
                
            
            # Check DDR in UAF start
            search_str = "SHELL / Mem / Mp0 Interface / Start in UAF"
            if search(search_str, s):
                s2 = s
                ddr_uaf_start_detected = 1
            
            # Check DDR in UAF stop
            search_str = "SHELL / Mem / Mp1 Interface / End in UAF"
            if search(search_str, s):
                s2 = s
                ddr_uaf_end_detected = 1
                
            # Replace ddr pattern in component declaration
            if (ddr_uaf_start_detected):
              excluding_pattern = "-- auto excluding Mp0-Mp1 in UAF           "
              if (search(excluding_pattern, s2)):
                if (ddr):
                  s2 = s2.replace(str(excluding_pattern), str(''))
                  replaced = replaced + 1 
              else:
                if (not ddr):
                  s2 = excluding_pattern + s2
                  replaced = replaced + 1 
              if (ddr_uaf_end_detected):
                ddr_uaf_start_detected = ddr_uaf_end_detected = 0;                
                
             
            # Check DDR in TAF start
            search_str = "SHELL / Mem / Mp0 Interface / Start in TAF"
            if search(search_str, s):
                s2 = s
                ddr_taf_start_detected = 1
            
            # Check DDR in TAF stop
            search_str = "SHELL / Mem / Mp1 Interface / End in TAF"
            if search(search_str, s):
                s2 = s
                ddr_taf_end_detected = 1
                
            # Replace ddr pattern in component declaration
            if (ddr_taf_start_detected):
              excluding_pattern = "-- auto excluding Mp0-Mp1 in TAF           "
              if (search(excluding_pattern, s2)):
                if (ddr):
                  s2 = s2.replace(str(excluding_pattern), str(''))
                  replaced = replaced + 1 
              else:
                if (not ddr):
                  s2 = excluding_pattern + s2
                  replaced = replaced + 1 
              if (ddr_taf_end_detected):
                ddr_taf_start_detected = ddr_taf_end_detected = 0;                


            # Check DDR in dummy start
            search_str = "2nd Memory Port dummy connections Start"
            if search(search_str, s):
                s2 = s
                ddr_Mp2_dummy_start_detected = 1
            
            # Check DDR in dummy stop
            search_str = "2nd Memory Port dummy connections End"
            if search(search_str, s):
                s2 = s
                ddr_Mp2_dummy_end_detected = 1
                
            # Replace ddr pattern in component declaration
            if (ddr_Mp2_dummy_start_detected):
              excluding_pattern = "-- auto excluding Mp2 open connections           "
              if (search(excluding_pattern, s2)):
                if (not ddr):
                  s2 = s2.replace(str(excluding_pattern), str(''))
                  replaced = replaced + 1 
              else:
                if (ddr):
                  s2 = excluding_pattern + s2
                  replaced = replaced + 1 
              if (ddr_Mp2_dummy_end_detected):
                ddr_Mp2_dummy_start_detected = ddr_Mp2_dummy_end_detected = 0;        
            #######################################################################################

            search_str = "  UAF: "
            if search(search_str, s):
                s2 = "  UAF: "+new_kernel+"Application\n"
                uaf_start_detected = 1
                replaced = replaced + 1          

            search_str = "  TAF: "
            if search(search_str, s):
                s2 = "  TAF: "+new_kernel+"Application\n"
                taf_start_detected = 1
                replaced = replaced + 1 
                
            # Checking if we have to comment out UAF
            if (uaf_start_detected):
              excluding_pattern = "-- auto excluding UAF           "
              if (search(excluding_pattern, s2)):
                if (udp):
                  s2 = s2.replace(str(excluding_pattern), str(''))
                  replaced = replaced + 1 
              else:
                if (not udp):
                  s2 = excluding_pattern + s2
                  replaced = replaced + 1 
              if (search(" \);", s)):
                uaf_start_detected = 0;
 
            # Checking if we have to comment out TAF
            if (taf_start_detected):
              excluding_pattern = "-- auto excluding TAF           "
              if (search(excluding_pattern, s2)):
                if (tcp):
                  s2 = s2.replace(str(excluding_pattern), str(''))
                  replaced = replaced + 1 
              else:
                if (not tcp):
                  s2 = excluding_pattern + s2
                  replaced = replaced + 1 
              if (search(" \);", s)):
                taf_start_detected = 0;
                
            new_s = s.replace(str(s), str(s2))
            f2.write(new_s)                     
  
        if search("tcl", full_file):
            search_str = "# IBM-HSL-IP : "
            if search(search_str, s):
                s2 = "# IBM-HSL-IP : "+new_kernel+" Application Flash\n"
                replaced = replaced + 1 

            search_str = "set ipModName"
            if search(search_str, s):
                s2 = "set ipModName \""+new_kernel+"Application\"\n"
                replaced = replaced + 1   
  
            search_str = "set ipName"
            if search(search_str, s):
                s2 = "set ipName \""+new_kernel.lower()+"\"\n"
                replaced = replaced + 1   
  
            new_s = s.replace(str(s), str(s2))
            f2.write(new_s)                     
  
        if search("Makefile", full_file):
            search_str = ".PHONY: all clean mem_test_flash "
            if search(search_str, s):
                s2 = ".PHONY: all clean mem_test_flash "+new_kernel.lower()+"\n"
                replaced = replaced + 1 

            search_str = "all: mem_test_flash "
            if search(search_str, s):
                s2 = "all: mem_test_flash "+new_kernel.lower()+"\n"
                replaced = replaced + 1 
                
            new_s = s.replace(str(s), str(s2))
            f2.write(new_s)    
                
        if search("config.h", full_file):
            search_str1 = "#define NET_TYPE "
            search_str2 = "#define PACK_SIZE "
            if search(search_str1, s):
                if (udp and tcp):
                    s2 = search_str1 + "udp //tcp\n"
                elif (udp):
                    s2 = search_str1 + "udp\n"    
                elif (tcp):
                    s2 = search_str1 + "tcp\n"    
                replaced = replaced + 1 
            elif search(search_str2, s):
                s2 = search_str2 + mtu + "\n"
                replaced = replaced + 1 
            new_s = s.replace(str(s), str(s2))
            f2.write(new_s) 


        if search(".hpp", full_file):
            search_str1 = "#define DEFAULT_TX_PORT "
            search_str2 = "#define DEFAULT_RX_PORT "
            if search(search_str1, s):
                s2 = search_str1 + port + "\n"    
                replaced = replaced + 1 
            elif search(search_str2, s):
                s2 = search_str2 + port + "\n"
                replaced = replaced + 1 
            new_s = s.replace(str(s), str(s2))
            
            search_str3 = "#define ENABLE_DDR"
            if search(search_str3, s):
                if (ddr):
                    s2 = search_str3 + "\n"
                else:
                    s2 = "// " + search_str3 + "\n"
                replaced = replaced + 1 
            new_s = new_s.replace(str(s), str(s2))
            f2.write(new_s)


    print("INFO: Edits of file " + full_file + " : "+ str(replaced))
    print("#################")
    f1.close()
    f2.close()
    if (replaced != 0):
        shutil.copyfile(full_file, full_file+"_backup")
        shutil.move(full_file+"_new", full_file)
                

###################################################################################################
### Main script
###################################################################################################


kernels = ["Harris", "Median_Blur", "MCEuropeanEngine", "Uppercase"]
#print("Available kernels:")
#print('\n'.join(kernels)) 

# Count the arguments
arguments = len(sys.argv) - 1

if arguments != 6:
  print("ERROR: Invalid number of arguments. Expected 6 but provided " + str(arguments) + ". Aborting...")
  exit(-1)

kernel_id = -1
for i in range(len(kernels)):
  if (kernels[i] == sys.argv[3]):
    kernel_id = i
    break

if (kernel_id == -1):
   print("ERROR: Kernel " + sys.argv[3] + " not found. Aborting...")
   exit(-1)
else:
   print("INFO: Kernel " + sys.argv[3] + " found at index " + str(kernel_id) + ". Continuing...")

# select tcp/udp role
udp = tcp = 0
if search('udp', sys.argv[1]):
  udp = 1
if  search('tcp', sys.argv[1]):
  tcp = 1

mtu=sys.argv[4]
port=sys.argv[5]

# select ddr in role
if search('ddr_enabled', sys.argv[6]):
  ddr = 1
elif  search('ddr_disabled', sys.argv[6]):
  ddr = 0
else:
  print("ERROR: Invalid DDR option. Aborting...")
  exit(-1)

if ((kernel_id < 0 ) or kernel_id >= len(kernels)):
    print("ERROR: Invalid kernel id. Aborting...")
    exit(-1)
    
new_kernel = kernels[kernel_id]

role = os.getenv('roleName1')

file = "ROLE/"+role+"/hdl/Role.vhdl"
full_file = str(pathlib.Path().absolute()) + '/' + str(file)
print("#################\n"+full_file+"\n----------------")
edit_file(full_file, new_kernel, udp, tcp, mtu, port, ddr)

file = "ROLE/"+role+"/tcl/create_ip_cores.tcl"
full_file = str(pathlib.Path().absolute()) + '/' + str(file)
print("#################\n"+full_file+"\n----------------")
edit_file(full_file, new_kernel, udp, tcp, mtu, port, ddr)

file = "ROLE/"+role+"/hls/Makefile"
full_file = str(pathlib.Path().absolute()) + '/' + str(file)
print("#################\n"+full_file+"\n----------------")
edit_file(full_file, new_kernel, udp, tcp, mtu, port, ddr)

file = "HOST/"+role+"/"+new_kernel.lower()+"/languages/cplusplus/include/config.h"
full_file = str(pathlib.Path().absolute()) + '/' + str(file)
print("#################\n"+full_file+"\n----------------")
edit_file(full_file, new_kernel, udp, tcp, mtu, port, ddr)

file = "ROLE/"+role+"/hls/"+new_kernel.lower()+"/include/"+new_kernel.lower()+".hpp"
full_file = str(pathlib.Path().absolute()) + '/' + str(file)
print("#################\n"+full_file+"\n----------------")
edit_file(full_file, new_kernel, udp, tcp, mtu, port, ddr)


