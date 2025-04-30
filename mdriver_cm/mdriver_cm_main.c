//###########################################################################
//
// FILE:   mdriver_cm_main.c
// based on
// TITLE:  lwIP based Ethernet Example.
//
//###########################################################################
// $TI Release: $
// $Release Date: $
// $Copyright: $
//###########################################################################

#include <string.h>
#include <stdbool.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_emac.h"

#include "driverlib_cm/ethernet.h"
#include "driverlib_cm/gpio.h"
#include "driverlib_cm/interrupt.h"
#include "driverlib_cm/flash.h"

#include "driverlib_cm/sysctl.h"
#include "driverlib_cm/systick.h"

#include "utils/lwiplib.h"
#include "board_drivers/pinout.h"

#include "lwip/apps/httpd.h"
#include <stdbool.h>
#include <string.h>

#include "comm_interface.h"
//*****************************************************************************
//
//! \addtogroup master_example_list
//! <h1>Ethernet with lwIP (enet_lwip)</h1>
//!
//! This example application demonstrates the operation of the F2838x
//! microcontroller Ethernet controller using the lwIP TCP/IP Stack. Once
//! programmed, the device sits endlessly waiting for ICMP ping requests. It
//! has a static IP address. To ping the device, the sender has to be in the
//! same network. The stack also supports ARP.
//!
//! For additional details on lwIP, refer to the lwIP web page at:
//! http://savannah.nongnu.org/projects/lwip/
//
//*****************************************************************************

// ----------------- IPC Related ---------------
#include "ipc.h"
#include <stdint.h>

#define IPC_CMD_READ_MEM   0x1001
#define IPC_CMD_RESP       0x2001

#define TEST_PASS          0x5555
#define TEST_FAIL          0xAAAA

#pragma DATA_SECTION(readData, "MSGRAM_CM_TO_CPU1")
uint32_t readData[10];
struct mdriver_msg ipc_msg_tnb_mns;
#pragma DATA_SECTION(ipc_msg_tnb_mns_c2000, "MSGRAM_CM_TO_CPU1")
struct mdriver_msg ipc_msg_tnb_mns_c2000;
uint8_t sysstatebuffer[512];


uint32_t pass;
void processCommand(void);
// ---------------------------------------------

// Control specific constants
const u16_t COMM_PORT=30;               //tcp port number used for communication with control card
struct tcp_pcb* welcoming_socket_pcb;
bool connection_active=false;           //indicates whether there is a current tcp connection active
void setupCommInterface(void);
err_t accept_cb(void*,struct tcp_pcb*,err_t);
uint8_t buffer[2048];                    //a buffer storing the last received bytes
bool command_available=false;           //indicates whether a new command has been received via TCP
bool reset_connection=false;
unsigned int tcp_connection_reset_counter=0;
err_t tcp_recvd_cb(void*, struct tcp_pcb*, struct pbuf*,err_t);


// These are defined by the linker (see device linker command file)
extern uint16_t RamfuncsLoadStart;
extern uint16_t RamfuncsLoadSize;
extern uint16_t RamfuncsRunStart;
extern uint16_t RamfuncsLoadEnd;
extern uint16_t RamfuncsRunEnd;
extern uint16_t RamfuncsRunSize;

extern uint16_t constLoadStart;
extern uint16_t constLoadEnd;
extern uint16_t constLoadSize;
extern uint16_t constRunStart;
extern uint16_t constRunEnd;
extern uint16_t constRunSize;

#define DEVICE_FLASH_WAITSTATES 2

//*****************************************************************************
//
// Driver specific initialization code and macro.
//
//*****************************************************************************

#define ETHERNET_NO_OF_RX_PACKETS   2U
#define ETHERNET_MAX_PACKET_LENGTH 1538U
#define NUM_PACKET_DESC_RX_APPLICATION 8

Ethernet_Handle emac_handle;
Ethernet_InitConfig *pInitCfg;
uint32_t Ethernet_numRxCallbackCustom = 0;
uint32_t releaseTxCount = 0;
uint32_t genericISRCustomcount = 0;
uint32_t genericISRCustomRBUcount = 0;
uint32_t genericISRCustomROVcount = 0;
uint32_t genericISRCustomRIcount = 0;

