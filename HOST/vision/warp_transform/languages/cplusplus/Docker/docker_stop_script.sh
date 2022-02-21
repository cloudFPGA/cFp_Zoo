#!/bin/bash

cluster_size=${1:-1}

#container_name="zrlmpi_rank_"
container_name="rank_"

for (( i=0; i<$cluster_size; i++))
do
   tmp=$container_name
   mm=$(printf %02d $i)
   tmp+=$mm
   # docker stop $tmp
   docker kill $tmp

done
