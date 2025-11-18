################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c \
../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c 

C_DEPS += \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.d \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.d 

CREF += \
Graph_RTOS.cref 

OBJS += \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.o \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.o 

MAP += \
Graph_RTOS.map 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/%.o: ../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m85 -mthumb -mlittle-endian -mfloat-abi=hard -O0 -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -flax-vector-conversions -fshort-enums -fno-unroll-loops -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra_gen" -I"." -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra_cfg\\fsp_cfg\\bsp" -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra_cfg\\fsp_cfg" -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra_cfg\\aws" -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\src" -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra\\fsp\\inc" -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra\\fsp\\inc\\api" -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra\\fsp\\inc\\instances" -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra\\fsp\\src\\rm_freertos_port" -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra\\aws\\FreeRTOS\\FreeRTOS\\Source\\include" -I"C:\\Users\\laulef01\\e2_studio\\workspace\\Graph_RTOS\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -D_RENESAS_RA_ -D_RA_CORE=CM85 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

