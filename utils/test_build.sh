#!/bin/bash

#clone the source code
cd src
git clone https://github.com/$REPO_NAME.git -b $BRANCH_NAME

REPO_NAME_SHORT="${REPO_NAME#*/}"
CCS_DIR=/home/ubuntu/ti/ccs
C2000WareDir=/home/ubuntu/ti/C2000Ware_3_04_00_00
SRC_DIR=/home/ubuntu/src
export CCS_DIR
export SRC_DIR
export C2000WareDir

#build the CPU1 code
cd $HOME/src/$REPO_NAME_SHORT/mdriver_cpu1/build_dir
$CCS_DIR/utils/bin/gmake -k -j 14 all -O

#build the CM code
cd $HOME/src/$REPO_NAME_SHORT/mdriver_cm/build_dir
$CCS_DIR/utils/bin/gmake -k all