uint32_t systickPeriodValue = 1000000;
Ethernet_Pkt_Desc  pktDescriptorRXCustom[NUM_PACKET_DESC_RX_APPLICATION];
extern uint32_t Ethernet_numGetPacketBufferCallback;
extern Ethernet_Device Ethernet_device_struct;
uint8_t Ethernet_rxBuffer[ETHERNET_NO_OF_RX_PACKETS *
                          ETHERNET_MAX_PACKET_LENGTH];

extern Ethernet_Pkt_Desc*
lwIPEthernetIntHandler(Ethernet_Pkt_Desc *pPacket);

void CM_init(void)
{
    //
    // Disable the watchdog
    //
    SysCtl_disableWatchdog();

#ifdef _FLASH
    //
    // Copy time critical code and flash setup code to RAM. This includes the
    // following functions: InitFlash();
    //
    // The RamfuncsLoadStart, RamfuncsLoadSize, and RamfuncsRunStart symbols
    // are created by the linker. Refer to the device .cmd file.
    // Html pages are also being copied from flash to ram.
    //
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
    memcpy(&constRunStart, &constLoadStart, (size_t)&constLoadSize);
    //
    // Call Flash Initialization to setup flash waitstates. This function must
    // reside in RAM.
    //
    Flash_initModule(FLASH0CTRL_BASE, FLASH0ECC_BASE, DEVICE_FLASH_WAITSTATES);
#endif

    //
    // Sets the NVIC vector table offset address.
    //
#ifdef _FLASH
    Interrupt_setVectorTableOffset((uint32_t)vectorTableFlash);
#else
    Interrupt_setVectorTableOffset((uint32_t)vectorTableRAM);
#endif

}
//*****************************************************************************
//
// HTTP Webserver related callbacks and definitions.
//
//*****************************************************************************
//
// Currently, this implemented as a pointer to function which is called when
// corresponding query is received by the HTTP webserver daemon. When more
// features are needed to be added, it should be implemented as a separate
// interface.
//
void httpLEDToggle(void);
void(*ledtoggleFuncPtr)(void) = &httpLEDToggle;

//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Call the lwIP timer handler.
    //
    lwIPTimer(systickPeriodValue);
}

//*****************************************************************************
//
//  This function is a callback function called by the example to
//  get a Packet Buffer. Has to return a ETHERNET_Pkt_Desc Structure.
//  Rewrite this API for custom use case.
//
//*****************************************************************************
Ethernet_Pkt_Desc* Ethernet_getPacketBufferCustom(void)
{
    //
    // Get the next packet descriptor from the descriptor pool
    //
    uint32_t shortIndex = (Ethernet_numGetPacketBufferCallback + 3)
                % NUM_PACKET_DESC_RX_APPLICATION;

    //
    // Increment the book-keeping pointer which acts as a head pointer
    // to the circular array of packet descriptor pool.
    //
    Ethernet_numGetPacketBufferCallback++;

    //
    // Update buffer length information to the newly procured packet
    // descriptor.
    //
    pktDescriptorRXCustom[shortIndex].bufferLength =
                                  ETHERNET_MAX_PACKET_LENGTH;

    //
    // Update the receive buffer address in the packer descriptor.
    //
    pktDescriptorRXCustom[shortIndex].dataBuffer =
                                      &Ethernet_device_struct.rxBuffer [ \
               (ETHERNET_MAX_PACKET_LENGTH*Ethernet_device_struct.rxBuffIndex)];

    //
    // Update the receive buffer pool index.
    //
    Ethernet_device_struct.rxBuffIndex += 1U;
    Ethernet_device_struct.rxBuffIndex  = \
    (Ethernet_device_struct.rxBuffIndex%ETHERNET_NO_OF_RX_PACKETS);

    //
    // Receive buffer is usable from Address 0
    //
    pktDescriptorRXCustom[shortIndex].dataOffset = 0U;

    //
    // Return this new descriptor to the driver.
    //
    return (&(pktDescriptorRXCustom[shortIndex]));
}

