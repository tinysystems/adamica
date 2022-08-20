################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/bin/cl430" -vmspx --data_model=large --use_hw_mpy=F5 --include_path="C:/ti/ccs1040/ccs/ccs_base/msp430/include" --include_path="C:/Users/hakim/workspace_v12/adamica" --include_path="C:/Users/hakim/workspace_v12/adamica/fram-utilities/ctpl" --include_path="C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR5994__ --define=CTPL_STACK_SIZE=160 --define=CTPL_SAVE_TA0 --define=CTPL_SAVE_REF_A --define=CTPL_SAVE_ADC12_B --define=_MPU_ENABLE -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/bin/cl430" -vmspx --data_model=large --use_hw_mpy=F5 --include_path="C:/ti/ccs1040/ccs/ccs_base/msp430/include" --include_path="C:/Users/hakim/workspace_v12/adamica" --include_path="C:/Users/hakim/workspace_v12/adamica/fram-utilities/ctpl" --include_path="C:/Users/hakim/workspace_v12/adamica/fram-utilities/nvs" --include_path="C:/Users/hakim/workspace_v12/adamica/Includes/adamicalib" --include_path="C:/Users/hakim/workspace_v12/adamica/Includes/memlib" --include_path="C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR5994__ --define=CTPL_STACK_SIZE=160 --define=CTPL_SAVE_TA0 --define=CTPL_SAVE_REF_A --define=CTPL_SAVE_ADC12_B --define=_MPU_ENABLE -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


