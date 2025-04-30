/*
 * comm_interface.c
 *
 *  Created on: 04.11.2021
 *      Author: spadmin
 */

#include "comm_interface.h"

const char CMD_STOP[]="stop";
const char ENABLE_ALL_BUCK[]="enab";

const unsigned int OFFSET_DES_DUTIES=NO_CHANNELS*2;
const unsigned int OFFSET_STP_FLG_BYTE=NO_CHANNELS*2+NO_CHANNELS*2;
const unsigned int OFFSET_BUCK_FLG_BYTE=OFFSET_STP_FLG_BYTE+2;
const unsigned int OFFSET_REGEN_FLG_BYTE=OFFSET_BUCK_FLG_BYTE+2;
const unsigned int OFFSET_RESEN_FLG_BYTE=OFFSET_REGEN_FLG_BYTE+2;
