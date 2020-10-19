
import pathlib
import os
import shutil
from re import search



def replace_markdown_links(full_md_file, new_kernel):
    f1 = open(full_md_file, 'r')
    f2 = open(full_md_file+"_new", 'w')

    replaced = 0
    for s in f1:
        new_s = s2 = s
        if search("vhdl", full_md_file):
            search_str = "  component "
            if search(search_str, s):
                s2 = "  component "+new_kernel+"Application is\n"
                replaced = replaced + 1

            search_str = "  end component "
            if search(search_str, s):
                s2 = "  end component "+new_kernel+"Application;\n"
                replaced = replaced + 1

            search_str = "  UAF: "
            if search(search_str, s):
                s2 = "  UAF: "+new_kernel+"Application\n"
                replaced = replaced + 1          

            search_str = "  TAF: "
            if search(search_str, s):
                s2 = "  TAF: "+new_kernel+"Application\n"
                replaced = replaced + 1 
                
            new_s = s.replace(str(s), str(s2))
            f2.write(new_s)                     
  
        if search("tcl", full_md_file):
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
  
        if search("Makefile", full_md_file):
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
                
    print("INFO: Replaced links of " + full_md_file + " : "+ str(replaced))
    print("#################")
    f1.close()
    f2.close()
    if (replaced != 0):
        shutil.copyfile(full_md_file, full_md_file+"_backup")
        shutil.move(full_md_file+"_new", full_md_file)
                

kernels = ["Harris", "MCEuropeanEngine"]
print("Available kernels:")
print('\n'.join(kernels)) 

kernel_id_str = input("Select a kernel using index 0-"+str(len(kernels)-1)+":")   # Python 3
kernel_id = int(kernel_id_str)
if ((kernel_id < 0 ) or kernel_id >= len(kernels)):
    print("ERROR: Invalid kernel id. Aborting...")
    exit(-1)
    
new_kernel = kernels[kernel_id]

md_file = "ROLE/quantitative_finance/hdl/Role.vhdl"
full_md_file = str(pathlib.Path().absolute()) + '/' + str(md_file)
print("#################\n"+full_md_file+"\n----------------")
replace_markdown_links(full_md_file, new_kernel)

md_file = "ROLE/quantitative_finance/tcl/create_ip_cores.tcl"
full_md_file = str(pathlib.Path().absolute()) + '/' + str(md_file)
print("#################\n"+full_md_file+"\n----------------")
replace_markdown_links(full_md_file, new_kernel)

md_file = "ROLE/quantitative_finance/hls/Makefile"
full_md_file = str(pathlib.Path().absolute()) + '/' + str(md_file)
print("#################\n"+full_md_file+"\n----------------")
replace_markdown_links(full_md_file, new_kernel)