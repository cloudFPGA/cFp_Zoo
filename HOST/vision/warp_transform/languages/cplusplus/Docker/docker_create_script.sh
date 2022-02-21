#!/bin/bash 

mode=${1:-0}
cluster_size=${2:-1}
THREADS=${3:-1}
BENCH_PORTS=${4:-}
BENCH_IPS=${5:-}

#ip_addr_begin="10.12.10.1"
ip_addr_begin="192.168.0.1"

ENV_ALL=" --env THREADS=$THREADS --env PORTS=$BENCH_PORTS --env LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lib"
ENV_BENCHMARK=" --env IPS=$BENCH_IPS "
# docker network create -d bridge br0 --subnet=192.168.0.100/16 --> used this
# docker network create --driver=bridge --subnet=192.168.0.0/16 br0
# docker network create -d macvlan --subnet=10.12.0.0/16 -o parent=enp1s0f0 --ip-range=10.12.50.0/25 net0
dockercmd_base="docker run -d --rm -it --network br0 --cap-add=NET_ADMIN  --ip 192.168.0.1X --name rank_X --env ZRLMPI_CLUSTER_SIZE=$cluster_size"
# dockercmd_base="docker run -d --rm -it --network br0 --cap-add=NET_ADMIN  --ip 192.168.0.1X --name rank_X --cpuset-cpus Y --env ZRLMPI_CLUSTER_SIZE=$cluster_size"

#dockercmd="docker run -d --rm -it --network usernet --cap-add=NET_ADMIN -m 16g --ip 10.12.10.1X --name zrlmpi_rank_Y --cpuset-cpus Y --env ZRLMPI_CLUSTER_SIZE B ngl_runner "
#dockercmd="docker run -d --rm -it --network br0 --cap-add=NET_ADMIN -m 16g --ip 192.168.0.1X --name zrlmpi_rank_X --cpuset-cpus Y --env ZRLMPI_CLUSTER_SIZE=$cluster_size B ngl_runner "
if [[ $mode -eq 0 ]]
then
  echo "Server mode"
  dockercmd="$dockercmd_base $ENV_ALL PLACEHOLDER dco_runner_srvr "
else
  echo "Benchmark Mode"
  dockercmd="$dockercmd_base $ENV_ALL $ENV_BENCHMARK PLACEHOLDER dco_runner "
fi

first_container=""

for (( i=0; i<$cluster_size; i++))
#for (( i=1; i<$cluster_size; i++))
do
   mm=$(printf %02d $i)
   echo "MM $mm"
   tmp=$(echo $dockercmd | sed -r "s/X/$mm/g" )
   echo "tmp $tmp"

  #  tmp2=$(echo $tmp | sed -r "s/Y/$i/g" )
  #  echo "tmp2 $tmp2"

   #echo $tmp2
   oip=$ip_addr_begin 
   oip+=$mm
   #echo $oip
   envs="--env ZRLMPI_OWN_IP=$oip --env ZRLMPI_OWN_RANK=$i --env ZRLMPI_OTHER_IP_LIST='"
   olip=""
   for (( j=0; j<$i; j++))
   do 
	   jj=$(printf %02d $j)
	   olip+=$ip_addr_begin
	   olip+=$jj
	   olip+=" "
   done
   for (( j=$i+1; j<$cluster_size; j++))
   do 
	   jj=$(printf %02d $j)
	   olip+=$ip_addr_begin
	   olip+=$jj
	   olip+=" "
   done
   envs+=$olip
   envs+="'"
   
   tmp3=$(echo $tmp | sed -r "s/PLACEHOLDER/$envs/g" )
   echo "gno $tmp3"

   #$tmp3
   #"${tmp3[@]}"
   
   if [[ $i == 0 ]]
   then
     first_container+=$tmp3
   else
     echo $tmp3
     eval $tmp3
    fi
done

echo $first_container
eval $first_container

docker container ls

echo "started at: $(date)"

docker attach rank_00

echo -e "\nremaining containers:"

#docker container ls
docker ps -a


