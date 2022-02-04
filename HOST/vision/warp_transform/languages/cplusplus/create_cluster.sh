image_id=$1
fpgas_nmbr=${2:-1}
cpu_ip=${3:-"10.12.0.10"}
fpgas_nodes=()

for i in $(seq $fpgas_nmbr)
do
        fpgas_nodes+=("--image_id=${image_id}")
done

echo "cfsp cluster post --node_ip=$cpu_ip ${fpgas_nodes[@]}"
cfsp cluster post --node_ip=$cpu_ip ${fpgas_nodes[@]}
~
