#!/bin/bash
ping_fpga(){
	ping -c 16 $1
}

warm_up_fpga () {
	ip_list=$1
	port_list=$2
	threads=$3
	IFS=':' read -ra FPGAS <<< "$ip_list"
	IFS=':' read -ra PORTS <<< "$port_list"
	for i in ${FPGAS[@]}
	do
		echo $i
	done
	echo ""
	len=${#FPGAS[@]}
	lenport=${#PORTS[@]}
	#check if 
	if [[ threads -gt len ]]
	then
		echo "ERROR more threads than availble platforms"
		exit 1
	fi
	if [[ lenport -ne len ]]
	then
		echo "ERROR not same ports and ip nodes"
		exit 1
	fi

	if [[ threads -lt len ]]
	then
		len=$threads
	fi

	updated_len=$((len-1))
	for i in $(seq 0 $updated_len )
	do
		echo ${FPGAS[$i]}
		ping_fpga ${FPGAS[$i]}
	done
	for i in $(seq 0 $updated_len )
	do
		echo "Warming up $i node"
		#ping_fpga ${FPGAS[$i]}
		echo ${FPGAS[$i]}
		echo ${PORTS[$i]}
		./warp_transform_host_lightweight ${FPGAS[$i]} ${PORTS[$i]} ./128x128.png ./ 2
		echo "Warm up $i completed"
		sleep 2 
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
threads=1

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
	threads=$((2**$max_power_threads))
else
	logfile=cf_logger.csv
	THR=./out_thread_cf
	OMP=./out_openmp_cf
	thr_list=$(seq 1 $max_power_threads)
	threads=$max_power_threads
	warm_up_fpga $IPs $PORTs $threads

fi
#exit
echo $logfile
echo ${thr_list[@]}
echo "$IPs $PORTs"



echo -e "\n************ Benchmark Begins :) *******************\n"
echo "thr[#],thread[ms],openmp[ms]" > $logfile
echo "Logging everything"
for i in ${thr_list[@]}
do
	echo "Clean the out folders of $i"
	mkdir -p $THR-$i/ 
	mkdir -p $OMP-$i/
	rm -rf ${THR}-$i/* ${OMP}-$i/*
	echo "Executing OpenMP with $i nodes"
	ompres=$(./warp_transform_host_parallel_openmp ../dataset/ $OMP-$i/ $EXE_MODE $i $WAX_MODE $PORTs $IPs | grep chrono | sed 's/.*=//')
	sleep 5 
	echo "Executing std::thread with $i nodes"
	thrres=$(./warp_transform_host_parallel_thread ../dataset/ $THR-$i/ $EXE_MODE $i $WAX_MODE $PORTs $IPs | grep chrono | sed 's/.*=//')
	echo "$i,$thrres,$ompres" >> $logfile
	sleep 5 
	if [[ EXE_MODE -ne 0 ]]
	then
		warm_up_fpga $IPs $PORTs $i
	fi
	echo -e "\n Done with iteration $i\n"
done

echo -e "\n************ Benchmark Completed :) *******************\n"
