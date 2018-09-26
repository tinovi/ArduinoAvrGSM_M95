################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ByteBuffer.cpp \
../DHT.cpp \
../DS1820.cpp \
../DallasTemperature.cpp \
../HttpClient.cpp \
../OneWire.cpp \
../ZiinodeGsm.cpp \
../b64.cpp \
../main.cpp 

OBJS += \
./ByteBuffer.o \
./DHT.o \
./DS1820.o \
./DallasTemperature.o \
./HttpClient.o \
./OneWire.o \
./ZiinodeGsm.o \
./b64.o \
./main.o 

CPP_DEPS += \
./ByteBuffer.d \
./DHT.d \
./DS1820.d \
./DallasTemperature.d \
./HttpClient.d \
./OneWire.d \
./ZiinodeGsm.d \
./b64.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\ed\eclipseWS\ZiinodeAvrGSM" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -funsigned-char -funsigned-bitfields -fno-exceptions -Wno-switch -mmcu=atmega644p -DF_CPU=7372800UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


