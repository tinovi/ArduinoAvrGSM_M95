################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../NeoHWSerial/NeoHWSerial.cpp \
../NeoHWSerial/NeoHWSerial0.cpp \
../NeoHWSerial/NeoHWSerial1.cpp \
../NeoHWSerial/NeoHWSerial2.cpp \
../NeoHWSerial/NeoHWSerial3.cpp 

OBJS += \
./NeoHWSerial/NeoHWSerial.o \
./NeoHWSerial/NeoHWSerial0.o \
./NeoHWSerial/NeoHWSerial1.o \
./NeoHWSerial/NeoHWSerial2.o \
./NeoHWSerial/NeoHWSerial3.o 

CPP_DEPS += \
./NeoHWSerial/NeoHWSerial.d \
./NeoHWSerial/NeoHWSerial0.d \
./NeoHWSerial/NeoHWSerial1.d \
./NeoHWSerial/NeoHWSerial2.d \
./NeoHWSerial/NeoHWSerial3.d 


# Each subdirectory must supply rules for building sources it contributes
NeoHWSerial/%.o: ../NeoHWSerial/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\ed\eclipseWS\ZiinodeAvrGSM" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -funsigned-char -funsigned-bitfields -fno-exceptions -Wno-switch -mmcu=atmega644p -DF_CPU=7372800UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


