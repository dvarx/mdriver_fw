/*
 * mdriver_cpu1.c
 *
 *  Created on: 14.10.2021
 *      Author: dvarx
 */

#include <mdriver_cpu1.h>
#include <mdriver_fsm.h>
#include <mdriver_hw_defs.h>
#include "driverlib.h"
#include "device.h"
#include "fbctrl.h"

float frame_period_ms=200;

// ------------------------------------------------------------------------------------
// Pin & Pad Configuration Structures
// ------------------------------------------------------------------------------------

// channel a
struct buck_configuration cha_buck={40,41,8,GPIO_8_EPWM5A,9,GPIO_9_EPWM5B,EPWM5_BASE,false};
struct bridge_configuration cha_bridge={30,22,23,12,GPIO_12_EPWM7A,13,GPIO_13_EPWM7B,EPWM7_BASE,false,false};
struct driver_channel channela={0,&cha_buck,&cha_bridge,READY,87};

// channel b
struct buck_configuration chb_buck={35,60,15,GPIO_15_EPWM8B,14,GPIO_14_EPWM8A,EPWM8_BASE,false};
struct bridge_configuration chb_bridge={63,61,65,6,GPIO_6_EPWM4A,7,GPIO_7_EPWM4B,EPWM4_BASE,false,false};
struct driver_channel channelb={1,&chb_buck,&chb_bridge,READY,85};

// channel c
struct buck_configuration chc_buck={95,89,4,GPIO_4_EPWM3A,5,GPIO_5_EPWM3B,EPWM3_BASE,true};
struct bridge_configuration chc_bridge={107,133,93,0,GPIO_0_EPWM1A,1,GPIO_1_EPWM1B,EPWM1_BASE,false,false};
struct driver_channel channelc={2,&chc_buck,&chc_bridge,READY,83};

// channel d
struct buck_configuration chd_buck={32,33,2,GPIO_2_EPWM2A,3,GPIO_3_EPWM2B,EPWM2_BASE,true};
//struct bridge_configuration chd_bridge={48,49,54,10,GPIO_10_EPWM6A,11,GPIO_11_EPWM6B,EPWM6_BASE,false};
struct bridge_configuration chd_bridge={48,49,54,11,GPIO_11_EPWM6B,10,GPIO_10_EPWM6A,EPWM6_BASE,false,true};

struct driver_channel channeld={3,&chd_buck,&chd_bridge,READY,71}; // TODO : resonant enable pin

// channel e
struct buck_configuration che_buck={125,45,16,GPIO_16_EPWM9A,17,GPIO_17_EPWM9B,EPWM9_BASE,true};
struct bridge_configuration che_bridge={50,51,55,18,GPIO_18_EPWM10A,19,GPIO_19_EPWM10B,EPWM10_BASE,false,true};
struct driver_channel channele={4,&che_buck,&che_bridge,READY,73}; // TODO : resonant enable pin

// channel f
struct buck_configuration chf_buck={96,98,24,GPIO_24_EPWM13A,25,GPIO_25_EPWM13B,EPWM13_BASE,true};
struct bridge_configuration chf_bridge={52,53,56,26,GPIO_26_EPWM14A,27,GPIO_27_EPWM14B,EPWM14_BASE,false,true};
struct driver_channel channelf={5,&chf_buck,&chf_bridge,READY,126}; // TODO : resonant enable pin

struct driver_channel* driver_channels[NO_CHANNELS]={&channela,&channelb,&channelc,&channeld,&channele,&channelf};

// ---------------------
// Main Program related globals
// ---------------------

bool run_main_task=false;
struct system_dynamic_state system_dyn_state;
struct system_dynamic_state system_dyn_state_filtered;
uint32_t enable_res_cap_a=0;     //variable control the resonant relay of channel a, if it is set to 1, res cap switched in
uint32_t enable_res_cap_b=0;     //variable control the resonant relay of channel b, if it is set to 1, res cap switched in
uint32_t enable_res_cap_c=0;     //variable control the resonant relay of channel c, if it is set to 1, res cap switched in
float des_duty_bridge[NO_CHANNELS]={0.5,0.5,0.5,0.5,0.5,0.5};
float des_currents[NO_CHANNELS]={0.0,0.0,0.0,0.0,0.0,0.0};
float des_duty_buck[NO_CHANNELS]={0,0,0,0,0,0};
bool communication_active=false;

