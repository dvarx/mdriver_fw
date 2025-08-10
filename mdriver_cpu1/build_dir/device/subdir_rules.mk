################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
device/%.obj: ../device/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"$(CCS_DIR)/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu64 --idiv_support=idiv0 --tmu_support=tmu0 --vcu_support=vcrc -Ooff --include_path="$(SRC_DIR)/mdriver_fw/mdriver_cpu1" --include_path="$(SRC_DIR)/mdriver_fw/mdriver_cpu1/device" --include_path="$(C2000WareDir)/driverlib/f2838x/driverlib" --include_path="$(CCS_DIR)/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --define=_FLASH --define=DEBUG --define=ETHERNET --define=CPU1 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="device/$(basename $(<F)).d_raw" --obj_directory="device" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '

device/%.obj: ../device/%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"$(CCS_DIR)/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu64 --idiv_support=idiv0 --tmu_support=tmu0 --vcu_support=vcrc -Ooff --include_path="$(SRC_DIR)/mdriver_fw/mdriver_cpu1" --include_path="$(SRC_DIR)/mdriver_fw/mdriver_cpu1/device" --include_path="$(C2000WareDir)/driverlib/f2838x/driverlib" --include_path="$(CCS_DIR)/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --define=_FLASH --define=DEBUG --define=ETHERNET --define=CPU1 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="device/$(basename $(<F)).d_raw" --obj_directory="device" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


