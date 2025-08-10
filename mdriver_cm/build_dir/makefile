################################################################################
# Automatically-generated file. Do not edit!
################################################################################

CG_TOOL_ROOT := $(CCS_DIR)/tools/compiler/ti-cgt-arm_20.2.7.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./comm_interface.obj" \
"./f2838xif.obj" \
"./fs.obj" \
"./httpd.obj" \
"./lwiplib.obj" \
"./mdriver_cm_main.obj" \
"./pinout.obj" \
"./startup_ccs.obj" \
"./ustdlib.obj" \
"../2838x_flash_lnk_cm_lwip.cmd" \
$(GEN_CMDS__FLAG) \
-l"$(C2000WareDir)/driverlib/f2838x/driverlib_cm/ccs/Debug/driverlib_cm.lib" \
-llibc.a \

-include ../makefile.init

RM := rm -rf
RMDIR := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
enet_lwip_cm.out 

EXE_OUTPUTS__QUOTED += \
"enet_lwip_cm.out" 


# All Target
all: enet_lwip_cm.out

# Tool invocations
enet_lwip_cm.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"$(CCS_DIR)/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=none -me -O0 --opt_for_speed=0 --define=_FLASH --define=ccs -g --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual -z -m"enet_lwip_cm.map" --heap_size=0 --stack_size=2048 -i"$(CCS_DIR)/tools/compiler/ti-cgt-arm_20.2.7.LTS/lib" -i"$(CCS_DIR)/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" -i"$(CCS_DIR)/tools/compiler" -i"$(C2000WareDir)/driverlib/f2838x/driverlib_cm/ccs/Debug" --reread_libs --warn_sections --xml_link_info="enet_lwip_cm_linkInfo.xml" --rom_model -o "enet_lwip_cm.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)
	-$(RM) "comm_interface.obj" "f2838xif.obj" "fs.obj" "httpd.obj" "lwiplib.obj" "mdriver_cm_main.obj" "pinout.obj" "startup_ccs.obj" "ustdlib.obj" 
	-$(RM) "comm_interface.d" "f2838xif.d" "fs.d" "httpd.d" "lwiplib.d" "mdriver_cm_main.d" "pinout.d" "startup_ccs.d" "ustdlib.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

