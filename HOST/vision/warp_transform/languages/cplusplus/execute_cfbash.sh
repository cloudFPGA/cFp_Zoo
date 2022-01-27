#!/bin/bash

#number of threads to use. count from 0
max_thread=${1:-0}
#if greater than 1 use the lite version
light=${2:-0}
#the amount of imgs from 1 to max_imgs
max_imgs=${3:-33}
# where to find the input imgs. expected N fldrs dataset_1 dataset_2 ... dataset_N
in_dir="./dataset"
# same as for input. expected N fldrs out_1 out_2 ... out_N
out_dir="./out"

#how many threads
#echo "Max threads $max_thread"
threads=$(seq 0 ${max_thread})
# max threads but starting from 1
fldrs=($(seq 3))
#fpga in the cluster ips
ips=("10.12.200.9" "10.12.200.16" "10.12.200.241")
#ips=("10.12.200.16" "10.12.200.9" "10.12.200.241")
#ips=("10.12.200.241" "10.12.200.16" "10.12.200.9")
#ports for the fpga in the clusters
ports=("2718" "2719" "2720")

executable_lite=warp_transform_host_lightweight
executable_normal=warp_transform_host
#number of imgs to consider
top_val=$max_imgs
sleep_time="0.001s"

if [[ $light -gt 1 ]]
then
	executable=$executable_lite
else
	executable=$executable_normal
fi

for i in $(seq 1 $top_val)
do
	echo "my idx is $i"
	#echo ${threads[@]}
	for t in $threads
	do
		echo "my thread is $t"
		if [[ ${fldrs[$t]} -lt 4 ]]
		then
			echo "./${executable} ${ips[$t]} ${ports[$t]} ${in_dir}_${fldrs[$t]}/img${i}.png ${out_dir}_${fldrs[$t]}/ 2" 
			./${executable} ${ips[$t]} ${ports[$t]} ${in_dir}_${fldrs[$t]}/img${i}.png ${out_dir}_${fldrs[$t]}/ 2
		else
			echo "./${executable} ${ips[$t]} ${ports[$t]} ${in_dir}_${fldrs[$t]}/img${i}.png ${out_dir}_${fldrs[$t]}/ 2"
			#./${executable} ${ips[$t]} ${ports[$t]} ${in_dir}_${fldrs[$t]}/img${i}.png ${out_dir}_${fldrs[$t]}/ 2
		fi
	done
	#sleep $sleep_time

done
exit
