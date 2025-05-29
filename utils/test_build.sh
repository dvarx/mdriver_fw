#!/bin/bash

#clone the source code
cd src
git clone https://github.com/dvarx/mdriver_fw.git
cd mdriver_fw
CCS_DIR=/home/ubuntu/ti/ccs

#build CPU1 project
echo "building CPU1 project..."
$CCS_DIR/eclipse/eclipse -noSplash -data /home/ubuntu/ccs_ws -application com.ti.ccstudio.apps.projectImport -ccs.location /home/ubuntu/src/mdriver_fw/mdriver_cpu1
$CCS_DIR/eclipse/eclipse -noSplash -data /home/ubuntu/ccs_ws -application com.ti.ccstudio.apps.projectBuild -ccs.autoOpen -ccs.projects mdriver_cpu1 -ccs.configuration CPU1_FLASH

#build CM project
# echo "building CM project..."
$CCS_DIR/eclipse/eclipse -noSplash -data /home/ubuntu/ccs_ws -application com.ti.ccstudio.apps.projectImport -ccs.location /home/ubuntu/src/mdriver_fw/mdriver_cm
$CCS_DIR/eclipse/eclipse -noSplash -data /home/ubuntu/ccs_ws -application com.ti.ccstudio.apps.projectBuild -ccs.autoOpen -ccs.projects mdriver_cm -ccs.configuration Flash
