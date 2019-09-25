################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/home/yingshaoxo/ti/ccs910/ccs/tools/compiler/ti-cgt-msp430_18.12.2.LTS/bin/cl430" -vmsp --use_hw_mpy=16 --include_path="/home/yingshaoxo/ti/ccs910/ccs/ccs_base/msp430/include" --include_path="/home/yingshaoxo/Software_Engineering/super_led_fan_lte/OpenSmart_LCD/MSP430F169" --include_path="/home/yingshaoxo/ti/ccs910/ccs/tools/compiler/ti-cgt-msp430_18.12.2.LTS/include" --advice:power="all" --define=__MSP430F169__ -g --printf_support=full --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


