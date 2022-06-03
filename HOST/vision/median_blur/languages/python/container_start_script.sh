#!/bin/bash


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
ip a
tc qdisc add dev enp35s0f0 root netem delay 0.4ms 0.2ms rate 10Gbit limit 10000000000000
tc qdisc show dev enp35s0f0

#ray start --head --port=6379 --num-cpus=7 --resources='{"cloudFPGA": 7}' --redis-password=1234 --object-manager-port=8076 --redis-shard-ports=6379 -include-dashboard=false


# keep container alive
# tail -f /dev/null
