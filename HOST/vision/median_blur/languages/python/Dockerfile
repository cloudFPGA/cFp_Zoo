FROM rayproject/ray 

USER root
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install openssh-server sudo build-essential htop iproute2 inetutils-ping git python3-virtualenv python3-pip libgl1-mesa-glx vim -y

#RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 test 

#RUN  echo 'test:test' | chpasswd

RUN echo "ALL ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

RUN service ssh start

EXPOSE 22
EXPOSE 2718

#USER test

SHELL ["/bin/bash", "-c"]

WORKDIR /app

RUN git clone https://github.com/cloudFPGA/cFp_Zoo.git /app/cFp_Zoo
WORKDIR /app/cFp_Zoo
RUN source env/setenv.sh
WORKDIR /app/cFp_Zoo/HOST/vision/median_blur/languages/python/
RUN pip install -r ./requirements_ray.txt
EXPOSE 8076
EXPOSE 10001
EXPOSE 6379
RUN chmod +x /app/cFp_Zoo/HOST/vision/median_blur/languages/python/container_start_script.sh

#CMD ["/usr/sbin/sshd","-D"]
CMD ["/app/cFp_Zoo/HOST/vision/median_blur/languages/python/container_start_script.sh"]
