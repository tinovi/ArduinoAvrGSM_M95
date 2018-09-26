################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Arduino/WInterrupts.c \
../Arduino/hooks.c \
../Arduino/wiring.c \
../Arduino/wiring_analog.c \
../Arduino/wiring_digital.c \
../Arduino/wiring_pulse.c \
../Arduino/wiring_shift.c 

CPP_SRCS += \
../Arduino/CDC.cpp \
../Arduino/HID.cpp \
../Arduino/HardwareSerial.cpp \
../Arduino/HardwareSerial0.cpp \
../Arduino/HardwareSerial1.cpp \
../Arduino/HardwareSerial2.cpp \
../Arduino/HardwareSerial3.cpp \
../Arduino/IPAddress.cpp \
../Arduino/Print.cpp \
../Arduino/Stream.cpp \
../Arduino/Tone.cpp \
../Arduino/USBCore.cpp \
../Arduino/WMath.cpp \
../Arduino/WString.cpp \
../Arduino/main.cpp \
../Arduino/new.cpp 

C_DEPS += \
./Arduino/WInterrupts.d \
./Arduino/hooks.d \
./Arduino/wiring.d \
./Arduino/wiring_analog.d \
./Arduino/wiring_digital.d \
./Arduino/wiring_pulse.d \
./Arduino/wiring_shift.d 

OBJS += \
./Arduino/CDC.o \
./Arduino/HID.o \
./Arduino/HardwareSerial.o \
./Arduino/HardwareSerial0.o \
./Arduino/HardwareSerial1.o \
./Arduino/HardwareSerial2.o \
./Arduino/HardwareSerial3.o \
./Arduino/IPAddress.o \
./Arduino/Print.o \
./Arduino/Stream.o \
./Arduino/Tone.o \
./Arduino/USBCore.o \
./Arduino/WInterrupts.o \
./Arduino/WMath.o \
./Arduino/WString.o \
./Arduino/hooks.o \
./Arduino/main.o \
./Arduino/new.o \
./Arduino/wiring.o \
./Arduino/wiring_analog.o \
./Arduino/wiring_digital.o \
./Arduino/wiring_pulse.o \
./Arduino/wiring_shift.o 

CPP_DEPS += \
./Arduino/CDC.d \
./Arduino/HID.d \
./Arduino/HardwareSerial.d \
./Arduino/HardwareSerial0.d \
./Arduino/HardwareSerial1.d \
./Arduino/HardwareSerial2.d \
./Arduino/HardwareSerial3.d \
./Arduino/IPAddress.d \
./Arduino/Print.d \
./Arduino/Stream.d \
./Arduino/Tone.d \
./Arduino/USBCore.d \
./Arduino/WMath.d \
./Arduino/WString.d \
./Arduino/main.d \
./Arduino/new.d 


# Each subdirectory must supply rules for building sources it contributes
Arduino/%.o: ../Arduino/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\ed\eclipseWS\ZiinodeAvrGSM" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -funsigned-char -funsigned-bitfields -fno-exceptions -Wno-switch -mmcu=atmega644p -DF_CPU=7372800UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Arduino/%.o: ../Arduino/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -I"C:\ed\eclipseWS\ZiinodeAvrGSM" -Wall -Os -ffunction-sections -fdata-sections -ffunction-sections -fdata-sections -std=gnu99 -Wno-switch -mmcu=atmega644p -DF_CPU=7372800UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