//*****************************************************************************
//
//  This is a hook function and called by the driver when it receives a
//  packet. Application is expected to replenish the buffer after consuming it.
//  Has to return a ETHERNET_Pkt_Desc Structure.
//  Rewrite this API for custom use case.
//
//*****************************************************************************
Ethernet_Pkt_Desc* Ethernet_receivePacketCallbackCustom(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    //
    // Book-keeping to maintain number of callbacks received.
    //
#ifdef ETHERNET_DEBUG
    Ethernet_numRxCallbackCustom++;
#endif

    //
    // This is a placeholder for Application specific handling
    // We are replenishing the buffer received with another buffer
    //
    return lwIPEthernetIntHandler(pPacket);
}

void Ethernet_releaseTxPacketBufferCustom(
        Ethernet_Handle handleApplication,
        Ethernet_Pkt_Desc *pPacket)
{
    //
    // Once the packet is sent, reuse the packet memory to avoid
    // memory leaks. Call this interrupt handler function which will take care
    // of freeing the memory used by the packet descriptor.
    //
    lwIPEthernetIntHandler(pPacket);

    //
    // Increment the book-keeping counter.
    //
#ifdef ETHERNET_DEBUG
    releaseTxCount++;
#endif
}

interrupt void Ethernet_genericISRCustom(void)
{
    genericISRCustomcount++;
    Ethernet_RxChDesc *rxChan;
    Ethernet_TxChDesc *txChan;
    Ethernet_HW_descriptor    *descPtr;
    Ethernet_HW_descriptor    *tailPtr;
    if((ETHERNET_DMA_CH0_STATUS_AIS |
                       ETHERNET_DMA_CH0_STATUS_RBU) ==
                     (HWREG(Ethernet_device_struct.baseAddresses.enet_base +
                            ETHERNET_O_DMA_CH0_STATUS) &
                            (uint32_t)(ETHERNET_DMA_CH0_STATUS_AIS |
                                       ETHERNET_DMA_CH0_STATUS_RBU)))
    {
        genericISRCustomRBUcount++;
        /*
             * Clear the AIS and RBU status bit. These MUST be
             * cleared together!
             */
            Ethernet_clearDMAChannelInterrupt(
                    Ethernet_device_struct.baseAddresses.enet_base,
                    ETHERNET_DMA_CHANNEL_NUM_0,
                    ETHERNET_DMA_CH0_STATUS_AIS |
                    ETHERNET_DMA_CH0_STATUS_RBU);

            /*
           *Recover from Receive Buffer Unavailable (and hung DMA)
         *
         * All descriptor buffers are owned by the application, and
         * in result the DMA cannot transfer incoming frames to the
         * buffers (RBU condition). DMA has also entered suspend
         * mode at this point, too.
         *
         * Drain the RX queues
         */

            /* Upon RBU error, discard all previously received packets */
            if(Ethernet_device_struct.initConfig.pfcbDeletePackets != NULL)
                (*Ethernet_device_struct.initConfig.pfcbDeletePackets)();

            rxChan =
               &Ethernet_device_struct.dmaObj.rxDma[ETHERNET_DMA_CHANNEL_NUM_0];

    /*
     * Need to disable multiple interrupts, so protect the code to do so within
     * a global disable block (to prevent getting interrupted in between)
     */

            if(NULL!= Ethernet_device_struct.ptrPlatformInterruptDisable)
            {
                (*Ethernet_device_struct.ptrPlatformInterruptDisable)(
                    Ethernet_device_struct.interruptNum[
                        ETHERNET_RX_INTR_CH0 + rxChan->chInfo->chNum]);

                (*Ethernet_device_struct.ptrPlatformInterruptDisable)(
                    Ethernet_device_struct.interruptNum[
                        ETHERNET_GENERIC_INTERRUPT]);
            }

            /* verify we have full capacity in the descriptor queue */
            if(rxChan->descQueue.count < rxChan->descMax) {
              /* The queue is not at full capacity due to OOM errors.
              Try to fill it again */
                Ethernet_addPacketsIntoRxQueue(rxChan);
            }

    /*
     * Need to re-enable multiple interrupts. Again, protect the code to do so
     * within a global disable block (to prevent getting interrupted in between)
     */

            if(NULL!= Ethernet_device_struct.ptrPlatformInterruptEnable)
            {
                (*Ethernet_device_struct.ptrPlatformInterruptEnable)(
                    Ethernet_device_struct.interruptNum[
                        ETHERNET_RX_INTR_CH0 + rxChan->chInfo->chNum]);
                (*Ethernet_device_struct.ptrPlatformInterruptEnable)(
                    Ethernet_device_struct.interruptNum[
                        ETHERNET_GENERIC_INTERRUPT]);
            }
            Ethernet_removePacketsFromRxQueue(rxChan,
                    ETHERNET_COMPLETION_NORMAL);

            /* To un-suspend the DMA:
             *
             * 1. Change ownership of current RX descriptor to DMA
             *
             * 2. Issue a receive poll demand command
             *
             * 3. Issue a write to the descriptor tail pointer register
             */

            /* 1. Change current descriptor owernship back to DMA */
            descPtr = (Ethernet_HW_descriptor *)(HWREG(
                    Ethernet_device_struct.baseAddresses.enet_base +
                    (uint32_t)ETHERNET_O_DMA_CH0_CURRENT_APP_RXDESC));

            descPtr->des3 = ETHERNET_DESC_OWNER | ETHERNET_RX_DESC_IOC |
                              ETHERNET_RX_DESC_BUF1_VALID;

            /* 2. Issue a receive poll demand command */

            /* 3. Issue a write to the descriptor tail pointer register */
            tailPtr = (Ethernet_HW_descriptor *)(HWREG(
                        Ethernet_device_struct.baseAddresses.enet_base +
                        (uint32_t)ETHERNET_O_DMA_CH0_RXDESC_TAIL_POINTER));

            Ethernet_writeRxDescTailPointer(
                    Ethernet_device_struct.baseAddresses.enet_base,
                    ETHERNET_DMA_CHANNEL_NUM_0,
                    tailPtr);


    }
    if((ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOVFIS) ==
                         (HWREG(Ethernet_device_struct.baseAddresses.enet_base +
                                ETHERNET_O_MTL_Q0_INTERRUPT_CONTROL_STATUS) &
                                (uint32_t)(ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOVFIS
                                           )))
    {
        genericISRCustomROVcount++;

        //Acknowledge the MTL RX Queue overflow status
        Ethernet_enableMTLInterrupt(Ethernet_device_struct.baseAddresses.enet_base,0,
                                    ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOVFIS);
        /*
                 *Recover from Receive Buffer Unavailable (and hung DMA)
               *
               * All descriptor buffers are owned by the application, and
               * in result the DMA cannot transfer incoming frames to the
               * buffers (RBU condition). DMA has also entered suspend
               * mode at this point, too.
               *
               * Drain the RX queues
               */

                  /* Upon RBU error, discard all previously received packets */
                  if(Ethernet_device_struct.initConfig.pfcbDeletePackets != NULL)
                      (*Ethernet_device_struct.initConfig.pfcbDeletePackets)();

                  rxChan =
                     &Ethernet_device_struct.dmaObj.rxDma[ETHERNET_DMA_CHANNEL_NUM_0];

          /*
           * Need to disable multiple interrupts, so protect the code to do so within
           * a global disable block (to prevent getting interrupted in between)
           */

                  if(NULL!= Ethernet_device_struct.ptrPlatformInterruptDisable)
                  {
                      (*Ethernet_device_struct.ptrPlatformInterruptDisable)(
                          Ethernet_device_struct.interruptNum[
                              ETHERNET_RX_INTR_CH0 + rxChan->chInfo->chNum]);

                      (*Ethernet_device_struct.ptrPlatformInterruptDisable)(
                          Ethernet_device_struct.interruptNum[
                              ETHERNET_GENERIC_INTERRUPT]);
                  }

                  /* verify we have full capacity in the descriptor queue */
                  if(rxChan->descQueue.count < rxChan->descMax) {
                    /* The queue is not at full capacity due to OOM errors.
                    Try to fill it again */
                      Ethernet_addPacketsIntoRxQueue(rxChan);
                  }

          /*
           * Need to re-enable multiple interrupts. Again, protect the code to do so
           * within a global disable block (to prevent getting interrupted in between)
           */

                  if(NULL!= Ethernet_device_struct.ptrPlatformInterruptEnable)
                  {
                      (*Ethernet_device_struct.ptrPlatformInterruptEnable)(
                          Ethernet_device_struct.interruptNum[
                              ETHERNET_RX_INTR_CH0 + rxChan->chInfo->chNum]);
                      (*Ethernet_device_struct.ptrPlatformInterruptEnable)(
                          Ethernet_device_struct.interruptNum[
                              ETHERNET_GENERIC_INTERRUPT]);
                  }
                  Ethernet_removePacketsFromRxQueue(rxChan,
                          ETHERNET_COMPLETION_NORMAL);

                  /* To un-suspend the DMA:
                   *
                   * 1. Change ownership of current RX descriptor to DMA
                   *
                   * 2. Issue a receive poll demand command
                   *
                   * 3. Issue a write to the descriptor tail pointer register
                   */

                  /* 1. Change current descriptor owernship back to DMA */
                  descPtr = (Ethernet_HW_descriptor *)(HWREG(
                          Ethernet_device_struct.baseAddresses.enet_base +
                          (uint32_t)ETHERNET_O_DMA_CH0_CURRENT_APP_RXDESC));

                  descPtr->des3 = ETHERNET_DESC_OWNER | ETHERNET_RX_DESC_IOC |
                                    ETHERNET_RX_DESC_BUF1_VALID;

                  /* 2. Issue a receive poll demand command */

                  /* 3. Issue a write to the descriptor tail pointer register */
                  tailPtr = (Ethernet_HW_descriptor *)(HWREG(
                              Ethernet_device_struct.baseAddresses.enet_base +
                              (uint32_t)ETHERNET_O_DMA_CH0_RXDESC_TAIL_POINTER));

                  Ethernet_writeRxDescTailPointer(
                          Ethernet_device_struct.baseAddresses.enet_base,
                          ETHERNET_DMA_CHANNEL_NUM_0,
                          tailPtr);
    }
    if(0U != (HWREG(Ethernet_device_struct.baseAddresses.enet_base +
                                 ETHERNET_O_DMA_CH0_STATUS) &
                           (uint32_t) ETHERNET_DMA_CH0_STATUS_RI))
    {
        genericISRCustomRIcount++;
        Ethernet_clearDMAChannelInterrupt(
                        Ethernet_device_struct.baseAddresses.enet_base,
                        ETHERNET_DMA_CHANNEL_NUM_0,
                        ETHERNET_DMA_CH0_STATUS_NIS | ETHERNET_DMA_CH0_STATUS_RI);
    }
}

