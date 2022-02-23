HOST_SRC_DIR=../src
EXEC_DIR=../build

HOST_SRC=$(HOST_SRC_DIR)/warp_transform_host_parallel.cpp $(HOST_SRC_DIR)/warp_transform_host_lightweight.cpp
HOST_MODE=1
SRVR_MODE=0
#define this variable to use the CF IPS
USE_CF=

CLSTR_SIZE?=1
THREADS?=1
PORTS="2718:2719:2720:2721:2722:2723:2724:2725:2726:2727:2728:2729:2730:2731:2732:2733:2734:2735:2736:2737:2738:2739:2740:2741:2742:2743:2744:2745:2746:2747:2748:2749"
CPU_IPS="10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9:10.12.0.9"
CF_IPS="10.12.200.128:10.12.200.45:10.12.200.30:10.12.200.189:10.12.200.47:10.12.200.133:10.12.200.167:10.12.200.105:10.12.200.233:10.12.200.148:10.12.200.145:10.12.200.56:10.12.200.89:10.12.200.28"
CF_PORTS="2718:2719:2720:2721:2722:2723:2724:2725:2726:2727:2728:2729:2730:2731"

.PHONY: docker_run all run

all: docker_run
run: docker_run

build_clean_sw: $(HOST_SRC) $(HOST_SRC_DIR)/warp_transform_srvr_cpu.cpp
	cd $(EXEC_DIR); make clean; make; cd -

build_sw: $(HOST_SRC) $(HOST_SRC_DIR)/warp_transform_srvr_cpu.cpp
	cd $(EXEC_DIR); make; cd -

setup_host: build_clean_sw ../benchmark_threads.sh
	mkdir -p ./host_dir
	cp $(EXEC_DIR)/warp_transform_host_parallel* ./host_dir
	cp $(EXEC_DIR)/warp_transform_host_lightweight ./host_dir
	cp $(EXEC_DIR)/128x128.png ./host_dir
	cp -R ../dataset ./host_dir
	cp ../benchmark_threads.sh ./host_dir
	cp container_start_script.sh ./host_dir


setup_srvr: build_clean_sw ../setup_multisrvr.sh
	mkdir -p ./srvr_dir
	cp $(EXEC_DIR)/warp_transform_srvr_cpu ./srvr_dir
	cp ../setup_multisrvr.sh ./srvr_dir
	cp container_start_srvr.sh ./srvr_dir


docker.srvr_stamp: Dockerfile_srvr setup_srvr  container_start_script.sh
	docker build -f Dockerfile_srvr -t dco_runner_srvr:latest .
	touch $@

docker.host_stamp: Dockerfile setup_host container_start_script.sh
	docker build -f Dockerfile -t dco_runner:latest .
	touch $@

docker_run: 
	@echo ${CF_IPS}
	@echo ${CPU_IPS}
	@echo ${PORTS}
	

docker_run_host: docker_create_script.sh docker.host_stamp
	./docker_create_script.sh ${HOST_MODE} ${CLSTR_SIZE} ${THREADS} ${PORTS} ${CPU_IPS}

docker_run_host_cf: docker_create_script.sh docker.host_stamp
	./docker_create_script.sh ${HOST_MODE} ${CLSTR_SIZE} ${THREADS} ${CF_PORTS} ${CF_IPS}

docker_run_srvr: docker_create_script.sh docker.srvr_stamp
	./docker_create_script.sh  ${SRVR_MODE} ${CLSTR_SIZE} ${THREADS} ${PORTS} ${CPU_IPS}
