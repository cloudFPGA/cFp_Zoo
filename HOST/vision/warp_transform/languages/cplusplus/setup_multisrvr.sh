#!/bin/bash

echo "Usage <# of threads> <port list p0:p1:p2>"
max_power_threads=${1:-2}
thr_list=()
PORTs=("2718" "2719" "2720" "2721" "2722" "2723" "2724" "2725" "2726" "2727" "2728" "2729" "2730" "2731")
thr_list=($(seq 1 $max_power_threads))
len=${#thr_list[@]}
updated_len=$((len-1))
echo -e "\n************ Server setup Begins :) *******************\n"

for i in $(seq 0 $updated_len )
do
    ./warp_transform_srvr_cpu ${PORTs[$i]} &
done

echo -e "\n************  Server Setup Completed :) *******************\n"
