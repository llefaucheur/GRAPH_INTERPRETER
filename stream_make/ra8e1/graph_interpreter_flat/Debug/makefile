################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

-include makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/graph/stream_src/subdir.mk
-include src/graph/stream_platform/ra8e1/stream_libraries/CMSIS-DSP/Source/FilteringFunctions/subdir.mk
-include src/graph/stream_platform/ra8e1/stream_libraries/CMSIS-DSP/Source/CommonTables/subdir.mk
-include src/graph/stream_platform/ra8e1/subdir.mk
-include src/graph/stream_platform/platform_services/stream_libraries/standard_library/subdir.mk
-include src/graph/stream_platform/platform_services/stream_libraries/CMSIS-DSP/Source/FilteringFunctions/subdir.mk
-include src/graph/stream_platform/platform_services/subdir.mk
-include src/graph/stream_nodes/signal-processingFR/resampler/subdir.mk
-include src/graph/stream_nodes/signal-processingFR/kws/subdir.mk
-include src/graph/stream_nodes/signal-processingFR/detector2D/subdir.mk
-include src/graph/stream_nodes/signal-processingFR/detector/subdir.mk
-include src/graph/stream_nodes/signal-processingFR/decompressor/subdir.mk
-include src/graph/stream_nodes/signal-processingFR/compressor/subdir.mk
-include src/graph/stream_nodes/elm-lang/TJpgDec/subdir.mk
-include src/graph/stream_nodes/bitbank/JPEGENC/subdir.mk
-include src/graph/stream_nodes/arm/script/subdir.mk
-include src/graph/stream_nodes/arm/router/subdir.mk
-include src/graph/stream_nodes/arm/modulator/subdir.mk
-include src/graph/stream_nodes/arm/fixedbf/subdir.mk
-include src/graph/stream_nodes/arm/filter2D/subdir.mk
-include src/graph/stream_nodes/arm/filter/subdir.mk
-include src/graph/stream_nodes/arm/demodulator/subdir.mk
-include src/graph/stream_nodes/arm/anr/subdir.mk
-include src/graph/stream_nodes/arm/amplifier/subdir.mk
-include src/subdir.mk
-include ra_gen/subdir.mk
-include ra/fsp/src/rm_freertos_port/subdir.mk
-include ra/fsp/src/r_ioport/subdir.mk
-include ra/fsp/src/bsp/mcu/ra8e1/subdir.mk
-include ra/fsp/src/bsp/mcu/all/subdir.mk
-include ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/subdir.mk
-include ra/board/ra8e1_fpb/subdir.mk
-include ra/aws/FreeRTOS/FreeRTOS/Source/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(JMP_DEPS)),)
-include $(JMP_DEPS)
endif
ifneq ($(strip $(FSY_DEPS)),)
-include $(FSY_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(SRC_UPPER_DEPS)),)
-include $(SRC_UPPER_DEPS)
endif
ifneq ($(strip $(P_UPPER_DEPS)),)
-include $(P_UPPER_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(PP_UPPER_DEPS)),)
-include $(PP_UPPER_DEPS)
endif
ifneq ($(strip $(P_DEPS)),)
-include $(P_DEPS)
endif
ifneq ($(strip $(FSY_UPPER_DEPS)),)
-include $(FSY_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(SRC_DEPS)),)
-include $(SRC_DEPS)
endif
ifneq ($(strip $(JMP_UPPER_DEPS)),)
-include $(JMP_UPPER_DEPS)
endif
ifneq ($(strip $(PP_DEPS)),)
-include $(PP_DEPS)
endif
ifneq ($(strip $(SX_DEPS)),)
-include $(SX_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(SX_UPPER_DEPS)),)
-include $(SX_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
graph_interpreter_flat.srec \

SECONDARY_SIZE += \
graph_interpreter_flat.siz \


# All Target
all: graph_interpreter_flat.elf secondary-outputs

# Tool invocations
graph_interpreter_flat.elf: $(OBJS) $(USER_OBJS) $(LINKER_SCRIPT)
	$(file > graph_interpreter_flat.elf.in,-mthumb -mfloat-abi=hard -mcpu=cortex-m85+nopacbti -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -g -T "fsp.ld" -Xlinker --gc-sections -L "C:\\Users\\laulef01\\e2_studio\\workspace\\graph_interpreter_flat/script" -Wl,-Map,"graph_interpreter_flat.map" --specs=nano.specs -o "graph_interpreter_flat.elf" -Wl,--start-group $(OBJS) $(USER_OBJS) $(LIBS) -Wl,--end-group)
	@echo Building target: $@ && arm-none-eabi-gcc @"graph_interpreter_flat.elf.in"
graph_interpreter_flat.srec: graph_interpreter_flat.elf
	arm-none-eabi-objcopy -O srec "graph_interpreter_flat.elf"  "graph_interpreter_flat.srec"
graph_interpreter_flat.siz: graph_interpreter_flat.elf
	arm-none-eabi-size --format=berkeley "graph_interpreter_flat.elf"
# Other Targets
clean:
	-$(file > clean_file.tmp,  $(JMP_DEPS)  $(FSY_DEPS)  $(C_UPPER_DEPS)  $(SECONDARY_SIZE)  $(SRC_UPPER_DEPS)  $(P_UPPER_DEPS)  $(S_DEPS)  $(PP_UPPER_DEPS)  $(P_DEPS)  $(FSY_UPPER_DEPS)  $(C_DEPS)  $(SRC_DEPS)  $(JMP_UPPER_DEPS)  $(PP_DEPS)  $(SX_DEPS)  $(ASM_UPPER_DEPS)  $(SX_UPPER_DEPS)  $(OBJS)  $(SECONDARY_FLASH)  $(ASM_DEPS)  $(SREC)  $(S_UPPER_DEPS)  $(MAP) )
	-@ xargs -P 2 -s 32000 -t -a clean_file.tmp $(RM)
	-@$(RM) clean_file.tmp
	-$(RM)  graph_interpreter_flat.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
