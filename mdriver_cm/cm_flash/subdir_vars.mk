################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../2838x_flash_lnk_cm_lwip.cmd 

C_SRCS += \
../comm_interface.c \
$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/lwip-2.1.2/ports/C2000/netif/f2838xif.c \
$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/lwip-2.1.2/src/apps/http/fs.c \
$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/lwip-2.1.2/src/apps/http/httpd.c \
$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/utils/lwiplib.c \
../mdriver_cm_main.c \
$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/board_drivers/pinout.c \
../startup_ccs.c \
$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/utils/ustdlib.c 

C_DEPS += \
./comm_interface.d \
./f2838xif.d \
./fs.d \
./httpd.d \
./lwiplib.d \
./mdriver_cm_main.d \
./pinout.d \
./startup_ccs.d \
./ustdlib.d 

OBJS += \
./comm_interface.obj \
./f2838xif.obj \
./fs.obj \
./httpd.obj \
./lwiplib.obj \
./mdriver_cm_main.obj \
./pinout.obj \
./startup_ccs.obj \
./ustdlib.obj 

OBJS__QUOTED += \
"comm_interface.obj" \
"f2838xif.obj" \
"fs.obj" \
"httpd.obj" \
"lwiplib.obj" \
"mdriver_cm_main.obj" \
"pinout.obj" \
"startup_ccs.obj" \
"ustdlib.obj" 

C_DEPS__QUOTED += \
"comm_interface.d" \
"f2838xif.d" \
"fs.d" \
"httpd.d" \
"lwiplib.d" \
"mdriver_cm_main.d" \
"pinout.d" \
"startup_ccs.d" \
"ustdlib.d" 

C_SRCS__QUOTED += \
"../comm_interface.c" \
"$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/lwip-2.1.2/ports/C2000/netif/f2838xif.c" \
"$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/lwip-2.1.2/src/apps/http/fs.c" \
"$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/lwip-2.1.2/src/apps/http/httpd.c" \
"$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/utils/lwiplib.c" \
"../mdriver_cm_main.c" \
"$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/board_drivers/pinout.c" \
"../startup_ccs.c" \
"$(C2000WareDir)/libraries/communications/Ethernet/third_party/lwip/utils/ustdlib.c" 


