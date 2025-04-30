# CCARD PC Interface
A repository to implement a TCP connection beteween the TMDSCNCD28388D Control Card and a Windows or Linux / ROS PC.

## Network Configuration

### PC Settings

* **Subnet Mask** : `255.255.255.0`
* **IP** : `192.168.0.1`
* **Gateway** : Windows: `192.168.0.0` Linux: ` <empty>`

On Linux, try to leave the Gateway setting empty. Otherwise, Linux might try to contact a non-existing gateway which causes a LAN connection error.

### Control Card Settings

The network configuration of the Control Card is done in the `main` routine of the CM at `lwIPInit(0, pucMACArray, IPAddr, NetMask, GWAddr, IPADDR_USE_STATIC);`.

* Subnet Mask : `255.255.255.0`
* IP : `192.168.0.4`
* MAC-Address: `A8-63-F2-00-00-80`

## Compilation & Deploying

* Fork the `ethernet_c28_config` project from the ressource explorer in CCS.

* Compile `ethernet_c28_config` and `enet_lwip`

* Under `Run`->`Debug Configurations`->`Main`->`Target Configuration` select `C28xx_CPU1` and `Cortex_M4_0`, then click `Debug`

* In the Debug Perspective select `Connect Target` for both CPU targets:

  ![](img\connect_target.png)

* Load the compiled codes of the two projects first onto `C28xx_CPU1` and then on `Cortex_M4_0`:

  ![](\img\program_target.png)

## Launching Multiple Cores for Debugging

* Go to `Debug Configurations`->`Program` and for each core go to `Project`->`Workspace` and select the associated project.
* Go to `Debug Configurations`->`Target`->`Auto Run and Launch Options` and check `Connect to the target on debugger startup`

## Network Debugging Linux / `lwIP`

[lwIP Documentation](https://www.nongnu.org/lwip/2_1_x/index.html)

* ROS Node Error Message `No Route To Host`: Use `sudo arp -s 192.168.0.4 A8:63:F2:00:00:80` to manually resolve the IP / fill in the ARP table
* On Linux, ensure that the `Gateway` option in the LAN settings is left empty.
* Make sure to free packet buffers in `lwIP` using `pbuf_free()`, otherwise uC might not react after receiving a certain number of packets.
* Adjust the settings in `lwipopts.h` to enable/disable certain protocols and set protocol compilation parameters

## General Tips

* When inspecting the binary contents of `buffer` use the memory browser with input `&buffer`. The variable preview will show the contents of `buffer` interpreted as ASCII characters since `uint8_t` is internally defined as `unsigned char`.

## Error Log

Linker Error `No Source Files - Nothing To Do` : Issue was that the -n flag was set in the ARM Compiler which caused `.asm` files to be generated instead of `.obj` files.te