/*
This file is part of the Tinovi.io GSM communications library for Arduino

This library has been developed by Tinovi.io based on Arduino GSM module library: https://github.com/arduino/Arduino/tree/master/libraries/GSM

2016

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at
https://github.com/BlueVia/Official-Arduino
*/

#ifndef ZiinodeGsm_h
#define ZiinodeGsm_h

#include <Arduino/Arduino.h>
#include <avr/eeprom.h>

#include <gsm/GSM.h>

#include "ByteBuffer.h"
#include "config.h"


//#ifndef F_CPU
//#define F_CPU              14745600UL
//#endif

//prefix
#define DISCONN 3
#define ACK 4
#define ENQ 5
#define CONN_ACK 6
#define TRAP 7  //
#define ANNOTATION 8  //
#define LOG 9  //
#define SUBDEV 10  //

//command
#define CMD_DISCOVER 11   //character response  +
#define CMD_MAC 12  //6 byte mac
#define CMD_NET 13  //net settings IP+GW+SM+DNS = 4x4=16bytes
#define CMD_INT_TYPE 14  // 8 byte
#define CMD_TRIG 15  //  6 byte
#define CMD_TRIG_ALL  16  // TRIG_COUNT * 6 byte
#define CMD_TRAP_ADDR 17  //28 byte adderess
#define CMD_TRIG_OUT 18  // 2byte
#define CMD_POSTING_INTERVAL 19  // 4byte
#define CMD_WDT_SLEEP 20  // 2byte
#define CMD_BAUD 21  // 2byte

#define MAX_SIZE 156

#define read_eeprom_byte(address) eeprom_read_byte ((const uint8_t*)address)
#define write_eeprom_byte(address,value) eeprom_write_byte ((uint8_t*)address,(uint8_t)value)
//#define read_eeprom_word(address) eeprom_read_word ((const uint16_t*)address)
//#define write_eeprom_word(address,value) eeprom_write_word ((uint16_t*)address,(uint16_t)value)
#define read_eeprom_dword(address) eeprom_read_dword ((const uint32_t*)address)
#define write_eeprom_dword(address,value) eeprom_write_dword ((uint32_t*)address,(uint32_t)value)
//#define read_eeprom_float(address) eeprom_read_float ((const float *)address)
//#define write_eeprom_float(address,value) eeprom_write_float ((float*)address,(float)value)
#define read_eeprom_array(address,value_p,length) eeprom_read_block ((void *)value_p, (const void *)address, length)
#define write_eeprom_array(address,value_p,length) eeprom_write_block ((const void *)value_p, (void *)address, length)



template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
    	eeprom_write_byte((unsigned char *)ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
	  *p++ = eeprom_read_byte((unsigned char *)ee++);
    return i;
}

class ZiinodeGsm
{
public:
	ZiinodeGsm();
	typedef void (*TOnDataHandler)(byte, ByteBuffer*);
	typedef void (*TOnConnAckHandler)();
	typedef void (*TOnAckHandler)(int);
    void    begin(TOnDataHandler handler, TOnConnAckHandler cack, TOnAckHandler ack);
    void    resetDevId();
    void    stop();
    void    stopS();
    void    begin(char const *apName);
    void debugL(const char *fmt, ...);
    void flush();
    size_t    write(uint8_t b);
    size_t    write(const uint8_t *buf, size_t size);
    void    writeInt(int in);
    void    writeIntE(int in);
    void    writeUint32(uint32_t in);
    void    writeInt64(int64_t in);
    void    sendEvent(int64_t time, int code, const char *fmt, ...);
    void    writeLog(int64_t time, int code, const char *fmt, ...);
    void 	sendNet();
    void    sendInterval();
    void    sendBinTrap();
    void    sendEnq();
    void    ether_loop();
    boolean checkConn();
    uint8_t connected();
    int readThermistor(int adcpin, int bconf);
    boolean hasConnected();
    void setDevId(char * did);
    void setAddress(char * addr);
    
    void    setTimeout(unsigned long seconds);
    void    setDebugOutput(boolean debug);
    float 	readAnalogVolts(int sensorpin, int voltage);
    int    	readVcc();
    int     serverLoop();
    boolean turnOnGsm();
    void gsmOff();
    void gsmRestart();
    int16_t getInt();
    int getModemState();
    uint32_t postingInterval;
    int    	ncnt();
    int    msg(uint8_t type, int size);
    void    writeBody(uint8_t cmd, const uint8_t *buf, int size);

private:
    byte cmd;
    int zsize;
    int cnt;
    bool hasAddr;
    boolean notConnected;
    ByteBuffer* _clientBuffer;
    GSMClient _client;
    TOnDataHandler _onDataHandler;
    TOnConnAckHandler _cack;
    TOnAckHandler _ack;
    char _devid[9];
    char address[17];
    boolean _hasDevId;
};



#endif
