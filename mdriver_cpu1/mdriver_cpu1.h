/*
 * mdriver_cpu1.h
 *
 *  Created on: 14.10.2021
 *      Author: dvarx
 */

#ifndef MDRIVER_CPU1_H_
#define MDRIVER_CPU1_H_

#include <mdriver_cpu1.h>
#include <mdriver_hw_defs.h>
#include <stdint.h>
#include "driverlib.h"
#include "device.h"
#include "stdint.h"
#include "fbctrl.h"
#include "comm_interface.h"

#define HEARTBEAT_GPIO 70
#define MAIN_RELAY_GPIO 92
#define SLAVE_RELAY_GPIO 62
#define DEFAULT_RES_FREQ_MILLIHZ    10000000
#define MINIMUM_RES_FREQ_MILLIHZ    80000
#define LED_1_GPIO 31
#define LED_2_GPIO 34
#define COMMUNICATION_TIMEOUT_MS    500
//define the pin polarity of the voltage needed to enable the ate driver
#define DRIVER_ENABLE_POLARITY 0
#define DRIVER_DISABLE_POLARITY 1
extern float frame_period_ms;

struct buck_configuration{
    uint32_t enable_gpio;
    uint32_t state_gpio;
    uint32_t bridge_h_pin;
    uint32_t bridge_h_pinconfig;
    uint32_t bridge_l_pin;
    uint32_t bridge_l_pinconfig;
    uint32_t epwmbase;
    bool is_inverted;
};

struct bridge_configuration{
    uint32_t enable_gpio;
    uint32_t state_u_gpio;
    uint32_t state_v_gpio;
    uint32_t bridge_h_pin;
    uint32_t bridge_h_pinconfig;
    uint32_t bridge_l_pin;
    uint32_t bridge_l_pinconfig;
    uint32_t epwmbase;
    bool resonant_active;
    bool is_inverted;
};

struct system_dynamic_state{
    float is[NO_CHANNELS];      //A
    float vs[NO_CHANNELS];      //A
    float is_res[NO_CHANNELS];  //A
};

enum driver_channel_state {READY=0,BUCK_ENABLED=1,INIT_REGULAR=2,RUN_REGULAR=3,INIT_RESONANT=4,RUN_RESONANT=5,FAULT=6,TERMINATE_RESONANT=7,TERMINATE_REGULAR=8};

struct driver_channel{
    uint8_t channel_no;
    struct buck_configuration* buck_config;
    struct bridge_configuration* bridge_config;
    enum driver_channel_state channel_state;
    uint32_t enable_resonant_gpio;
};

extern struct buck_configuration cha_buck;
extern struct buck_configuration chb_buck;
extern struct buck_configuration chc_buck;
extern struct buck_configuration chd_buck;
extern struct buck_configuration che_buck;
extern struct buck_configuration chf_buck;
extern struct driver_channel channela;
extern struct driver_channel channelb;
extern struct driver_channel channelc;
extern struct driver_channel channeld;
extern struct driver_channel channele;
extern struct driver_channel channelf;
extern struct driver_channel* driver_channels[NO_CHANNELS];

extern struct bridge_configuration cha_bridge;
extern struct bridge_configuration chb_bridge;
extern struct bridge_configuration chc_bridge;
extern struct bridge_configuration chd_bridge;
extern struct bridge_configuration che_bridge;
extern struct bridge_configuration chf_bridge;

extern struct mdriver_sysstate ipc_mdriver_state_msg;

// ---------------------
// Main Program related globals
// ---------------------

extern bool run_main_task;                              //variable is set by CPU1 ISR
extern struct system_dynamic_state system_dyn_state;
extern float des_duty_bridge[NO_CHANNELS];             //desired duties for bridges, set by COMM interface
extern float des_duty_buck[NO_CHANNELS];               //desired duties for bucks, set by COMM interface
extern float des_currents[NO_CHANNELS];
extern uint32_t des_freq_resonant_mhz[NO_CHANNELS];     //desired frequencies for resonant bridges, set by COMM interface
extern float des_currents_res[NO_CHANNELS];
extern struct first_order des_duty_buck_filt[NO_CHANNELS];
extern struct second_order_system des_current_filt[NO_CHANNELS];
extern struct pi_controller current_pi[NO_CHANNELS];
extern bool communication_active;                       //variable indicates whether there is a TCP connection active (true if a package was received in the last 200ms)

// ---------------------
// Ripple Localization related variables
// ---------------------
extern float irippleamps[NO_CHANNELS];
extern float ripplefreqs[NO_CHANNELS];
extern float current_log[1024];

// ---------------------
// Main CPU Timer Related Functions
// ---------------------

//
// Globals
//
extern uint16_t cpuTimer0IntCount;
//extern uint16_t cpuTimer1IntCount;
//extern uint16_t cpuTimer2IntCount;

__interrupt void cpuTimer0ISR(void);
__interrupt void IPC_ISR0(void);
__interrupt void cpuTimer1ISR(void);
void initCPUTimers(void);
void configCPUTimer(uint32_t, uint32_t);


#endif /* MDRIVER_CPU1_H_ */
