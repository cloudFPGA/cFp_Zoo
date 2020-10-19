# *****************************************************************************
# *                            cloudFPGA
# *            All rights reserved -- Property of IBM
# *----------------------------------------------------------------------------
# * Created : October 2020
# * Authors : FAB, WEI, NGL, DID
# *
# * Description : A makefile that synthesizes and packages the current HLS core
# *               as an IP.
# *
# * Synopsis:
# *   make clean   : Cleanup of the current directory.
# *   make         : Runs the default build for this core.
# *
# ******************************************************************************

# Name of the HLS core to build
ipName =mceuropeanengine

DEPS := $(shell find ./src/ -type f)

.PHONY: all csim cosim_view

include ./Makefile.rules

all: ${ipName}_prj/solution1/impl/ip


# assert ENVIRONMENT
assert_env:
	@echo "This HLS IP Core needs the cFp environment activated....checking..."
	@#We need to match the case with and without PR
	@( [ -f ../../../../cFDK/SRA/LIB/bash/assert_envs.sh ] && ../../../../cFDK/SRA/LIB/bash/assert_envs.sh ) || ../../../cFDK/SRA/LIB/bash/assert_envs.sh

${ipName}_prj/solution1/impl/ip: $(DEPS)
	@#rm -rf $@
	rm -rf ${ipName}_prj
	export hlsSim=0; export hlsCoSim=0; export hlsSyn=1; vivado_hls -f run_hls.tcl
	@#touch ${ipName}_prj
	@touch $@
	@touch ../../.ip_guard

CHECK_VIVADO := $(shell command -v vivado 2> /dev/null)
VIVADOBINDIR := `dirname ${CHECK_VIVADO}`

CHECK_VITIS := $(shell command -v vitis 2> /dev/null)
VITISBINDIR := `dirname ${CHECK_VITIS}`

INPUT_FILE ?= "./etc/mce.conf"

src = ./src/${ipName}.cpp \
			./src/kernel_mc.cpp \
			./test/test_${ipName}.cpp

obj = $(src:.cpp=.o)
CXXFLAGS = -g -Wall -fstack-check -Wno-reorder -Wno-unknown-pragmas -Wno-attributes \
					 -D__SDSVHLS__ -std=c++0x -std=c++11 \
					 `xml2-config --cflags --libs` \
					 -I$(VIVADOBINDIR)/../include \
					 -I$(VIVADOBINDIR)/../include/hls/linear_algebra/utils \
					 -I$(VITISBINDIR)/../include -I$(cFpRootDir)cFDK/SRA/LIB/hls \
					 -I$(cFpRootDir)Vitis_Libraries/quantitative_finance/L1/include \
					 -I$(cFpRootDir)Vitis_Libraries/quantitative_finance/L2/include

LDFLAGS = -lpthread -lm $(LFLAG) 




## Runs fast HLS C simulation using g++ toolchain
fcompile: $(obj)
ifdef CHECK_VIVADO
	@echo "Found version $(CHECK_VIVADO)"
else
	@echo Vivado was not found in PATH
endif
ifdef CHECK_VITIS
	@echo "Found version $(CHECK_VITIS)"
else
	@echo Vitis was not found in PATH
endif
	$(Verb)mkdir -p ./${ipName}_prj
	$(Verb)mkdir -p ./${ipName}_prj/solution1
	$(Verb)mkdir -p ./${ipName}_prj/solution1/fcsim/
	$(Verb)mkdir -p ./${ipName}_prj/solution1/fcsim/build
	$(Verb)$(CXX) -o ./${ipName}_prj/solution1/fcsim/build/fcsim $^ $(CXXFLAGS) $(LDFLAGS) 

fcsim: fcompile
	@make logo
	cd ./${ipName}_prj/solution1/fcsim/build && ./$@ ../../../../${INPUT_FILE}
	
memchecksim: fcompile
	cd ./${ipName}_prj/solution1/fcsim/build; valgrind --tool=memcheck --leak-check=full --track-origins=yes ./fcsim ../../../../${INPUT_FILE}

callgraph: fcsim
	cd ./${ipName}_prj/solution1/fcsim/build; valgrind --tool=callgrind ./fcsim ../../../../${INPUT_FILE}

kcachegrind: callgraph
	kcachegrind ./${ipName}_prj/solution1/fcsim/build/callgrind.out.* &

csynth: assert_env
	export hlsSim=0; export hlsCoSim=0; export hlsSyn=1; vivado_hls -f run_hls.tcl

csim: assert_env
	export hlsSim=1; export hlsCoSim=0; export hlsSyn=0; export SimFile=../../../../$(INPUT_FILE); vivado_hls -f run_hls.tcl

cosim: assert_env
	export hlsSim=0; export hlsCoSim=1; export hlsSyn=0; export SimFile=../../../../$(INPUT_FILE); vivado_hls -f run_hls.tcl
	#@#rm -rf ${ipName}_prj

cosim_view:
	@/bin/echo -e "current_fileset\nopen_wave_database $(ipName).wdb\n" > ./$(ipName)_prj/solution1/sim/verilog/open_wave.tcl
	cd ./$(ipName)_prj/solution1/sim/verilog/; vivado -source open_wave.tcl

clean:
	$(Verb)rm -rf ${ipName}_prj $(obj)
	$(Verb)rm -f vivado*.log vivado_pid*.str hs_err_pid*.log csim.log fcsim.log

	
logo:
	@echo "                                                          ";
	@echo "...build with:                                            ";
	@echo " ██████╗███████╗██████╗   ██╗   ██╗██╗████████╗██╗███████╗";
	@echo "██╔════╝██╔════╝██╔══██╗  ██║   ██║██║╚══██╔══╝██║██╔════╝";
	@echo "██║     █████╗  ██████╔╝  ██║   ██║██║   ██║   ██║███████╗";
	@echo "██║     ██╔══╝  ██╔═══╝   ╚██╗ ██╔╝██║   ██║   ██║╚════██║";
	@echo "╚██████╗██║     ██║███████╗╚████╔╝ ██║   ██║   ██║███████║";
	@echo " ╚═════╝╚═╝     ╚═╝╚══════╝ ╚═══╝  ╚═╝   ╚═╝   ╚═╝╚══════╝";
	@echo "                                               v1.0 --did ";
	@echo "                                                          ";