void
Ethernet_init(const unsigned char *mac)
{
    Ethernet_InitInterfaceConfig initInterfaceConfig;
    uint32_t macLower;
    uint32_t macHigher;
    uint8_t *temp;

    initInterfaceConfig.ssbase = EMAC_SS_BASE;
    initInterfaceConfig.enet_base = EMAC_BASE;
    initInterfaceConfig.phyMode = ETHERNET_SS_PHY_INTF_SEL_MII;

    //
    // Assign SoC specific functions for Enabling,Disabling interrupts
    // and for enabling the Peripheral at system level
    //
    initInterfaceConfig.ptrPlatformInterruptDisable =
                                                    &Platform_disableInterrupt;
    initInterfaceConfig.ptrPlatformInterruptEnable =
                                                     &Platform_enableInterrupt;
    initInterfaceConfig.ptrPlatformPeripheralEnable =
                                                    &Platform_enablePeripheral;
    initInterfaceConfig.ptrPlatformPeripheralReset =
                                                     &Platform_resetPeripheral;

    //
    // Assign the peripheral number at the SoC
    //
    initInterfaceConfig.peripheralNum = SYSCTL_PERIPH_CLK_ENET;

    //
    // Assign the default SoC specific interrupt numbers of Ethernet interrupts
    //
    initInterfaceConfig.interruptNum[0] = INT_EMAC;
    initInterfaceConfig.interruptNum[1] = INT_EMAC_TX0;
    initInterfaceConfig.interruptNum[2] = INT_EMAC_TX1;
    initInterfaceConfig.interruptNum[3] = INT_EMAC_RX0;
    initInterfaceConfig.interruptNum[4] = INT_EMAC_RX1;

    pInitCfg = Ethernet_initInterface(initInterfaceConfig);

    Ethernet_getInitConfig(pInitCfg);
    pInitCfg->dmaMode.InterruptMode = ETHERNET_DMA_MODE_INTM_MODE2;

    //
    // Assign the callbacks for Getting packet buffer when needed
    // Releasing the TxPacketBuffer on Transmit interrupt callbacks
    // Receive packet callback on Receive packet completion interrupt
    //
    pInitCfg->pfcbRxPacket = &Ethernet_receivePacketCallbackCustom;
    pInitCfg->pfcbGetPacket = &Ethernet_getPacketBufferCustom;
    pInitCfg->pfcbFreePacket = &Ethernet_releaseTxPacketBufferCustom;

    //
    //Assign the Buffer to be used by the Low level driver for receiving
    //Packets. This should be accessible by the Ethernet DMA
    //
    pInitCfg->rxBuffer = Ethernet_rxBuffer;

    //
    // The Application handle is not used by this application
    // Hence using a dummy value of 1
    //
    Ethernet_getHandle((Ethernet_Handle)1, pInitCfg , &emac_handle);

    //
    // Disable transmit buffer unavailable and normal interrupt which
    // are enabled by default in Ethernet_getHandle.
    //
    Ethernet_disableDmaInterrupt(Ethernet_device_struct.baseAddresses.enet_base,
                                 0, (ETHERNET_DMA_CH0_INTERRUPT_ENABLE_TBUE |
                                     ETHERNET_DMA_CH0_INTERRUPT_ENABLE_NIE));

    //
    // Enable the MTL interrupt to service the receive FIFO overflow
    // condition in the Ethernet module.
    //
    Ethernet_enableMTLInterrupt(Ethernet_device_struct.baseAddresses.enet_base,0,
                                ETHERNET_MTL_Q0_INTERRUPT_CONTROL_STATUS_RXOIE);

    //
    // Disable the MAC Management counter interrupts as they are not used
    // in this application.
    //
    HWREG(Ethernet_device_struct.baseAddresses.enet_base + ETHERNET_O_MMC_RX_INTERRUPT_MASK) = 0xFFFFFFFF;
    HWREG(Ethernet_device_struct.baseAddresses.enet_base + ETHERNET_O_MMC_IPC_RX_INTERRUPT_MASK) = 0xFFFFFFFF;
    //
    //Do global Interrupt Enable
    //
    (void)Interrupt_enableInProcessor();

    //
    //Assign default ISRs
    //
    Interrupt_registerHandler(INT_EMAC_TX0, Ethernet_transmitISR);
    Interrupt_registerHandler(INT_EMAC_RX0, Ethernet_receiveISR);
    Interrupt_registerHandler(INT_EMAC, Ethernet_genericISRCustom);

    //
    //Enable the default interrupt handlers
    //
    Interrupt_enable(INT_EMAC_TX0);
    Interrupt_enable(INT_EMAC_RX0);
    Interrupt_enable(INT_EMAC);

    //
    // Convert the mac address string into the 32/16 split variables format
    // that is required by the driver to program into hardware registers.
    // Note: This step is done after the Ethernet_getHandle function because
    //       a dummy MAC address is programmed in that function.
    //
    temp = (uint8_t *)&macLower;
    temp[0] = mac[0];
    temp[1] = mac[1];
    temp[2] = mac[2];
    temp[3] = mac[3];

    temp = (uint8_t *)&macHigher;
    temp[0] = mac[4];
    temp[1] = mac[5];

    //
    // Program the unicast mac address.
    //
    Ethernet_setMACAddr(EMAC_BASE,
                        0,
                        macHigher,
                        macLower,
                        ETHERNET_CHANNEL_0);
}


