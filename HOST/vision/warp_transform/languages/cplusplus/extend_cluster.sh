image_id=$1
fpgas_nmbr=${2:-1}
cluster_id=${4:-260}
fpga_offs=${3:-0}

for i in $(seq $fpga_offs $fpgas_nmbr)
do
	cfsp cluster extend --cluster_id $cluster_id --image_id=${image_id} --node_id $i
done