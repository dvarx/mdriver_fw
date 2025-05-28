#!/bin/bash

#build CPU1 project
echo "building CPU1 project..."
$HOME/ti/ccs1281/ccs/eclipse/eclipse -noSplash -data "$HOME/workspace_v12" -application com.ti.ccstudio.apps.projectBuild -ccs.autoOpen -ccs.projects mdriver_cpu1 -ccs.configuration CPU1_FLASH

#build CM project
echo "building CM project..."
$HOME/ti/ccs1281/ccs/eclipse/eclipse -noSplash -data "$HOME/workspace_v12" -application com.ti.ccstudio.apps.projectBuild -ccs.autoOpen -ccs.projects mdriver_cm -ccs.configuration Flash