//*****************************************************************************
//
// This example demonstrates the use of the Ethernet Controller.
//
//*****************************************************************************
int
main(void)
{
    unsigned long ulUser0, ulUser1;
    unsigned char pucMACArray[8];

    //
    // User specific IP Address Configuration.
    // Current implementation works with Static IP address only.
    //
    unsigned long IPAddr = 0xC0A8010A;
    unsigned long NetMask = 0xFFFF0000;
    unsigned long GWAddr = 0x00000000;

    //
    // Initializing the CM. Loading the required functions to SRAM.
    //
    CM_init();
    //clear all ipc flags
    IPC_clearFlagLtoR(IPC_CM_L_CPU1_R, IPC_FLAG_ALL);
    //synchronize both core using ipc flag31
    IPC_sync(IPC_CM_L_CPU1_R, IPC_FLAG31);
    //initial test ipc
    int i;
    for(i=0; i<10; i++){readData[i] = i;}
    IPC_sendCommand(IPC_CM_L_CPU1_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE,
                    IPC_CMD_READ_MEM, (uint32_t)readData, 10);
    IPC_waitForAck(IPC_CM_L_CPU1_R, IPC_FLAG0);
    if(IPC_getResponse(IPC_CM_L_CPU1_R) == TEST_PASS){pass = 1;}
    else{pass = 0;}

    //
    // IPC Initialization for Communication with CPU1
    //


    SYSTICK_setPeriod(systickPeriodValue);
    SYSTICK_enableCounter();
    SYSTICK_registerInterruptHandler(SysTickIntHandler);
    SYSTICK_enableInterrupt();

    //
    // Enable processor interrupts.
    //
    Interrupt_enableInProcessor();
        
    // Set user/company specific MAC octets
    // (for this code we are using A8-63-F2-00-00-80)
    // 0x00 MACOCT3 MACOCT2 MACOCT1
    ulUser0 = 0x00F263A8;

    // 0x00 MACOCT6 MACOCT5 MACOCT4
    ulUser1 = 0x00800000;

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    pucMACArray[0] = ((ulUser0 >>  0) & 0xff);
    pucMACArray[1] = ((ulUser0 >>  8) & 0xff);
    pucMACArray[2] = ((ulUser0 >> 16) & 0xff);
    pucMACArray[3] = ((ulUser1 >>  0) & 0xff);
    pucMACArray[4] = ((ulUser1 >>  8) & 0xff);
    pucMACArray[5] = ((ulUser1 >> 16) & 0xff);

    //
    // Initialize ethernet module.
    //
    Ethernet_init(pucMACArray);

    //
    // Initialze the lwIP library, using DHCP.
    //
    lwIPInit(0, pucMACArray, IPAddr, NetMask, GWAddr, IPADDR_USE_STATIC);

    //
    // Initialize the HTTP webserver daemon.
    //
    //httpd_init();

    //
    // Initialize the ccard communication interface
    //
    setupCommInterface();

    //
    // Loop forever. All the work is done in interrupt handlers.
    //
    while(1){
        if(reset_connection){
            reset_connection=false;
            tcp_recvd_cb(NULL,NULL,NULL,NULL);
        }
        if(command_available){
            processCommand();
            command_available=false;
        }

    }
}

