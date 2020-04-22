# *****************************************************************************
# *                            cloudFPGA
# *            All rights reserved -- Property of IBM
# *----------------------------------------------------------------------------
# * Created : Sep 2018
# * Authors : Francois Abel, Burkhard Ringlein
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
ipName =triangle_app

DEPS := $(shell find ./src/ -type f)

.PHONY: all sim cosim cosim_view
all: ${ipName}_prj/solution1/impl/ip


# assert ENVIRONMENT
assert_env:
	@echo "This HLS IP Core needs the cFp environment activated....checking..."
	@#We need to match the case with and without PR
	@( [ -f ../../../../cFDK/SRA/LIB/bash/assert_envs.sh ] && ../../../../cFDK/SRA/LIB/bash/assert_envs.sh ) || ../../../cFDK/SRA/LIB/bash/assert_envs.sh

${ipName}_prj/solution1/impl/ip: $(DEPS)
	@#rm -rf $@
	rm -rf ${ipName}_prj
	export hlsSim=0; export hlsCoSim=0; vivado_hls -f run_hls.tcl
	@#touch ${ipName}_prj
	@touch $@
	@touch ../../.ip_guard

CHECK_VIVADO := $(shell command -v vivado 2> /dev/null)
VIVADOBINDIR := `dirname ${CHECK_VIVADO}`


src = ./src/${ipName}.cpp \
			./test/test_${ipName}.cpp

obj = $(src:.cpp=.o)
CXXFLAGS = -g -Wall -fstack-check -Wno-reorder -Wno-unknown-pragmas -I$(VIVADOBINDIR)/../include -I$(cFpRootDir)/cFDK/SRA/LIB/hls -std=c++11

csim: assert_env
	export hlsSim=1; export hlsCoSim=0; vivado_hls -f run_hls.tcl

## Runs fast HLS C simulation using g++ toolchain
fcsim: $(obj)
ifdef CHECK_VIVADO
	@echo "Found version $(CHECK_VIVADO)"
else
	@echo Vivado was not found in PATH
endif
	mkdir -p ./${ipName}_prj
	mkdir -p ./${ipName}_prj/solution1
	mkdir -p ./${ipName}_prj/solution1/fcsim/
	mkdir -p ./${ipName}_prj/solution1/fcsim/build
	$(CXX) -o ./${ipName}_prj/solution1/fcsim/build/$@ $^ $(CXXFLAGS)
	cd ./${ipName}_prj/solution1/fcsim/build && ./$@

memchecksim: fcsim
	cd ./${ipName}_prj/solution1/fcsim/build; valgrind --tool=memcheck --leak-check=full --track-origins=yes ./fcsim

callgraph: fcsim
	cd ./${ipName}_prj/solution1/fcsim/build; valgrind --tool=callgrind ./fcsim 

kcachegrind: callgraph
	kcachegrind ./${ipName}_prj/solution1/fcsim/build/callgrind.out.* &

cosim: assert_env
	@#rm -rf ${ipName}_prj
	export hlsSim=0; export hlsCoSim=1; vivado_hls -f run_hls.tcl

cosim_view:
	@/bin/echo -e "current_fileset\nopen_wave_database $(ipName).wdb\n" > ./$(ipName)_prj/solution1/sim/verilog/open_wave.tcl
	cd ./$(ipName)_prj/solution1/sim/verilog/; vivado -source open_wave.tcl

clean:
	rm -rf ${ipName}_prj $(obj)
	rm -f vivado*.log
