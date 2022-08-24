################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../alt_generalpurpose_io.c \
../hps_linux.c 

OBJS += \
./alt_generalpurpose_io.o \
./hps_linux.o 

C_DEPS += \
./alt_generalpurpose_io.d \
./hps_linux.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler 7.5.0 [arm-linux-gnueabihf]'
	arm-linux-gnueabihf-gcc.exe -Dsoc_cv_av -I"D:\intelFPGA\20.1\embedded\ip\altera\hps\altera_hps\hwlib\include" -I"D:\intelFPGA\20.1\embedded\ip\altera\hps\altera_hps\hwlib\include\soc_cv_av" -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


