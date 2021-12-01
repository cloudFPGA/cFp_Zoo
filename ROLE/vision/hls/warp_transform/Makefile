# *****************************************************************************
# *                            cloudFPGA
# *            All rights reserved -- Property of IBM
# *----------------------------------------------------------------------------
# * Created : Apr 2020
# * Authors : Dionysios Diamantopoulos
# *
# * Description : A makefile that synthesizes and packages the current HLS core
# *     as an IP.
# *
# * Synopsis:
# *   make clean   : Cleanup of the current directory.
# *   make         : Runs the default build for this core.
# *
# ******************************************************************************

# Name of the HLS core to build
ipName =warp_transform

DEPS := $(shell find ./src/ -type f)

.PHONY: all csim cosim_view

all: ${ipName}_prj/solution1/impl/ip


# assert ENVIRONMENT
assert_env:
	@echo "This HLS IP Core needs the cFp environment activated....checking..."
	@#We need to match the case with and without PR
	@( [ -f ../../../../cFDK/SRA/LIB/bash/assert_envs.sh ] && ../../../../cFDK/SRA/LIB/bash/assert_envs.sh ) || ../../../cFDK/SRA/LIB/bash/assert_envs.sh

## Either vivado_hls or vitis_hls
hlstool=vivado_hls

HLS_VERSION := $(shell echo `$(hlstool) -version | head -n 1 | awk '{ print $$11}' | tr -d 'v' | tr -d '.'` | cut -c1-5)

${ipName}_prj/solution1/impl/ip: $(DEPS)
	@#rm -rf $@
	rm -rf ${ipName}_prj
	export hlsSim=0; export hlsCoSim=0; export hlsSyn=1; export hlsVersion=$(HLS_VERSION); $(hlstool) -f run_hls.tcl
	@#touch ${ipName}_prj
	@touch $@
	@touch ../../.ip_guard

CHECK_VIVADO := $(shell command -v vivado 2> /dev/null)
VIVADOBINDIR := `dirname ${CHECK_VIVADO}`

CHECK_VITIS := $(shell command -v vitis 2> /dev/null)
VITISBINDIR := `dirname ${CHECK_VITIS}`

# INPUT_IMAGE = ./test/128x128.png
INPUT_IMAGE = ./test/512x512.png
#INPUT_IMAGE ?= ./test/8x8.png

src = ./src/${ipName}.cpp \
			./src/xf_warp_transform_accel.cpp \
			./test/test_${ipName}.cpp

obj = $(src:.cpp=.o)

CXXFLAGS = -g -Wall -fstack-check -Wno-reorder -Wno-unknown-pragmas -Wno-attributes \
					 -D__SDSVHLS__ -std=c++0x -std=c++11 \
					 -DHLS_VERSION=${HLS_VERSION} \
					 `pkg-config --libs opencv` `xml2-config --cflags --libs` \
					 -I$(VIVADOBINDIR)/../include -I$(cFpRootDir)cFDK/SRA/LIB/hls \
					 -I$(cFpRootDir)Vitis_Libraries/vision/L1/include
					 
LDFLAGS = -lpthread

## Runs fast HLS C simulation using g++ toolchain
fcompile: $(obj)
ifdef CHECK_VIVADO
	@echo "Found vivado in $(CHECK_VIVADO)"
	@echo "Found hls version $(HLS_VERSION)"
else
	@echo Vivado was not found in PATH
endif
ifdef CHECK_VITIS
	@echo "Found vitis in $(CHECK_VITIS)"
	@echo "Found hls version $(HLS_VERSION)"
else
	@echo Vitis was not found in PATH
endif
	mkdir -p ./${ipName}_prj
	mkdir -p ./${ipName}_prj/solution1
	mkdir -p ./${ipName}_prj/solution1/fcsim/
	mkdir -p ./${ipName}_prj/solution1/fcsim/build
	$(CXX) -o ./${ipName}_prj/solution1/fcsim/build/fcsim $^ $(CXXFLAGS) $(LDFLAGS)

fcsim: fcompile
	@make logo
	cd ./${ipName}_prj/solution1/fcsim/build && ./$@ ../../../../$(INPUT_IMAGE)
	
memchecksim: fcompile
	cd ./${ipName}_prj/solution1/fcsim/build; valgrind --tool=memcheck --leak-check=full --track-origins=yes ./fcsim ../../../../$(INPUT_IMAGE)

callgraph: fcsim
	cd ./${ipName}_prj/solution1/fcsim/build; valgrind --tool=callgrind ./fcsim ../../../../$(INPUT_IMAGE)

kcachegrind: callgraph
	kcachegrind ./${ipName}_prj/solution1/fcsim/build/callgrind.out.* &

csynth: assert_env
	export hlsSim=0; export hlsCoSim=0; export hlsSyn=1; export hlsVersion=$(HLS_VERSION); $(hlstool) -f run_hls.tcl

csim: assert_env
	export hlsSim=1; export hlsCoSim=0; export hlsSyn=0; export hlsVersion=$(HLS_VERSION); export SimFile=../../../../$(INPUT_IMAGE); $(hlstool) -f run_hls.tcl

cosim: assert_env
	export hlsSim=0; export hlsCoSim=1; export hlsSyn=0; export hlsVersion=$(HLS_VERSION); export SimFile=../../../../$(INPUT_IMAGE); $(hlstool) -f run_hls.tcl
	#@#rm -rf ${ipName}_prj

cosim_view:
	@/bin/echo -e "current_fileset\nopen_wave_database $(ipName).wdb\n" > ./$(ipName)_prj/solution1/sim/verilog/open_wave.tcl
	cd ./$(ipName)_prj/solution1/sim/verilog/; vivado -source open_wave.tcl

clean:
	rm -rf ${ipName}_prj $(obj)
	rm -f vivado*.log vivado_pid*.str hs_err_pid*.log csim.log fcsim.log

	
logo:
	@echo "                                                          ";
	@echo "...build with:                                            ";
	@echo " ██████╗███████╗██████╗   ██╗   ██╗██╗████████╗██╗███████╗";
	@echo "██╔════╝██╔════╝██╔══██╗  ██║   ██║██║╚══██╔══╝██║██╔════╝";
	@echo "██║     █████╗  ██████╔╝  ██║   ██║██║   ██║   ██║███████╗";
	@echo "██║     ██╔══╝  ██╔═══╝   ╚██╗ ██╔╝██║   ██║   ██║╚════██║";
	@echo "╚██████╗██║     ██║███████╗╚████╔╝ ██║   ██║   █║█████║";
	@echo " ╚═════╝╚═╝     ╚═╝╚══════╝ ╚═══╝  ╚═╝   ╚═╝   ╚═╝╚══════╝";
	@echo "                                               v1.0 --did ";
	@echo "                                                          ";