//*****************************************************************************
//
// Called by lwIP Library. Could be used for periodic custom tasks.
//
//*****************************************************************************
void lwIPHostTimerHandler(void)
{

}


//*****************************************************************************
//
// Called by lwIP Library. Toggles the led when a command is received by the
// HTTP webserver.
//
//*****************************************************************************
void httpLEDToggle(void)
{
    //
    // Toggle the LED D1 on the control card.
    //
    GPIO_togglePin(DEVICE_GPIO_PIN_LED1);
}

// *****************************************************************************
// cCard specific code
// *****************************************************************************

// TCP received callback function is called when new data was received on COMM_PORT
err_t tcp_recvd_cb(void* arg, struct tcp_pcb* tcppcb, struct pbuf* p,err_t err){
    //payload being NULL indicated that the TCP connection has been terminated
    if(p==NULL){
        tcp_connection_reset_counter++;
        connection_active=false;
        // close the pcb
        if(tcp_close(tcppcb)!=ERR_OK){
            while(1){}
        }
        // listen to new connections again
        //tcp_accept(welcoming_socket_pcb,accept_cb);
        return ERR_OK;
    }
    else{
        if(err==ERR_OK){
            // -- process the data in p , p is NULL if the remote host terminated the connection --
            memcpy(buffer,p->payload,p->len);
            u16_t bytes_read=p->len;
            //indicate that a new command is available for processing in the buffer
            command_available=true;
            // -- indicate that bytes were read and we are ready to receive more data --
            tcp_recved(tcppcb,bytes_read);   //indicate that no_bytes_read were read and we are ready to receive more data
            pbuf_free(p);
            // -- echo the received data back
            tcp_write(tcppcb,sysstatebuffer,sizeof(struct mdriver_msg_sysstate),TCP_WRITE_FLAG_COPY);
            return ERR_OK;
        }
        else{
            //wait forever and preserve debug state
            while(1);
        }
    }
}

