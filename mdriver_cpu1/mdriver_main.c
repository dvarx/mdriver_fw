//#############################################################################
// mdriver_main.c
//
// derived from
// FILE:   cm_common_config_c28x.c
//
// TITLE:  C28x Common Configurations to be used for the CM Side.
//
//! \addtogroup driver_example_list
//! <h1>C28x Common Configurations</h1>
//!
//! This example configures the GPIOs and Allocates the shared peripherals
//! according to the defines selected by the users.
//!
//
//#############################################################################
// $TI Release: F2838x Support Library v3.04.00.00 $
// $Release Date: Fri Feb 12 19:08:49 IST 2021 $
// $Copyright:
// Copyright (C) 2021 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include <stdbool.h>
#include "fbctrl.h"
#include <math.h>
#include <mdriver_adc.h>
#include <mdriver_cpu1.h>
#include <mdriver_cpu1.h>
#include <mdriver_cpu1.h>
#include <mdriver_epwm.h>
#include <mdriver_fsm.h>
#include <mdriver_hw_defs.h>
#include "ipc.h"

bool run_main_control_task=false;
bool enable_waveform_debugging=false;

void main(void)
{

    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Boot CM core
    //
#ifdef _FLASH
    Device_bootCM(BOOTMODE_BOOT_TO_FLASH_SECTOR0);
#else
    Device_bootCM(BOOTMODE_BOOT_TO_S0RAM);
#endif

    //
    // Disable pin locks and enable internal pull-ups.
    //
    Device_initGPIO();

    //
    // Setup heartbeat GPIO & input relay gpios
    //heartbeat
    GPIO_setDirectionMode(HEARTBEAT_GPIO, GPIO_DIR_MODE_OUT);   //output
    GPIO_setPadConfig(HEARTBEAT_GPIO,GPIO_PIN_TYPE_STD);        //push pull output
    //main input relay
    GPIO_setDirectionMode(MAIN_RELAY_GPIO, GPIO_DIR_MODE_OUT);   //output
    GPIO_setPadConfig(MAIN_RELAY_GPIO,GPIO_PIN_TYPE_STD);        //push pull output
    //input relay to slave
    GPIO_setDirectionMode(SLAVE_RELAY_GPIO, GPIO_DIR_MODE_OUT);   //output
    GPIO_setPadConfig(SLAVE_RELAY_GPIO,GPIO_PIN_TYPE_STD);        //push pull output
    //LED 1 for debugging
    GPIO_setDirectionMode(LED_1_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(LED_1_GPIO,GPIO_PIN_TYPE_STD);
    GPIO_writePin(LED_1_GPIO,1);
    //LED 2 for debugging
    GPIO_setDirectionMode(LED_2_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(LED_2_GPIO,GPIO_PIN_TYPE_STD);
    GPIO_writePin(LED_2_GPIO,1);
    //start of frame (SOF) GPIO
    GPIO_setDirectionMode(SOF_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SOF_GPIO,GPIO_PIN_TYPE_STD);
    GPIO_writePin(SOF_GPIO,1);

    //
    // Initialize ADCs
    //
    //setup ADC clock, single-ended mode, enable ADCs
    initADCs();
    //setup ADC SOC configurations
    initADCSOCs();

    //
    // Initialize Half Bridges
    //
    //channel A
    setup_pin_config_buck(&cha_buck);
    setup_pinmux_config_bridge(&cha_bridge,0);
    //channel B
    setup_pin_config_buck(&chb_buck);
    setup_pinmux_config_bridge(&chb_bridge,1);
    //channel C
    setup_pin_config_buck(&chc_buck);
    setup_pinmux_config_bridge(&chc_bridge,2);
    //channel D
    setup_pin_config_buck(&chd_buck);
    setup_pinmux_config_bridge(&chd_bridge,3);
    //channel E
    setup_pin_config_buck(&che_buck);
    setup_pinmux_config_bridge(&che_bridge,4);
    //channel F
    setup_pin_config_buck(&chf_buck);
    setup_pinmux_config_bridge(&chf_bridge,5);


    //
    // Initialize synchronization of bridges
    //
    //enable phase synchronization for all PWM channels
    //synchronization is to ePWM12 module on Pin22
    unsigned int channelno=0;
    for(channelno=0; channelno< NO_CHANNELS; channelno++){
        synchronize_pwm_to_epwm12(driver_channels,channelno);
    }
    //enable PWM outputs of ePWM11
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_OUT);   //output
    GPIO_setPadConfig(20,GPIO_PIN_TYPE_STD);        //push pull output
    GPIO_setPinConfig(GPIO_20_EPWM11A);
    //set up ePWM11 (DAQ sample clock)
    initEPWMWithoutDB(EPWM11_BASE,false);
    setupEPWMActiveHighComplementary(EPWM11_BASE);
    EPWM_setClockPrescaler(EPWM11_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_8);
    EPWM_setTimeBasePeriod(EPWM11_BASE, EPWM_TIMER_TBPRD_BRIDGE/2);
    EPWM_setCounterCompareValue(EPWM11_BASE, EPWM_COUNTER_COMPARE_A, EPWM_TIMER_TBPRD_BRIDGE/4);
    //enable PWM outputs of EPWM5
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_OUT);   //output
    GPIO_setPadConfig(8,GPIO_PIN_TYPE_STD);        //push pull output
    GPIO_setPinConfig(GPIO_8_EPWM5A);
    //set up EPWM5 (DAQ trigger clock)
    /*
     * setting the time base of EPWM5 to 240 will result in a EPWM5 period corresponds to one base frame
     * we set it to 120 so that the DAQ card is always triggered after two base_frames have passed
     */
    init_trigger_epwm(EPWM5_BASE);
    //synchronize ePWM11and EPWM5 to ePWM12
    EPWM_setSyncInPulseSource(EPWM11_BASE,EPWM_SYNC_IN_PULSE_SRC_SYNCOUT_EPWM12);
    EPWM_setSyncInPulseSource(EPWM5_BASE,EPWM_SYNC_IN_PULSE_SRC_SYNCOUT_EPWM12);
    //enable phase shift load for channel <channel_to_sync>
    EPWM_enablePhaseShiftLoad(EPWM11_BASE);
    EPWM_enablePhaseShiftLoad(EPWM5_BASE);
    //set phase shift register to zero
    EPWM_setPhaseShift(EPWM11_BASE, 0);
    EPWM_setPhaseShift(EPWM5_BASE, 0);
    //enable sync output of EPWM12
    EPWM_enableSyncOutPulseSource(EPWM12_BASE,EPWM_SYNC_OUT_PULSE_ON_SOFTWARE);
    EPWM_forceSyncPulse(EPWM12_BASE);

    //
    // Enable Half Bridges
    //
    //set_enabled(&cha_buck,true,true);
    set_enabled(&cha_bridge,false,true);
    set_enabled(&chb_buck,true,true);
    set_enabled(&chb_bridge,false,true);
    set_enabled(&chc_buck,true,true);
    set_enabled(&chc_bridge,false,true);
    set_enabled(&chd_buck,true,true);
    set_enabled(&chd_bridge,false,true);
    set_enabled(&che_buck,true,true);
    set_enabled(&che_bridge,false,true);
    set_enabled(&chf_buck,true,true);
    set_enabled(&chf_bridge,false,true);

    //
    // Initialize Reed Switch Interface
    //
    unsigned int n=0;
    for(n=0; n<NO_CHANNELS; n++){
        GPIO_setDirectionMode(driver_channels[n]->enable_resonant_gpio, GPIO_DIR_MODE_OUT);   //output
        GPIO_setPadConfig(driver_channels[n]->enable_resonant_gpio,GPIO_PIN_TYPE_STD);        //push pull output
    }

    //
    // Setup main control task interrupt
    //
    // Initializes PIE and clears PIE registers. Disables CPU interrupts.
    Interrupt_initModule();
    // Initializes the PIE vector table with pointers to the shell Interrupt Service Routines (ISRs)
    Interrupt_initVectorTable();
    //--------------- IPC interrupt ---------------
    //clear any IPC flags
    IPC_clearFlagLtoR(IPC_CPU1_L_CM_R, IPC_FLAG_ALL);
    //register IPC interrupt from CM to CPU1 using IPC_INT0
    IPC_registerInterrupt(IPC_CPU1_L_CM_R, IPC_INT0, IPC_ISR0);
    //synchronize CM and CPU1 using IPC_FLAG31
    IPC_sync(IPC_CPU1_L_CM_R, IPC_FLAG31);
    //--------------- CPU1 Timer0 interrupt (main task) ---------------
    // Register ISR for cupTimer0
    Interrupt_register(INT_TIMER0, &cpuTimer0ISR);
    // Initialize CPUTimer0
    configCPUTimer(CPUTIMER0_BASE, 1e6*deltaT);
    // Enable CPUTimer0 Interrupt within CPUTimer0 Module
    CPUTimer_enableInterrupt(CPUTIMER0_BASE);
    // Enable TIMER0 Interrupt on CPU coming from TIMER0
    Interrupt_enable(INT_TIMER0);
    // Start CPUTimer0
    CPUTimer_startTimer(CPUTIMER0_BASE);
    //--------------- CPU1 Timer1 interrupt (communication active) ---------------
    // Register ISR for cupTimer1
    Interrupt_register(INT_TIMER1, &cpuTimer1ISR);
    // Initialize CPUTimer1
    configCPUTimer(CPUTIMER1_BASE, 500000);
    // Enable CPUTimer0 Interrupt within CPUTimer1 Module
    CPUTimer_enableInterrupt(CPUTIMER1_BASE);
    // Enable TIMER1 Interrupt on CPU coming from TIMER1
    Interrupt_enable(INT_TIMER1);
    // Start CPUTimer0
    CPUTimer_startTimer(CPUTIMER1_BASE);

    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    EINT;
    ERTM;

#ifdef ETHERNET
    //
    // Set up EnetCLK to use SYSPLL as the clock source and set the
    // clock divider to 2.
    //
    // This way we ensure that the PTP clock is 100 MHz. Note that this value
    // is not automatically/dynamically known to the CM core and hence it needs
    // to be made available to the CM side code beforehand.
    SysCtl_setEnetClk(SYSCTL_ENETCLKOUT_DIV_2, SYSCTL_SOURCE_SYSPLL);

    //
    // Configure the GPIOs for ETHERNET.
    //

    //
    // MDIO Signals
    //
    GPIO_setPinConfig(GPIO_105_ENET_MDIO_CLK);
    GPIO_setPinConfig(GPIO_106_ENET_MDIO_DATA);

    //
    // Use this only for RMII Mode
    //GPIO_setPinConfig(GPIO_73_ENET_RMII_CLK);
    //

    //
    //MII Signals
    //
    GPIO_setPinConfig(GPIO_109_ENET_MII_CRS);
    GPIO_setPinConfig(GPIO_110_ENET_MII_COL);

    GPIO_setPinConfig(GPIO_75_ENET_MII_TX_DATA0);
    GPIO_setPinConfig(GPIO_122_ENET_MII_TX_DATA1);
    GPIO_setPinConfig(GPIO_123_ENET_MII_TX_DATA2);
    GPIO_setPinConfig(GPIO_124_ENET_MII_TX_DATA3);

    //
    //Use this only if the TX Error pin has to be connected
    //GPIO_setPinConfig(GPIO_46_ENET_MII_TX_ERR);
    //

    GPIO_setPinConfig(GPIO_118_ENET_MII_TX_EN);

    GPIO_setPinConfig(GPIO_114_ENET_MII_RX_DATA0);
    GPIO_setPinConfig(GPIO_115_ENET_MII_RX_DATA1);
    GPIO_setPinConfig(GPIO_116_ENET_MII_RX_DATA2);
    GPIO_setPinConfig(GPIO_117_ENET_MII_RX_DATA3);
    GPIO_setPinConfig(GPIO_113_ENET_MII_RX_ERR);
    GPIO_setPinConfig(GPIO_112_ENET_MII_RX_DV);

    GPIO_setPinConfig(GPIO_44_ENET_MII_TX_CLK);
    GPIO_setPinConfig(GPIO_111_ENET_MII_RX_CLK);

    //
    //Power down pin to bring the external PHY out of Power down
    //
    GPIO_setDirectionMode(108, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(108, GPIO_PIN_TYPE_PULLUP);
    GPIO_writePin(108,1);

    //
    //PHY Reset Pin to be driven High to bring external PHY out of Reset
    //

    GPIO_setDirectionMode(119, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(119, GPIO_PIN_TYPE_PULLUP);
    GPIO_writePin(119,1);
#endif

#ifdef MCAN
    //
    // Setting the MCAN Clock.
    //
    SysCtl_setMCANClk(SYSCTL_MCANCLK_DIV_4);

    //
    // Configuring the GPIOs for MCAN.
    //
    GPIO_setPinConfig(DEVICE_GPIO_CFG_MCANRXA);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_MCANTXA);
#endif

#ifdef CANA
    //
    // Configuring the GPIOs for CAN A.
    //
    GPIO_setPinConfig(DEVICE_GPIO_CFG_CANRXA);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_CANTXA);

    //
    // Allocate Shared Peripheral CAN A to the CM Side.
    //
    SysCtl_allocateSharedPeripheral(SYSCTL_PALLOCATE_CAN_A,0x1U);
#endif

#ifdef CANB
    //
    // Configuring the GPIOs for CAN B.
    //
    GPIO_setPinConfig(DEVICE_GPIO_CFG_CANRXB);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_CANTXB);

    //
    // Allocate Shared Peripheral CAN B to the CM Side.
    //
    SysCtl_allocateSharedPeripheral(SYSCTL_PALLOCATE_CAN_B,0x1U);
