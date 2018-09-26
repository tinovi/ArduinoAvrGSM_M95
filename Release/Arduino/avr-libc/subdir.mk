################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Arduino/avr-libc/malloc.c \
../Arduino/avr-libc/realloc.c 

C_DEPS += \
./Arduino/avr-libc/malloc.d \
./Arduino/avr-libc/realloc.d 

OBJS += \
./Arduino/avr-libc/malloc.o \
./Arduino/avr-libc/realloc.o 


# Each subdirectory must supply rules for building sources it contributes
Arduino/avr-libc/%.o: ../Arduino/avr-libc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -I"C:\ed\eclipseWS\ZiinodeAvrGSM" -Wall -Os -ffunction-sections -fdata-sections -ffunction-sections -fdata-sections -std=gnu99 -Wno-switch -mmcu=atmega644p -DF_CPU=7372800UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


