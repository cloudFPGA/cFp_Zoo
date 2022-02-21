#!/bin/bash

# DO NOT FORGET TO UPDATE zrlmpi_cpu_app!!


# Limit all incoming and outgoing network to 10Gb/s
#sudo tc qdisc add dev eth0 root tbf rate 10gbit latency 0.4ms burst 1540

# https://wiki.linuxfoundation.org/networking/netem
#DELAY_MS=0.4
#RATE_GBIT=10
#BUF_PKTS=33
#BDP_BYTES=$(echo "($DELAY_MS/1000.0)*($RATE_MBIT*1000000.0/8.0)" | bc -q -l)
#BDP_PKTS=$(echo "$BDP_BYTES/1500" | bc -q)
#LIMIT_PKTS=$(echo "$BDP_PKTS+$BUF_PKTS" | bc -q)
#sudo tc qdisc add dev eth0 root netem delay ${DELAY_MS}ms rate ${RATE_GBIT}Gbit limit ${LIMIT_PKTS}

#sudo tc qdisc add dev eth0 root netem delay 0.4ms 0.2ms rate 10Gbit limit 100000000 loss 0.0000125% 1%
sudo tc qdisc add dev eth0 root netem delay 0.4ms 0.2ms rate 10Gbit limit 10000000000000
sudo tc qdisc show dev eth0

#start ZRLMPI binary

#if not there
cd /app 
#
if [[ $ZRLMPI_OWN_RANK == 0 ]]; then
	echo "sleeping..."
	s=$(( 4*$ZLMPI_CLUSTER_SIZE + 5 ))
	sleep $s
fi

# zrlmpicmd="./zrlmpi_cpu_app udp $ZRLMPI_OWN_IP $ZRLMPI_CLUSTER_SIZE $ZRLMPI_OWN_RANK $ZRLMPI_OTHER_IP_LIST"
zrlmpicmd="./setup_multisrvr.sh $THREADS"



echo $zrlmpicmd

#$zrlmpicmd | tee run.log
$zrlmpicmd

echo "docker done"

if [[ $ZRLMPI_OWN_RANK == 0 ]]; then
	/bin/bash
#else
	# start ssh
#	sudo /usr/sbin/sshd -D 
fi


