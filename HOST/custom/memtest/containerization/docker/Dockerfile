# Copyright (c) Jupyter Development Team.
# Distributed under the terms of the Modified BSD License.
ARG BASE_CONTAINER=jupyter/minimal-notebook
FROM $BASE_CONTAINER

LABEL maintainer="Jupyter Project <jupyter@googlegroups.com>"

USER root

# Install all OS dependencies for fully functional notebook server
RUN apt-get update && apt-get install -yq --no-install-recommends \
    cmake \
    pkg-config \
    libxml2-dev \
    rename \
    rpl \
    python3-dev \
    swig \
    curl \
    net-tools \ 
    iproute2 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

    
# Expose port & cmd
EXPOSE 8888 2718 1234

# Set default work directory
RUN mkdir /workspace
WORKDIR /workspace

# Copy the file from your host to your current location.
COPY . /workspace

SHELL ["/bin/bash", "-c"]

# Switch back to jovyan to avoid accidental container runs as root
#USER $NB_UID

RUN /bin/bash -c "source env/setenv.sh && cd HOST/custom/memtest/languages/python/ && rm -rf build && mkdir build && cd build && cmake ../ && make -j 2 && cd ../../../../"

CMD /bin/bash -c "source ~/.bashrc && source env/setenv.sh && export PYTHONPATH=/workspace/HOST/custom/memtest/languages/python/build && export FPGA_IP=`bash ZYC2/create_cluster_and_get_ip.sh` && jupyter lab --ip=0.0.0.0 --port=8888 --no-browser --notebook-dir=/workspace/HOST/custom/memtest/languages/python --allow-root"