#endif

#ifdef UART
    //
    // Configure GPIO85 as the UART Rx pin.
    //
    GPIO_setPinConfig(GPIO_85_UARTA_RX);
    GPIO_setDirectionMode(85, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(85, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(85, GPIO_QUAL_ASYNC);

    //
    // Configure GPIO84 as the UART Tx pin.
    //
    GPIO_setPinConfig(GPIO_84_UARTA_TX);
    GPIO_setDirectionMode(84, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(84, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(84, GPIO_QUAL_ASYNC);
#endif

#ifdef USB
#ifdef USE_20MHZ_XTAL
    //
    // Set up the auxiliary PLL so a 60 MHz output clock is provided to the USB module.
    // This fixed frequency is required for all USB operations.
    //
    SysCtl_setAuxClock(SYSCTL_AUXPLL_OSCSRC_XTAL |
                       SYSCTL_AUXPLL_IMULT(48) |
                       SYSCTL_REFDIV(2U) | SYSCTL_ODIV(4U) |
                       SYSCTL_AUXPLL_DIV_2 |
                       SYSCTL_AUXPLL_ENABLE |
                       SYSCTL_DCC_BASE_0);
#else
    //
    // Set up the auxiliary PLL so a 60 MHz output clock is provided to the USB module.
    // This fixed frequency is required for all USB operations.
    //
    SysCtl_setAuxClock(SYSCTL_AUXPLL_OSCSRC_XTAL |
                       SYSCTL_AUXPLL_IMULT(48) |
                       SYSCTL_REFDIV(2U) | SYSCTL_ODIV(5U) |
                       SYSCTL_AUXPLL_DIV_2 |
                       SYSCTL_AUXPLL_ENABLE |
                       SYSCTL_DCC_BASE_0);
#endif

    //
    // Allocate Shared Peripheral USB to the CM Side.
    //
    SysCtl_allocateSharedPeripheral(SYSCTL_PALLOCATE_USBA, 1);

    GPIO_setPinConfig(GPIO_0_GPIO0);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setMasterCore(0, GPIO_CORE_CM);

    //
    // Set the master core of GPIOs to CM.
    //
    GPIO_setMasterCore(42, GPIO_CORE_CM);
    GPIO_setMasterCore(43, GPIO_CORE_CM);
    GPIO_setMasterCore(46, GPIO_CORE_CM);
    GPIO_setMasterCore(47, GPIO_CORE_CM);
    GPIO_setMasterCore(120, GPIO_CORE_CM);
    GPIO_setMasterCore(121, GPIO_CORE_CM);

    //
    // Set the USB DM and DP GPIOs.
    //
    GPIO_setAnalogMode(42, GPIO_ANALOG_ENABLED);
    GPIO_setAnalogMode(43, GPIO_ANALOG_ENABLED);

    //
    // Set the direction for VBUS and ID.
    //
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_IN);
    GPIO_setDirectionMode(47, GPIO_DIR_MODE_IN);

    //
    // Configure the Power Fault.
    //
    GPIO_setMasterCore(120, GPIO_CORE_CM);
    GPIO_setDirectionMode(120, GPIO_DIR_MODE_IN);

    //
    // Configure the External Power Signal Enable.
    //
    GPIO_setMasterCore(121, GPIO_CORE_CM);
	GPIO_setDirectionMode(121, GPIO_DIR_MODE_OUT);
	GPIO_writePin(121, 1);

    //
    // Set the CM Clock to run at 120MHz.
    // The CM Clock is a fractional multiple of the AUXPLL Clock (120 Mhz) from
    // which the USB Clock (60 MHz) is derived.
    //
    SysCtl_setCMClk(SYSCTL_CMCLKOUT_DIV_1, SYSCTL_SOURCE_AUXPLL);
#endif

    //
    // Initialize Outputs
    //
    for(n=0; n<NO_CHANNELS; n++){
        READY_enter(n);
        driver_channels[n]->channel_state=READY;
    }

    uint32_t loop_counter=0;
    // Main Loop
    while(1){
        if(run_main_task){
            //toggle heartbeat gpio
            GPIO_writePin(HEARTBEAT_GPIO,0);


            //set start of frame (SOF) output
            if(loop_counter<(int)(SAMPLE_PERIOD_S/deltaT))
                GPIO_writePin(SOF_GPIO,1);
            else
                GPIO_writePin(SOF_GPIO,0);

            //---------------------
            // State Machine
            //---------------------
            //Main Relay Opening Logic
            unsigned int channel_counter=0;
            bool main_relay_active=false;
            for(channel_counter=0; channel_counter<NO_CHANNELS; channel_counter++){
                run_channel_fsm(driver_channels[channel_counter]);
                //we enable the main relay when one channel is not in state READY anymore (e.g. when one channel requires power)
                if(driver_channels[channel_counter]->channel_state!=READY)
                    main_relay_active=true;
            }
            GPIO_writePin(MAIN_RELAY_GPIO,main_relay_active);
            GPIO_writePin(SLAVE_RELAY_GPIO,main_relay_active);
            GPIO_writePin(LED_1_GPIO,!main_relay_active);
            //Communication Active Logic (If no communication, issue a STOP command
            if(!communication_active){
                for(channel_counter=0; channel_counter<NO_CHANNELS; channel_counter++){
                    fsm_req_flags_stop[channel_counter]=1;
                }
            }



            //---------------------
            // Signal Acquisition & Filtering
            //---------------------

            // Read ADCs sequentially, this updates the system_dyn_state structure
            readAnalogInputs();
            // TODO: Filter the acquired analog signals in system_dyn_state_filtered
            // ---
            // TODO: Filter the input reference signals
            unsigned int i=0;
//            for(i=0; i<NO_CHANNELS; i++){
//                //update_first_order(des_duty_buck_filt+i,des_duty_buck[i]); //don't need this without buck stage
//                update_second_order_system(des_current_filt+i,des_currents[i]);
//            }


            //---------------------
            // Control Law Execution
            //---------------------
            //compute optional reference waveform
            //#define OMEGA 2*3.14159265358979323846*5
            //float ides=sin(OMEGA*loop_counter*deltaT);
//            const unsigned int periodn=500e-3/deltaT;
//            float ides=0.0;
//            if(loop_counter%periodn<periodn/2)
//                ides=1.0;
//            else
//                ides=-1.0;
            //regulate outputs of channels
            // ...


            //---------------------
            // Control Law Execution & Output Actuation
            //---------------------
            //set output duties for buck
            for(i=0; i<NO_CHANNELS; i++){
                set_duty_buck(driver_channels[i]->buck_config,(des_duty_buck_filt+i)->y);
            }
            //set output duties for bridge [regular mode]
            for(i=0; i<NO_CHANNELS; i++){
                if(driver_channels[i]->channel_state==RUN_REGULAR){
                    #ifdef TUNE_CLOSED_LOOP
                        if(enable_waveform_debugging)
                            des_currents[i]=ides;
                        else
                            ides=0.0;
                    #endif
                    //execute the PI control low
                    float voltage_dclink=VIN;
                    //compute feed forward actuation term (limits [-1,1] for this duty) - feed-forward term currently not used
                    #ifdef FEED_FORWARD_ONLY
                        float act_voltage_ff=des_currents[i]*RDC;
                        float act_voltage_fb=0.0;
                    #endif
                    #ifdef CLOSED_LOOP
                        float act_voltage_ff=0.0;
                        //compute feedback actuation term (limits [-1,1] for this duty)
                        bool output_saturated=fabsf((current_pi+i)->u)>=0.95*voltage_dclink;
//                        if(i==0)
//                            current_log[loop_counter%1024]=des_current;
//                        float des_current=0;
//                        if((loop_counter%5000)<2500)
//                            des_current=1.0;
//                        else
//                            des_current=0;

                        float act_voltage_fb=update_pid(current_pi+i,des_currents[i],system_dyn_state.is[i],output_saturated);
                    #endif
                    float duty_ff=act_voltage_ff/voltage_dclink;
                    float duty_fb=act_voltage_fb/(voltage_dclink);
                    // TODO-PID : Sanity / Limit Checks on PID go here

                    //convert normalized duty cycle, limit it and apply
                    float duty_bridge=0.5*(1+(duty_ff+duty_fb));
                    if(duty_bridge>0.9)
                        duty_bridge=0.9;
                    if(duty_bridge<0.1)
                        duty_bridge=0.1;
                    set_duty_bridge(driver_channels[i]->bridge_config,duty_bridge,i);
                }
                //set_duty_bridge(driver_channels[i]->bridge_config,des_duty_bridge[i]);
            }
            //set frequency for bridge [resonant mode] do not do this in this branch
//            for(i=0; i<NO_CHANNELS; i++){
//                if(driver_channels[i]->channel_state==RUN_RESONANT)
//                    set_freq_bridge(driver_channels[i]->bridge_config,des_freq_resonant_mhz[i]);
//            }

            //---------------------
            // Read State Of Bridges
            //---------------------
            uint32_t cha_buck_state=GPIO_readPin(cha_buck.state_gpio);
            uint32_t cha_bridge_state_u=GPIO_readPin(cha_bridge.state_v_gpio);
            uint32_t cha_bridge_state_v=GPIO_readPin(cha_bridge.state_u_gpio);

            uint32_t chb_buck_state=GPIO_readPin(chb_buck.state_gpio);
            uint32_t chb_bridge_state_u=GPIO_readPin(chb_bridge.state_v_gpio);
            uint32_t chb_bridge_state_v=GPIO_readPin(chb_bridge.state_u_gpio);

            uint32_t chc_buck_state=GPIO_readPin(chc_buck.state_gpio);
            uint32_t chc_bridge_state_u=GPIO_readPin(chc_bridge.state_v_gpio);
            uint32_t chc_bridge_state_v=GPIO_readPin(chc_bridge.state_u_gpio);

            run_main_task=false;
            //reset the loop counter
            if(loop_counter>=(float)frame_period_ms*1e-3/deltaT)
                loop_counter=0;
            else
                loop_counter+=1;
            GPIO_writePin(HEARTBEAT_GPIO,1);
        }
    }
}


