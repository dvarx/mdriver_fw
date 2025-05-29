# Use Ubuntu 24.04 as the base image
FROM ubuntu:24.04

# Set the working directory inside the container
WORKDIR /home/ubuntu/

#install CCS 12.8.1 dependencies
RUN apt update
RUN apt-get install curl libc6-i386 libusb-0.1-4 git sudo -y
#RUN curl --output libtinfo5_6.3-2ubuntu0.1_amd64.deb http://de.archive.ubuntu.com/ubuntu/pool/universe/n/ncurses/libtinfo5_6.3-2ubuntu0.1_amd64.deb
ADD ./libtinfo5_6.3-2ubuntu0.1_amd64.deb ./libtinfo5_6.3-2ubuntu0.1_amd64.deb
RUN dpkg -i libtinfo5_6.3-2ubuntu0.1_amd64.deb
RUN rm libtinfo5_6.3-2ubuntu0.1_amd64.deb

#download CCS 12.8.1 and install
#RUN curl --output ccs.tar.gz https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-J1VdearkvK/12.8.1/CCS12.8.1.00005_linux-x64.tar.gz
#Note: docker automatically extracts the archive when adding an archive to an image. the directory name is the same as the archive filename
USER ubuntu
ADD ./CCS12.8.1.00005_linux-x64.tar.gz ./ccs
RUN mkdir ti
RUN ./ccs/CCS12.8.1.00005_linux-x64/ccs_setup_12.8.1.00005.run --mode unattended --prefix /home/ubuntu/ti --enable-components PF_C28
RUN CCS_DIR=/home/ubuntu/ti/ccs

#install C2000 Compiler
#ADD ./ti_cgt_c2000_20.2.5.LTS_linux-x64_installer.bin ./ti_cgt_c2000_20.2.5.LTS_linux-x64_installer.bin
#RUN ./ti_cgt_c2000_20.2.5.LTS_linux-x64_installer.bin ----unattendedmodeui none --mode unattended --prefix /home/ubuntu/ti/ccs/tools/compiler

#set up C2000Ware 3.04
#https://www.ti.com/tool/download/C2000WARE/3.04.00.00 (authentication required for download)
ADD ./C2000Ware_3_04_00_00_setup.run ./C2000Ware_3_04_00_00_setup.run
RUN ./C2000Ware_3_04_00_00_setup.run --unattendedmodeui none --mode unattended --prefix /home/ubuntu/ti


#create the source directory
RUN mkdir src 
RUN mkdir ccs_ws

#add the build test script
ADD ./test_build.sh ./test_build.sh

ENV PATH="${PATH}:/home/ubuntu/ti/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin"

# clone the git repository and compile the CPU1 and CM project
CMD ["bash","./test_build.sh"]