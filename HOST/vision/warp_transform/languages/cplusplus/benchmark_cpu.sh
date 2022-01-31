#!/bin/bash

THR=./out_thread
OMP=./out_openmp
max_power_threads=${1:-7}
thr_list=()
EXE_MODE=0
WAX_MODE=2

for i in $(seq 0 $max_power_threads)
do
	echo $((2**$i)) 
	thr_list+=($((2**$i)))
done

logfile=cpu_logger.csv
echo "thr[#],thread[ms],openmp[ms]" > $logfile
echo "Logging everythong"
for i in ${thr_list[@]}
do
	echo "Clean the out folders of $i"
	rm -rf ${THR}-$i/* ${OMP}-$i/*
	mkdir -p $THR-$i 
	mkdir -p $OMP-$i/
	echo "Executing OpenMP with $i threads"
	ompres=$(./warp_transform_host_parallel_openmp ../dataset/ $OMP-$i/ $EXE_MODE $i $WAX_MODE | grep chrono | sed 's/.*=//')
	echo "Executing std::thread with $i threads"
	thrres=$(./warp_transform_host_parallel_thread ../dataset/ $THR-$i/ $EXE_MODE $i $WAX_MODE | grep chrono | sed 's/.*=//')
	echo "$i,$thrres,$ompres" >> $logfile
done
