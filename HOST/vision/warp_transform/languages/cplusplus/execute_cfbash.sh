#!/bin/bash

#if greater than 1 use the lite version
light=$1
# where to find the input imgs. expected N fldrs dataset_1 dataset_2 ... dataset_N
in_dir="./dataset"
# same as for input. expected N fldrs out_1 out_2 ... out_N
out_dir="./out"

#how many threads
threads=$(seq 0 2)
# max threads but starting from 1
fldrs=($(seq 3))
#fpga in the cluster ips
ips=("10.12.200.129" "10.12.200.169" "10.12.200.54")
#ports for the fpga in the clusters
ports=("2718" "2719" "2720")

executable_lite=warp_transform_host_lightweight
executable_normal=warp_transform_host
#number of imgs to consider
top_val=33

if [[ $light -gt 1 ]]
then
	executable=$executable_lite
else
	executable=$executable_normal
fi

for i in $(seq 1 $top_val)
do
	echo "my idx is $i"
	for t in $threads
	do
		echo "my thread is $t"
		if [[ ${fldrs[$t]} -lt 4 ]]
		then
			echo "./${executable} ${ips[$t]} ${ports[$t]} ${in_dir}_${fldrs[$t]}/img${i}.png ${out_dir}_${fldrs[$t]}/ 2" 
			./${executable} ${ips[$t]} ${ports[$t]} ${in_dir}_${fldrs[$t]}/img${i}.png ${out_dir}_${fldrs[$t]}/ 2
		else
			echo "./${executable} ${ips[$t]} ${ports[$t]} ${in_dir}_${fldrs[$t]}/img${i}.png ${out_dir}_${fldrs[$t]}/ 2"
			./${executable} ${ips[$t]} ${ports[$t]} ${in_dir}_${fldrs[$t]}/img${i}.png ${out_dir}_${fldrs[$t]}/ 2
		fi
	done

done
exit(1)