struct pi_controller current_pi[NO_CHANNELS]={
                                 {CTRL_KP,CTRL_KI,0.0,0.0,0.0,0.0},
                                 {CTRL_KP,CTRL_KI,0.0,0.0,0.0,0.0},
                                 {CTRL_KP,CTRL_KI,0.0,0.0,0.0,0.0},
                                 {CTRL_KP,CTRL_KI,0.0,0.0,0.0,0.0},
                                 {CTRL_KP,CTRL_KI,0.0,0.0,0.0,0.0},
                                 {CTRL_KP,CTRL_KI,0.0,0.0,0.0,0.0}
};
struct first_order des_duty_buck_filt[NO_CHANNELS]={
                                 {1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),-(1.0-2.0*TAU_BUCK_DUTY/deltaT)/(1.0+2.0*TAU_BUCK_DUTY/deltaT),0,0,0},
                                 {1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),-(1.0-2.0*TAU_BUCK_DUTY/deltaT)/(1.0+2.0*TAU_BUCK_DUTY/deltaT),0,0,0},
                                 {1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),-(1.0-2.0*TAU_BUCK_DUTY/deltaT)/(1.0+2.0*TAU_BUCK_DUTY/deltaT),0,0,0}
};
/*
struct first_order des_current_filt[NO_CHANNELS]={
                                  {1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),-(1.0-2.0*TAU_BUCK_DUTY/deltaT)/(1.0+2.0*TAU_BUCK_DUTY/deltaT),0,0,0},
                                  {1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),-(1.0-2.0*TAU_BUCK_DUTY/deltaT)/(1.0+2.0*TAU_BUCK_DUTY/deltaT),0,0,0},
                                  {1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),-(1.0-2.0*TAU_BUCK_DUTY/deltaT)/(1.0+2.0*TAU_BUCK_DUTY/deltaT),0,0,0},
                                  {1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),-(1.0-2.0*TAU_BUCK_DUTY/deltaT)/(1.0+2.0*TAU_BUCK_DUTY/deltaT),0,0,0},
                                  {1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),-(1.0-2.0*TAU_BUCK_DUTY/deltaT)/(1.0+2.0*TAU_BUCK_DUTY/deltaT),0,0,0},
                                  {1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),1.0/(1.0+2.0*TAU_BUCK_DUTY/deltaT),-(1.0-2.0*TAU_BUCK_DUTY/deltaT)/(1.0+2.0*TAU_BUCK_DUTY/deltaT),0,0,0}
};
*/

struct second_order_system des_current_filt[NO_CHANNELS]={
                                         {{1.000000000000000,-1.998743757640070,0.998744152176286},{0,1e-6*0.197309424095861,1e-6*0.197226792624946},0,0,0,0},
                                         {{1.000000000000000,-1.998743757640070,0.998744152176286},{0,1e-6*0.197309424095861,1e-6*0.197226792624946},0,0,0,0},
                                         {{1.000000000000000,-1.998743757640070,0.998744152176286},{0,1e-6*0.197309424095861,1e-6*0.197226792624946},0,0,0,0}
};

uint32_t des_freq_resonant_mhz[NO_CHANNELS]={DEFAULT_RES_FREQ_MILLIHZ,DEFAULT_RES_FREQ_MILLIHZ,DEFAULT_RES_FREQ_MILLIHZ};
float des_currents_res[NO_CHANNELS]={0.4};
struct mdriver_msg ipc_mdriver_msg;
#pragma DATA_SECTION(ipc_mdriver_state_msg, "MSGRAM_CPU_TO_CM")
struct mdriver_sysstate ipc_mdriver_state_msg;

