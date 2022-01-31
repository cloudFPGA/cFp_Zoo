#!/bin/bash

warm_up_fpga () {
	ip_list=$1
	port_list=$2
	IFS=':' read -ra FPGAS <<< "$ip_list"
	IFS=':' read -ra PORTS <<< "$port_list"
	for i in ${FPGAS[@]}
	do
		echo $i
	done
	echo ""
	len=${#FPGAS[@]}
	updated_len=$((len-1))
	for i in $(seq 0 $updated_len )
	do
		echo "Warming up $i thread"
		echo ${FPGAS[$i]}
		echo ${PORTS[$i]}
		./warp_transform_host_lightweight ${FPGAS[$i]} ${PORTS[$i]} ./128x128.png ./ 2
		sleep 5
	done
	echo ""
	echo "Done warm up"
}

echo "Usage <# of threads> <exe mode 0|2 cpu|cf> <tx applied 0 to 8> <ip list ip0:ip1:ip2> <port list p0:p1:p2>"
max_power_threads=${1:-2}
thr_list=()
EXE_MODE=${2:-0}
WAX_MODE=${3:-2}
IPs=${4:-}
PORTs=${5:-}


if [[ EXE_MODE -eq 0 ]]
then
	logfile=cpu_logger.csv
	THR=./out_thread_cpu
	OMP=./out_openmp_cpu
	for i in $(seq 0 $max_power_threads)
	do
		echo $((2**$i)) 
		thr_list+=($((2**$i)))
	done
else
	logfile=cf_logger.csv
	THR=./out_thread_cf
	OMP=./out_openmp_cf
	thr_list=$(seq 1 $max_power_threads)
	warm_up_fpga $IPs $PORTs
fi

echo $logfile
echo ${thr_list[@]}
echo "$IPs $PORTs"



echo "thr[#],thread[ms],openmp[ms]" > $logfile
echo "Logging everythong"
for i in ${thr_list[@]}
do
	echo "Clean the out folders of $i"
	mkdir -p $THR-$i/ 
	mkdir -p $OMP-$i/
	rm -rf ${THR}-$i/* ${OMP}-$i/*
	echo "Executing OpenMP with $i threads"
	ompres=$(./warp_transform_host_parallel_openmp ../dataset/ $OMP-$i/ $EXE_MODE $i $WAX_MODE $PORTs $IPs | grep chrono | sed 's/.*=//')
	sleep 5 
	echo "Executing std::thread with $i threads"
	thrres=$(./warp_transform_host_parallel_thread ../dataset/ $THR-$i/ $EXE_MODE $i $WAX_MODE $PORTs $IPs | grep chrono | sed 's/.*=//')
	echo "$i,$thrres,$ompres" >> $logfile
	sleep 5 
	warm_up_fpga $IPs $PORTs
done
