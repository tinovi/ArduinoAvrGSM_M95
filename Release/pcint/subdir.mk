################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../pcint/PinChangeInterrupt.cpp \
../pcint/PinChangeInterrupt0.cpp \
../pcint/PinChangeInterrupt1.cpp \
../pcint/PinChangeInterrupt2.cpp \
../pcint/PinChangeInterrupt3.cpp 

OBJS += \
./pcint/PinChangeInterrupt.o \
./pcint/PinChangeInterrupt0.o \
./pcint/PinChangeInterrupt1.o \
./pcint/PinChangeInterrupt2.o \
./pcint/PinChangeInterrupt3.o 

CPP_DEPS += \
./pcint/PinChangeInterrupt.d \
./pcint/PinChangeInterrupt0.d \
./pcint/PinChangeInterrupt1.d \
./pcint/PinChangeInterrupt2.d \
./pcint/PinChangeInterrupt3.d 


# Each subdirectory must supply rules for building sources it contributes
pcint/%.o: ../pcint/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\ed\eclipseWS\ZiinodeAvrGSM" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -funsigned-char -funsigned-bitfields -fno-exceptions -Wno-switch -mmcu=atmega644p -DF_CPU=7372800UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


