/*
 * mdriver_adc.c
 *
 *  Created on: Oct 17, 2021
 *      Author: dvarx
 */

#include <mdriver_cpu1.h>
#include <mdriver_hw_defs.h>
#include "driverlib.h"
#include "device.h"
#include "stdbool.h"

//
// Defines
//
#define EX_ADC_RESOLUTION       12
// 12 for 12-bit conversion resolution, which supports single-ended signaling
// Or 16 for 16-bit conversion resolution, which supports single-ended or
// differential signaling
#define EX_ADC_SIGNALMODE       "SINGLE-ENDED"
//"SINGLE-ENDED" for ADC_MODE_SINGLE_ENDED:
// Sample on single pin (VREFLO is the low reference)
// Or "Differential" for ADC_MODE_DIFFERENTIAL:
// Sample on pair of pins (difference between pins is converted, subject to
// common mode voltage requirements; see the device data manual)


//
// Function to configure and power up ADCs A,B,C,D
//
void initADCs(void)
{
    //
    // Set ADCCLK divider to /4
    //
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);
    ADC_setPrescaler(ADCB_BASE, ADC_CLK_DIV_4_0);
    ADC_setPrescaler(ADCC_BASE, ADC_CLK_DIV_4_0);
    ADC_setPrescaler(ADCD_BASE, ADC_CLK_DIV_4_0);

    //
    // Set resolution and signal mode (see #defines above) and load
    // corresponding trims.
    //
#if(EX_ADC_RESOLUTION == 12)
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_setMode(ADCB_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_setMode(ADCC_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_setMode(ADCD_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
#elif(EX_ADC_RESOLUTION == 16)
    #if(EX_ADC_SIGNALMODE == "SINGLE-ENDED")
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_SINGLE_ENDED);
    ADC_setMode(ADCC_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_SINGLE_ENDED);
    #elif(EX_ADC_SIGNALMODE == "DIFFERENTIAL")
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_DIFFERENTIAL);
    ADC_setMode(ADCC_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_DIFFERENTIAL);
    #endif
#endif

    //
    // Set pulse positions to late
    //
    ADC_setInterruptPulseMode(ADCA_BASE, ADC_PULSE_END_OF_CONV);
    ADC_setInterruptPulseMode(ADCB_BASE, ADC_PULSE_END_OF_CONV);
    ADC_setInterruptPulseMode(ADCC_BASE, ADC_PULSE_END_OF_CONV);
    ADC_setInterruptPulseMode(ADCD_BASE, ADC_PULSE_END_OF_CONV);

    //
    // Power up the ADCs and then delay for 1 ms
    //
    ADC_enableConverter(ADCA_BASE);
    ADC_enableConverter(ADCB_BASE);
    ADC_enableConverter(ADCC_BASE);
    ADC_enableConverter(ADCD_BASE);

    DEVICE_DELAY_US(1000);
}

//
// Function to configure SOCs 0 and 1 of ADCs A and C.
//
void initADCSOCs(void)
{
    //----------------------------------------------------------------
    // ADCA Configuration
    //  ADCA measures: [iB(A0),iE(A2)]
    //----------------------------------------------------------------
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM7_SOCA,
                 ADC_CH_ADCIN0, 15);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM7_SOCA,
                 ADC_CH_ADCIN2, 15);

    //----------------------------------------------------------------
    // ADCB Configuration
    //  ADCB measures: [iA(B0),iB(B4)]
    //----------------------------------------------------------------
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM7_SOCA,
                 ADC_CH_ADCIN0, 15);
    ADC_setupSOC(ADCB_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM7_SOCA,
                 ADC_CH_ADCIN4, 15);

    //----------------------------------------------------------------
    // ADCC Configuration
    //  ADCC measures: [iF(C3)]
    //----------------------------------------------------------------
    ADC_setupSOC(ADCC_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM7_SOCA,
                 ADC_CH_ADCIN3, 15);

    //----------------------------------------------------------------
    // ADCD Configuration
    //  ADCD measures: [iC(D2),vC(D3),]
    //----------------------------------------------------------------
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM7_SOCA,
                 ADC_CH_ADCIN2, 15);
    ADC_setupSOC(ADCD_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM7_SOCA,
                 ADC_CH_ADCIN3, 15);

}

/*
 * calibration measurement is of the form m=alpha'*i+beta-0.280000001' => i=alpha*m+beta with alpha=1/alpha' and beta=-beta'/alpha
 *
 * the single ended measurement from the ADC has a resolution of 12bit with a reference of 3.0V
 * 2^12-1 is the max value and corresponds to 3.0V or 3.0/I_SENSITIVITY [A]
 * 2^11 is the mid value and corresponds to 1.5V or 1.5/I_SENSITIVITY [A]
 *
 * in the case of nominal current sensor sensitivity and offset
 * calib_factor_current_alphas[channelno]==(1.5V/I_SENSITIVITY)/2048
 * calib_factor_current_betas[channelno]==0
 * currentval_ampere = (adval-2048)*calib_factor_current_alphas[channelno]+calib_factor_current_betas[channelno]
 *
 * in the case of calibrated current sensor sensitivities and offsets, the values of calib_factor_current_alphas and
 * calib_factor_current_betas may differ from the nominal values (the calibrated values are computed in `mdriver_hw_defs.c`,
 * follow the calibration procedure outlined in `mdriver_hw_defs.c`)
 *
 */

//current sensor sensitivities and offsets are defined in `hardware_defs.c`
inline float conv_adc_meas_to_current_a(const uint16_t adc_output,unsigned int channelno){
    return calib_factor_current_alphas[channelno]*(float)((int16_t)adc_output-(int16_t)2048)+calib_factor_current_betas[channelno];
}

// This function reads the analog inputs and stores them in the system_dyn_state structure
void readAnalogInputs(void){
    // ADC A Measurements -----------------------------------------------
    // Wait for ADCA to complete, then acknowledge flag
    system_dyn_state.is[3] = conv_adc_meas_to_current_a(ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0),3);
    system_dyn_state.is[4] = conv_adc_meas_to_current_a(ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2),4);

    // ADC B Measurements -----------------------------------------------
    // Wait for ADCB to complete, then acknowledge flag
    system_dyn_state.is[0] = conv_adc_meas_to_current_a(ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER0),0);
    system_dyn_state.is[1] = conv_adc_meas_to_current_a(ADC_readResult(ADCBRESULT_BASE, ADC_SOC_NUMBER2),1);

    // ADC C Measurements -----------------------------------------------
    // Wait for ADCC to complete, then acknowledge flag
    system_dyn_state.is[5] = conv_adc_meas_to_current_a(ADC_readResult(ADCCRESULT_BASE, ADC_SOC_NUMBER2),5);

    // ADC D Measurements -----------------------------------------------
    // Wait for ADCD to complete, then acknowledge flag
    system_dyn_state.is[2] = conv_adc_meas_to_current_a(ADC_readResult(ADCDRESULT_BASE, ADC_SOC_NUMBER0),2);

    //copy the values into the state message sent back via TCP
    unsigned int i=0;
    for(i=0; i<NO_CHANNELS; i++)
        ipc_mdriver_state_msg.currents[i]=1000*system_dyn_state.is[i];
}
