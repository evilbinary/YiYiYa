# docker image for compiling YiYiYa on ARM platform
# 1. build image: docker build -t yiyiya_build:latest .
# 2. run container: docker run -it -v $(pwd):/var/YiYiYa yiyiya_build:latest /bin/sh -c "make all"
# 3. in the shell run: qemu-system-arm -name YiYiYa -M raspi2b -rtc base=localtime -kernel build/raspi2/armv7-a/debug/kernel.elf -serial stdio -D ./qemu.log -drive if=sd,id=sd0,format=raw,file=image/disk.img
FROM ubuntu:22.04
RUN apt update
RUN apt install -y wget
RUN apt install -y gcc-arm-none-eabi
RUN apt install -y mtools u-boot-tools dosfstools
RUN apt install -y make
RUN apt install -y python3.11
RUN ln -s /usr/bin/python3.11 /usr/bin/python
RUN wget https://bootstrap.pypa.io/get-pip.py -P /var/tmp
RUN python /var/tmp/get-pip.py
RUN pip install yymake
WORKDIR /var/YiYiYa
