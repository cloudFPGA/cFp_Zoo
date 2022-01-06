# *****************************************************************************
# *                            cloudFPGA
# *            All rights reserved -- Property of IBM
# *----------------------------------------------------------------------------
# * Created : Sep 2020
# * Authors : Dionysios Diamantopoulos, Davide Conficconi
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
ipName =memtest
SHELL:=/bin/bash
DEPS := $(shell find ./src/ -type f)

.PHONY: all csim cosim_view

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

START_COMMAND?=0100000000000000
STOP_COMMAND?=0200000000000000
fConvert_hex_to_ascii= $(shell echo ${1} | xxd -r -p)
fConvert_ascii_to_hex= $(shell xxd -pu <<< "${1}")

MY_STRING ?= "Hello_World!_It_is_extremely_fun_to_dive_into_cF_world_with_this_memtest_example."
MY_STRING_WITH_SEPARATOR=$(call fConvert_ascii_to_hex,${MY_STRING})
MY_STRING_CLEANED=$(shell sed -e 's/ //g' <<< "${MY_STRING_WITH_SEPARATOR}")
#INPUT_STRING="$(call fConvert_hex_to_ascii,${START_COMMAND})${MY_STRING}$(call fConvert_hex_to_ascii,${STOP_COMMAND} )"
#INPUT_STRING ?="${START_COMMAND}$(MY_STRING_CLEANED)${STOP_COMMAND}"
#INPUT_STRING ?="${START_COMMAND}$(MY_STRING_WITH_SEPARATOR)${STOP_COMMAND}"
INPUT_STRING ?= 5
TEST_NUMBER ?= 1
BURST_SIZE ?= 1
COMMAND_STRING ?=""

src = ./src/${ipName}.cpp \
			./test/test_${ipName}.cpp

obj = $(src:.cpp=.o)
CXXFLAGS = -g -Wall -fstack-check -Wno-reorder -Wno-unknown-pragmas -Wno-attributes \
					 -D__SDSVHLS__ -std=c++0x \
					 `xml2-config --cflags --libs` \
					 -I$(VIVADOBINDIR)/../include -I$(cFpRootDir)cFDK/SRA/LIB/hls 
					 
LDFLAGS = -lpthread					 

TEST_NUMBER_SEQ := $(shell seq 1 2)
ADDRESS_TO_TEST_SEQ := $(shell seq 2 3)
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
	mkdir -p ./${ipName}_prj
	mkdir -p ./${ipName}_prj/solution1
	mkdir -p ./${ipName}_prj/solution1/fcsim/
	mkdir -p ./${ipName}_prj/solution1/fcsim/build
	$(CXX) -o ./${ipName}_prj/solution1/fcsim/build/fcsim $^ $(CXXFLAGS) $(LDFLAGS)

# trying for an automation testing but not workings
#multicsim:
#	for i in ${TEST_NUMBER_SEQ}; do for j in ${ADDRESS_TO_TEST_SEQ}; do make clean csim TEST_NUMBER=${i} INPUT_STRING=${j}; done; done

fcsim: fcompile
	@make logo
	cd ./${ipName}_prj/solution1/fcsim/build && ./$@ $(INPUT_STRING) $(TEST_NUMBER) $(BURST_SIZE) $(COMMAND_STRING)
	
memchecksim: fcompile
	cd ./${ipName}_prj/solution1/fcsim/build; valgrind --tool=memcheck --leak-check=full --track-origins=yes ./fcsim $(INPUT_STRING) $(TEST_NUMBER) $(BURST_SIZE) $(COMMAND_STRING)

callgraph: fcsim
	cd ./${ipName}_prj/solution1/fcsim/build; valgrind --tool=callgrind ./fcsim $(INPUT_STRING) $(TEST_NUMBER) $(BURST_SIZE) $(COMMAND_STRING)

kcachegrind: callgraph
	kcachegrind ./${ipName}_prj/solution1/fcsim/build/callgrind.out.* &

csynth: assert_env
	export hlsSim=0; export hlsCoSim=0; export hlsSyn=1; vivado_hls -f run_hls.tcl

csim: assert_env
	echo ${INPUT_STRING}
	export hlsSim=1; export hlsCoSim=0; export hlsSyn=0; export SimString=${INPUT_STRING}; export SimNumberTests=$(TEST_NUMBER); export SimBurstSize=$(BURST_SIZE); export CommandString=$(COMMAND_STRING); vivado_hls -f run_hls.tcl

cosim: assert_env
		export hlsSim=0; export hlsCoSim=1; export hlsSyn=0; export SimString=$(INPUT_STRING); export SimNumberTests=$(TEST_NUMBER); export SimBurstSize=$(BURST_SIZE); export CommandString=$(COMMAND_STRING); vivado_hls -f run_hls.tcl
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
	@echo " ██████╗███████╗██████╗    ███████╗ ██████╗  ██████╗      ";
	@echo "██╔════╝██╔════╝██╔══██╗   ╚══███╔╝██╔═══██╗██╔═══██╗     ";
	@echo "██║     █████╗  ██████╔╝     ███╔╝ ██║   ██║██║   ██║     ";
	@echo "██║     ██╔══╝  ██╔═══╝     ███╔╝  ██║   ██║██║   ██║     ";
	@echo "╚██████╗██║     ██║███████╗███████╗╚██████╔╝╚██████╔╝     ";
	@echo " ╚═════╝╚═╝     ╚═╝╚══════╝╚══════╝ ╚═════╝  ╚═════╝      ";
	@echo "                                                     v1.0 ";
	@echo "                                                          ";



