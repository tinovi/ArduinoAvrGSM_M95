################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../gsm/AGSMSSL.cpp \
../gsm/AGSMSSLProvicer.cpp \
../gsm/GSM3CircularBuffer.cpp \
../gsm/GSM3MobileAccessProvider.cpp \
../gsm/GSM3MobileClientProvider.cpp \
../gsm/GSM3MobileClientService.cpp \
../gsm/GSM3MobileDataNetworkProvider.cpp \
../gsm/GSM3MobileNetworkProvider.cpp \
../gsm/GSM3MobileNetworkRegistry.cpp \
../gsm/GSM3MobileSMSProvider.cpp \
../gsm/GSM3SMSService.cpp \
../gsm/GSM3ShieldV1.cpp \
../gsm/GSM3ShieldV1AccessProvider.cpp \
../gsm/GSM3ShieldV1BaseProvider.cpp \
../gsm/GSM3ShieldV1ClientProvider.cpp \
../gsm/GSM3ShieldV1DataNetworkProvider.cpp \
../gsm/GSM3ShieldV1DirectModemProvider.cpp \
../gsm/GSM3ShieldV1ModemCore.cpp \
../gsm/GSM3ShieldV1MultiClientProvider.cpp \
../gsm/GSM3ShieldV1SMSProvider.cpp \
../gsm/GSM3SoftSerial.cpp 

OBJS += \
./gsm/AGSMSSL.o \
./gsm/AGSMSSLProvicer.o \
./gsm/GSM3CircularBuffer.o \
./gsm/GSM3MobileAccessProvider.o \
./gsm/GSM3MobileClientProvider.o \
./gsm/GSM3MobileClientService.o \
./gsm/GSM3MobileDataNetworkProvider.o \
./gsm/GSM3MobileNetworkProvider.o \
./gsm/GSM3MobileNetworkRegistry.o \
./gsm/GSM3MobileSMSProvider.o \
./gsm/GSM3SMSService.o \
./gsm/GSM3ShieldV1.o \
./gsm/GSM3ShieldV1AccessProvider.o \
./gsm/GSM3ShieldV1BaseProvider.o \
./gsm/GSM3ShieldV1ClientProvider.o \
./gsm/GSM3ShieldV1DataNetworkProvider.o \
./gsm/GSM3ShieldV1DirectModemProvider.o \
./gsm/GSM3ShieldV1ModemCore.o \
./gsm/GSM3ShieldV1MultiClientProvider.o \
./gsm/GSM3ShieldV1SMSProvider.o \
./gsm/GSM3SoftSerial.o 

CPP_DEPS += \
./gsm/AGSMSSL.d \
./gsm/AGSMSSLProvicer.d \
./gsm/GSM3CircularBuffer.d \
./gsm/GSM3MobileAccessProvider.d \
./gsm/GSM3MobileClientProvider.d \
./gsm/GSM3MobileClientService.d \
./gsm/GSM3MobileDataNetworkProvider.d \
./gsm/GSM3MobileNetworkProvider.d \
./gsm/GSM3MobileNetworkRegistry.d \
./gsm/GSM3MobileSMSProvider.d \
./gsm/GSM3SMSService.d \
./gsm/GSM3ShieldV1.d \
./gsm/GSM3ShieldV1AccessProvider.d \
./gsm/GSM3ShieldV1BaseProvider.d \
./gsm/GSM3ShieldV1ClientProvider.d \
./gsm/GSM3ShieldV1DataNetworkProvider.d \
./gsm/GSM3ShieldV1DirectModemProvider.d \
./gsm/GSM3ShieldV1ModemCore.d \
./gsm/GSM3ShieldV1MultiClientProvider.d \
./gsm/GSM3ShieldV1SMSProvider.d \
./gsm/GSM3SoftSerial.d 


# Each subdirectory must supply rules for building sources it contributes
gsm/%.o: ../gsm/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"C:\ed\eclipseWS\ZiinodeAvrGSM" -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -funsigned-char -funsigned-bitfields -fno-exceptions -Wno-switch -mmcu=atmega644p -DF_CPU=7372800UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