// ---------------------
// Ripple Localization related variables
// ---------------------
//float ripplefreqs[NO_CHANNELS]={48,52,56,50,50,50};
float ripplefreqs[NO_CHANNELS]={70,90,110,50,50,50};
//float irippleamps[NO_CHANNELS]={0.8,0.8,0.8,0.4,0.4,0.4};
float irippleamps[NO_CHANNELS]={0.4,0.4,0.4,0.4,0.4,0.4};
float current_log[1024];

// ------------------------------------------------------------------------------------
// Main CPU Timer Related Functions
// ------------------------------------------------------------------------------------

uint16_t cpuTimer0IntCount;
uint16_t cpuTimer1IntCount;
uint16_t cpuTimer2IntCount;

//
// initCPUTimers - This function initializes all three CPU timers
// to a known state.
//
void
initCPUTimers(void)
{
    //
    // Initialize timer period to maximum
    //
    CPUTimer_setPeriod(CPUTIMER0_BASE, 0xFFFFFFFF);
    //CPUTimer_setPeriod(CPUTIMER1_BASE, 0xFFFFFFFF);
    //CPUTimer_setPeriod(CPUTIMER2_BASE, 0xFFFFFFFF);

    //
    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
    //
    CPUTimer_setPreScaler(CPUTIMER0_BASE, 0);
    //CPUTimer_setPreScaler(CPUTIMER1_BASE, 0);
    //CPUTimer_setPreScaler(CPUTIMER2_BASE, 0);

    //
    // Make sure timer is stopped
    //
    CPUTimer_stopTimer(CPUTIMER0_BASE);
    //CPUTimer_stopTimer(CPUTIMER1_BASE);
    //CPUTimer_stopTimer(CPUTIMER2_BASE);

    //
    // Reload all counter register with period value
    //
    CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);
    //CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);
    //CPUTimer_reloadTimerCounter(CPUTIMER2_BASE);

    //
    // Reset interrupt counter
    //
    cpuTimer0IntCount = 0;
    //cpuTimer1IntCount = 0;
    //cpuTimer2IntCount = 0;
}

//
// configCPUTimer - This function initializes the selected timer to the
// period specified by the "freq" and "period" variables. The "freq" is
// CPU frequency in Hz and the period in uSeconds. The timer is held in
// the stopped state after configuration.
//
void
configCPUTimer(uint32_t cpuTimer, uint32_t period)
{
    uint32_t temp, freq = DEVICE_SYSCLK_FREQ;

    //
    // Initialize timer period:
    //
    temp = ((freq / 1000000) * period);
    CPUTimer_setPeriod(cpuTimer, temp);

    //
    // Set pre-scale counter to divide by 1 (SYSCLKOUT):
    //
    CPUTimer_setPreScaler(cpuTimer, 0);

    //
    // Initializes timer control register. The timer is stopped, reloaded,
    // free run disabled, and interrupt enabled.
    // Additionally, the free and soft bits are set
    //
    CPUTimer_stopTimer(cpuTimer);
    CPUTimer_reloadTimerCounter(cpuTimer);
    CPUTimer_setEmulationMode(cpuTimer,
                              CPUTIMER_EMULATIONMODE_STOPAFTERNEXTDECREMENT);
    CPUTimer_enableInterrupt(cpuTimer);

    //
    // Resets interrupt counters for the three cpuTimers
    //
    if (cpuTimer == CPUTIMER0_BASE)
    {
        cpuTimer0IntCount = 0;
    }
    else if(cpuTimer == CPUTIMER1_BASE)
    {
        cpuTimer1IntCount = 0;
    }
    else if(cpuTimer == CPUTIMER2_BASE)
    {
        cpuTimer2IntCount = 0;
    }
}

//
// IPC ISR for Flag 0.
// This flag is set by the CM to send commands & data to CPU1
//
// IPC Defines TODO : Remove these variables & the single IPC communication at startup
#define IPC_CMD_READ_MEM   0x1001
#define IPC_CMD_RESP       0x2001

#define TEST_PASS          0x5555
#define TEST_FAIL          0xAAAA

