/*
 * comm_interface.h
 *
 *  Created on: 04.11.2021
 *      Author: spadmin
 */

#ifndef COMM_INTERFACE_H_
#define COMM_INTERFACE_H_

#include <stdint.h>

enum command{IPC_MSG_STOP_ALL=0,IPC_MSG_NEW_MSG=1};

#define NO_CHANNELS 6

/*
 * desCurrents : are in units of [mA]
 * desDuties : are integers in range [0,UINT16_MAX]
 */
struct mdriver_msg{
    int16_t desCurrents[NO_CHANNELS];
    uint16_t desCurrentsRes[NO_CHANNELS];
    uint16_t desDuties[NO_CHANNELS];
    uint32_t desFreqs[NO_CHANNELS];
    uint16_t stp_flg_byte;
    uint16_t buck_flg_byte;
    uint16_t regen_flg_byte;
    uint16_t  resen_flg_byte;
};

//struct for receiving the state of the TNB MNS system
struct mdriver_msg_sysstate{
    uint16_t states[NO_CHANNELS];
    int16_t currents[NO_CHANNELS];          // [mA]
    uint16_t duties[NO_CHANNELS];
    uint32_t freqs[NO_CHANNELS];
    int16_t dclink_voltages[NO_CHANNELS];      // [mA]
};


extern const unsigned int OFFSET_DES_DUTIES;
extern const unsigned int OFFSET_STP_FLG_BYTE;
extern const unsigned int OFFSET_BUCK_FLG_BYTE;
extern const unsigned int OFFSET_REGEN_FLG_BYTE;
extern const unsigned int OFFSET_RESEN_FLG_BYTE;


#endif /* COMM_INTERFACE_H_ */