// TCP accept callback function is called when an incoming TCP connection on COMM_PORT is established
err_t accept_cb(void * arg, struct tcp_pcb* new_pcb, err_t err){
    if(err==ERR_OK){
        //set the connection active flag
        connection_active=true;
        //register callback for received data
        tcp_recv(new_pcb, tcp_recvd_cb);
        return ERR_OK;
    }
    else{
        while(1){
            //wait forever to preserve debug state
        }
    }
}

/*
 * creates a TCP welcoming socket and sets it to LISTEN state
 */
void setupCommInterface(void){
    //create a new tcp pcb
    struct tcp_pcb* connection_pcb = tcp_new();
    //bind the pcb to the port comm_port
    err_t err=tcp_bind(connection_pcb,IP_ANY_TYPE,COMM_PORT);
    if(err!=ERR_OK){
        if(err==ERR_USE){
            while(1){}
        }
        else if(err==ERR_VAL){
            while(1){}
        }
        while(1){}
    }

    welcoming_socket_pcb=tcp_listen(connection_pcb);
    //start accepting connections on port COMM_PORT
    tcp_accept(welcoming_socket_pcb,accept_cb);
}

uint8_t debug_flagdetect=0;
struct mdriver_msg lastmsg={0};
void processCommand(){

    //copy the data to the shared CM_CPU1 Ram
    memcpy(&ipc_msg_tnb_mns,&buffer,sizeof(ipc_msg_tnb_mns));
    memcpy(&ipc_msg_tnb_mns_c2000,&buffer,sizeof(ipc_msg_tnb_mns));
    ipc_msg_tnb_mns_c2000.buck_flg_byte=ipc_msg_tnb_mns.buck_flg_byte;
    ipc_msg_tnb_mns_c2000.stp_flg_byte=ipc_msg_tnb_mns.stp_flg_byte;
    ipc_msg_tnb_mns_c2000.regen_flg_byte=ipc_msg_tnb_mns.regen_flg_byte;
    ipc_msg_tnb_mns_c2000.resen_flg_byte=ipc_msg_tnb_mns.resen_flg_byte;
    if(ipc_msg_tnb_mns.buck_flg_byte!=0||ipc_msg_tnb_mns.regen_flg_byte!=0||ipc_msg_tnb_mns.resen_flg_byte!=0||ipc_msg_tnb_mns.stp_flg_byte!=0){
        debug_flagdetect=1;
    }

    //send IPC message from CM to CPU1
    IPC_sendCommand(IPC_CM_L_CPU1_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE,
                    IPC_MSG_NEW_MSG, &ipc_msg_tnb_mns_c2000, sizeof(ipc_msg_tnb_mns_c2000));
    IPC_waitForAck(IPC_CM_L_CPU1_R, IPC_FLAG0);

    //wait for IPC message from CPU1 containing system state
    uint32_t command,addr,datalen;
    IPC_readCommand(IPC_CM_L_CPU1_R, IPC_FLAG0, IPC_ADDR_CORRECTION_ENABLE,
                        &command, &addr, &datalen);
    memcpy(&sysstatebuffer,(struct mdriver_msg_sysstate*)addr,sizeof(struct mdriver_msg_sysstate));


}