__interrupt void IPC_ISR0()
{
    int i;
    uint32_t command, addr, data;
    bool status = false;

    //
    // Read the command
    //
    IPC_readCommand(IPC_CPU1_L_CM_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE,
                    &command, &addr, &data);

    if(command == IPC_MSG_NEW_MSG){
        //copy tnb mns message
        memcpy(&ipc_mdriver_msg,(struct mdriver_msg*)addr,sizeof(ipc_mdriver_msg));

        //check flags and copy their values in the flag arrays used by the FSM
        unsigned short i=0;

        for(i=0; i<NO_CHANNELS; i++){
            //check the BUCK_EN byte
            if(ipc_mdriver_msg.buck_flg_byte&(1<<i))
                fsm_req_flags_en_buck[i]=true;
            //check the STOP byte
            if(ipc_mdriver_msg.stp_flg_byte&(1<<i))
                fsm_req_flags_stop[i]=true;
            //check the RUN_REGULAR byte
            if(ipc_mdriver_msg.regen_flg_byte&(1<<i))
                fsm_req_flags_run_regular[i]=true;
            //check the RUN_RESONANT byte
            if(ipc_mdriver_msg.resen_flg_byte&(1<<i))
                fsm_req_flags_run_resonant[i]=true;
        }
        //check the currents and duties and translate them
        //do not allow modifying these values when the system is not running, e.g.
        //do not modify values when system in READY state
        for(i=0; i<NO_CHANNELS; i++){
            if(driver_channels[i]->channel_state==RUN_REGULAR||driver_channels[i]->channel_state==BUCK_ENABLED||driver_channels[i]->channel_state==RUN_RESONANT){
                //currents are sent in units of [mA]
                des_currents[i]=(float)(ipc_mdriver_msg.desCurrents[i])*1e-3;
                des_duty_buck[i]=(float)(ipc_mdriver_msg.desDuties[i])*(1.0/(float)(UINT16_MAX));
                des_currents_res[i]=(float)(ipc_mdriver_msg.desCurrentsRes[i])*1e-3;

            }
            else{
                des_currents[i]=0.0;
                des_duty_buck[i]=0.0;
                des_currents_res[i]=0.4;
            }
        }

        //fill out the system dynamic state and send back to CM
        unsigned int channelno=0;
        for(channelno=0; channelno<NO_CHANNELS; channelno++){
            ipc_mdriver_state_msg.states[channelno]=driver_channels[channelno]->channel_state;
        }
        IPC_sendCommand(IPC_CPU1_L_CM_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE,
                        IPC_MSG_NEW_MSG, &ipc_mdriver_state_msg, sizeof(ipc_mdriver_state_msg));

        //in this branch we interpret these fields as the frame time of the oscillations
        for(i=0; i<NO_CHANNELS; i++){
            frame_period_ms=(uint32_t)(ipc_mdriver_msg.desFreqs[0]);
        }
        //set the communication_active variable
        communication_active=true;
        //reset the timer
        CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);
    }

    if(command == IPC_CMD_READ_MEM)
    {
        status = true;

        //
        // Read and compare data
        //
        for(i=0; i<data; i++)
        {
            if(*((uint32_t *)addr + i) != i)
                status = false;
        }

        //
        // Send response to C28x core
        //
        if(status)
        {
            IPC_sendResponse(IPC_CPU1_L_CM_R, TEST_PASS);
        }
        else
        {
            IPC_sendResponse(IPC_CPU1_L_CM_R, TEST_FAIL);
        }
    }

    //
    // Acknowledge the flag
    //
    IPC_ackFlagRtoL(IPC_CPU1_L_CM_R, IPC_FLAG0);

    //
    // Acknowledge Interrupt Group
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP11);
}

//
// cpuTimer0ISR - Counter for CpuTimer0
//
__interrupt void
cpuTimer0ISR(void)
{
    cpuTimer0IntCount++;

    run_main_task=true;

    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

//
// cpuTimer1ISR - Counter for CpuTimer1
//
__interrupt void cpuTimer1ISR(void){
    communication_active=false;

    //reset CPU Timer 1 (it counts downwards)
    CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);
}
