#!/bin/bash
IMG_SIZES=(32 64 128 256 512 1024 2048 4096)
EXE_MODE=${1:-0}
bench_cmd=
if [[ EXE_MODE -eq 0 ]]
then
    bench_cmd=docker_run_srvr
else
    bench_cmd=docker_run_host
fi

for i in ${IMG_SIZES[@]}
do
    echo "sed -i \"s/#define FRAME_HEIGHT\s*[0-9]*/#define FRAME_HEIGHT $i/\" ../include/config.h"
    sed -i "s/#define FRAME_HEIGHT\s*[0-9]*/#define FRAME_HEIGHT $i/" ../include/config.h
    sed -i "s/#define FRAME_WIDTH\s*[0-9]*/#define FRAME_WIDTH $i/" ../include/config.h
    echo "run stuffs for $i"
    make $bench_cmd THREADS=32
done
