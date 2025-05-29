# Use Ubuntu 24.04 as the base image
FROM ubuntu:24.04

# Set the working directory inside the container
WORKDIR /home/ubuntu/

#install CCS 12.8.1 dependencies
RUN apt update
RUN apt-get install curl libc6-i386 libusb-0.1-4 git -y
RUN curl --output libtinfo5_6.3-2ubuntu0.1_amd64.deb http://de.archive.ubuntu.com/ubuntu/pool/universe/n/ncurses/libtinfo5_6.3-2ubuntu0.1_amd64.deb
RUN dpkg -i libtinfo5_6.3-2ubuntu0.1_amd64.deb

#download CCS 12.8.1 and install
RUN curl --output ccs.tar.gz https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-J1VdearkvK/12.8.1/CCS12.8.1.00005_linux-x64.tar.gz
RUN tar -xzf ccs.tar.gz
RUN mkdir ti
RUN CCS12.8.1.00005_linux-x64/ccs_setup_12.8.1.00005.run --mode unattended --prefix /home/ubuntu/ti --enable-components PF_C28
RUN CCS_DIR=/home/ubuntu/ti/ccs
RUN rm ccs.tar.gz
RUN rm libtinfo5_6.3-2ubuntu0.1_amd64.deb

#create the source directory
RUN mkdir src 
RUN mkdir ccs_ws

#add the build test script
ADD ./test_build.sh ./test_build.sh

# clone the git repository and compile the CPU1 and CM project
CMD ["bash","./test_build.sh"]