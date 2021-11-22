#!/bin/bash

bash ./ZYC2/post_cluster.sh `bash ./ZYC2/get_cpu_ip.sh` | grep role_ip | awk '{print $2}' | tr -d '"' | tr -d ','
 