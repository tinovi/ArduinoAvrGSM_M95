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

//needed for library
#include "ZiinodeGsm.h"
#include "DallasTemperature.h"
#include "DHT.h" //https://github.com/adafruit/DHT-sensor-library    place under libraries and rename folder to DHT
#include <NeoHWSerial/NeoHWSerial.h>
#include <pcint/PinChangeInterrupt.h>

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

ZiinodeGsm zn;

//analog 10k thermocouple
#define NTC10K 1
//dallas sensor
#define INPUT_DS 2
//digital themp & hummudity
#define INPUT_TH_11 3
#define INPUT_TH_22 4
//analog read
#define VOLTAGE 5
#define COUNTER 6

//total 4+1
//#define IN_start 15
//total 20*6+1
//#define TRIG_start 20

#define IN_COUNT 8
#define READ_COUNT 17
#define OUT_COUNT 4
#define TRIG_COUNT 20

#define GSM_RESET 20
#define GSM_ONOFF 21
#define GSM_STATUS 22


typedef struct{
	uint8_t input;
	uint8_t oper;
	uint8_t out;
	uint8_t out_dir;
	int16_t val;
} trigger_t;//size 6

uint8_t in_pin[IN_COUNT] = { A0, A1, A2, A3, A4, A5,A6, A7 };
uint8_t out_pins[OUT_COUNT] = {2,3,4,5};

//reading digital may have hummuduty as well


void initBaud();

#define OPER_EQUAL 1
#define OPER_LESS 2
#define OPER_GREATER 3

int readings[READ_COUNT];

byte in_type[IN_COUNT];
int in_res[IN_COUNT];
trigger_t trigger[TRIG_COUNT];

byte out_s;

boolean hasAck=false;

volatile int f_wdt=0;
int wdt_8sec=0;
int baud=11200;
#define BAUD_ADR 28//3
#define WDT_8S_ADR 31//3
#define EEP_S_ADR 34  //5
#define IN_TYP_ADR 39//9
#define IN_RES_ADR 48//17
#define TRIG_ADR 65 //



/***************************************************
 *  Name:        enterSleep
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Enters the arduino into sleep mode.
 *
 ***************************************************/
void enterSleep(void)
{
	//set_sleep_mode(SLEEP_MODE_IDLE);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
  sleep_enable();

  /* Now enter sleep mode. */
  sleep_mode();

  /* The program will continue from here after the WDT timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */

  /* Re-enable the peripherals. */
  power_all_enable();
}




/***************************************************
 *  Name:        ISR(WDT_vect)
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Watchdog Interrupt Service. This
 *               is executed when watchdog timed out.
 *
 ***************************************************/

ISR(WDT_vect)
{
	if(f_wdt > 0) //continue sleeping
  {
    f_wdt--;
#if DEBUG
	Tr("WDTv:");
	Trln(f_wdt);
#endif
  }
  else
  {
#if DEBUG
	Trln("WDTv.");
#endif
  }
	wdt_reset();
  //WDTCSR |= _BV(WDIE);
}


/***************************************************
 *  Name:        setup
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  None.
 *
 *  Description: Setup for the serial comms and the
 *                Watch dog timeout.
 *
 ***************************************************/
//call sthi in setup
//http://donalmorrissey.blogspot.com/2010/04/sleeping-arduino-part-5-wake-up-via.html
#ifdef POWERSAVE
void initPowerSave()
{


  /*** Setup the WDT ***/
	 cli();

  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);

  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */

  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);
  sei();
#if DEBUG
	Trln("initPowerSave done..");
#endif
  delay(100); //Allow for serial print to complete.
}
#endif


void  save8SInt(){
	write_eeprom_byte(WDT_8S_ADR, 55);
	EEPROM_writeAnything(WDT_8S_ADR+1,wdt_8sec);
}
void  saveBaud(){
	write_eeprom_byte(BAUD_ADR, 55);
	EEPROM_writeAnything(BAUD_ADR+1,baud);
}

void  savePostingInt(){
	#if DEBUG
		Tr("int:");
		Trln(zn.postingInterval);
	#endif
	write_eeprom_byte(EEP_S_ADR, 55);
	EEPROM_writeAnything(EEP_S_ADR+1,zn.postingInterval);
}


void saveInType(){
	write_eeprom_byte(IN_TYP_ADR, 55);
	EEPROM_writeAnything(IN_TYP_ADR+1,in_type);
}

void saveRes(){
	write_eeprom_byte(IN_RES_ADR, 55);
	EEPROM_writeAnything(IN_RES_ADR+1,in_res);
}

void saveTrig(){
	write_eeprom_byte(TRIG_ADR, 55);
	EEPROM_writeAnything(TRIG_ADR+1,trigger);
}

void readConf(){
  //total 4+1
	  if(read_eeprom_byte(BAUD_ADR)==55){
		  EEPROM_readAnything(BAUD_ADR+1,baud);
	  }
	  initBaud();

  if(read_eeprom_byte(WDT_8S_ADR)==55){
	  EEPROM_readAnything(WDT_8S_ADR+1,wdt_8sec);
  }

  if(read_eeprom_byte(EEP_S_ADR)==55){
	  EEPROM_readAnything(EEP_S_ADR+1,zn.postingInterval);
  }

  if(read_eeprom_byte(IN_TYP_ADR)==55){
	  EEPROM_readAnything(IN_TYP_ADR+1,in_type);
  }
  if(read_eeprom_byte(IN_RES_ADR)==55){
	  EEPROM_readAnything(IN_RES_ADR+1,in_res);
  }
  //total 20*6+1
  if(read_eeprom_byte(TRIG_ADR)==55){
	  EEPROM_readAnything(TRIG_ADR+1,trigger);
  }

	for (int i = 0; i < IN_COUNT; i++) {
		if(in_type[i]==COUNTER){
			pinMode(in_pin[i],INPUT_PULLUP);
			attachPinChangeInterrupt(i, FALLING);
		}
	}
}


void sendWDTSleepCnt(){
	zn.msg(CMD_WDT_SLEEP,2);
	zn.writeInt(wdt_8sec);
}

void sendInputType(){
	zn.msg(CMD_INT_TYPE, IN_COUNT*3);
	zn.write((const byte *)in_type,IN_COUNT);
	zn.write((const byte *)in_res,IN_COUNT*2);
}

void sendTrigAll(){
	zn.writeBody(CMD_TRIG_ALL,(const byte *)trigger,TRIG_COUNT * 6);
}

void sendNet(){
//	zn.write(CMD_NET);
//	zn.writeBody((const byte*)&zn.getNet(),16);
}

void sendBinTrap(){
	zn.msg(TRAP,(READ_COUNT * 2) + 1);
    for(int i=0;i<READ_COUNT;i++){
#if DEBUG
    	Tr(readings[i]);
    	Tr(',');
#endif
    	int in = i/2;
    	if(in_type[in]==COUNTER){
    		int rer = in_res[in];
    		if(rer==0){
    			rer=1;
    		}
        	zn.writeIntE(readings[i]*rer);
    		readings[i] = 0;
    	}else{
        	zn.writeIntE(readings[i]);
    	}
    }
	zn.write(out_s);
#if DEBUG
	Tr(':');
    Trln(out_s);
#endif
	//zn.write((const byte*)readings,8);
}

void ack(int mid){
#if DEBUG
	Tr(">ack:");
	Trln(mid);
#endif
}

void cack(){
	hasAck = true;
	sendInputType();
	sendTrigAll();
	sendWDTSleepCnt();
	zn.sendInterval();
#if DEBUG
	Trln("conn ack.... ");
#endif
}

#define BUFFER_SIZE 128
int sdSessid;
int sdMsgId;
uint32_t u32StartTime;
uint8_t arr[BUFFER_SIZE];
volatile uint8_t arrSize = 0;
unsigned int T1_5; // inter character time out in microseconds
unsigned int T3_5; // frame delay in microseconds
void initBaud(){
	// Modbus states that a baud rate higher than 19200 must use a fixed 750 us
  // for inter character time out and 1.75 ms for a frame delay.
  // For baud rates below 19200 the timeing is more critical and has to be calculated.
  // E.g. 9600 baud in a 10 bit packet is 960 characters per second
  // In milliseconds this will be 960characters per 1000ms. So for 1 character
  // 1000ms/960characters is 1.04167ms per character and finaly modbus states an
  // intercharacter must be 1.5T or 1.5 times longer than a normal character and thus
  // 1.5T = 1.04167ms * 1.5 = 1.5625ms. A frame delay is 3.5T.

	if (baud > 19200)
	{
		T1_5 = 750;
		T3_5 = 1750;
	}
	else
	{
		T1_5 = 15000000/baud; // 1T * 1.5 = T1.5
		T3_5 = 35000000/baud; // 1T * 3.5 = T3.5
	}

}

void dataReceived(byte cmd, ByteBuffer *buf){
//#if DEBUG
//	Tr("data:");
//	Trln(cmd);
//#endif
	if(cmd==CMD_POSTING_INTERVAL){
		zn.postingInterval = buf->getRUInt32();
		savePostingInt();
		zn.writeLog(0,1,"postingInterval:%i",zn.postingInterval);
	}else if(cmd==CMD_WDT_SLEEP){
		wdt_8sec = buf->getIntE();
		save8SInt();
		zn.writeLog(0,1,"wdt_8sec:%i",wdt_8sec);
	}else if(cmd==CMD_BAUD){
		int tt = buf->getIntE();
		if(tt>0){
			baud = tt;
			NeoSerial.flush(); // wait for last transmitted data to be sent
			NeoSerial.begin(baud);
			saveBaud();
			initBaud();
		}
		zn.writeLog(0,1,"baud tt:%i",tt);
	}else if(cmd==CONN_ACK){
		cack();
	}else if(cmd==CMD_TRIG_OUT){
		byte idx = buf->get();
		uint8_t val = buf->get();
		if(idx<OUT_COUNT){
			uint8_t pin = out_pins[idx];
			uint8_t cur = digitalRead(pin);
			zn.writeLog(0,1,"setPin #:%i val:%i  idx:%i cur%i",pin,val, idx, cur);
			if(cur!=val){
				digitalWrite(pin,val);
				if(val){
					bitSet(out_s, idx);
				}else{
					bitClear(out_s, idx);
				}
			}
		}else{
			zn.writeLog(0,1,"ERR out idx:%i max:%i",idx,OUT_COUNT);
		}
	}else if(cmd==CMD_INT_TYPE){
		byte idx = buf->get();
		if(idx<IN_COUNT){
			in_type[idx]=buf->get();
			if(in_type[idx]==COUNTER){
//#if DEBUG
//	Trln("counter..");
//#endif
				attachPinChangeInterrupt(idx, FALLING);
				pinMode(in_pin[idx],INPUT_PULLUP);
			}
			saveInType();
			if(buf->getLenght()>0){
				in_res[idx]=buf->getInt();
				saveRes();
			}
			zn.writeLog(0,1,"in type idx:%i val:%i res:%i",idx,in_type[idx],in_res[idx]);
		}else{
			zn.writeLog(0,1,"ERR in type idx:%i max:%i",idx,IN_COUNT);
		}
	}else if(cmd==CMD_TRIG){
		//typedef struct{
		//	uint8_t input;
		//	uint8_t oper;
		//	uint8_t out;
		//	uint8_t out_dir;
		//	int val;
		//} trigger_t;//size 6
		byte idx = buf->get();
		trigger[idx].input = buf->get();
		trigger[idx].oper = buf->get();
		trigger[idx].out = buf->get();
		trigger[idx].out_dir = buf->get();
		trigger[idx].val = buf->getInt();
		if(idx<TRIG_COUNT && trigger[idx].input<READ_COUNT){
			saveTrig();
			zn.writeLog(0,1,"trig idx:%i input:%i oper:%i out:%i out_dir:%i val:%i",idx,trigger[idx].input,trigger[idx].oper ,trigger[idx].out,trigger[idx].out_dir,trigger[idx].val);
		}else{
			zn.writeLog(0,1,"ERR ttrig idx:%i max:%i",idx,TRIG_COUNT);
		}
	}else if(cmd==CMD_TRIG_ALL){
		for(int idx=0;idx<TRIG_COUNT;idx++){
			trigger[idx].input = buf->get();
			trigger[idx].oper = buf->get();
			trigger[idx].out = buf->get();
			trigger[idx].out_dir = buf->get();
			trigger[idx].val = buf->getInt();
		}
		//EEPROM_writeAnything(e_trigger,trig);
	}else if(cmd==ACK){
		//ul_PreviousMillis = millis();
	}else if(cmd==SUBDEV){
		sdSessid = buf->getInt();
		sdMsgId = buf->getInt();
		//read all aout
		while (NeoSerial.read() != -1);
		PORTC |= (1 << PINC6); //HIGH
		while(buf->getLenght()>0){
			NeoSerial.write(buf->get());
		}
		NeoSerial.flush();
		// allow a frame delay to indicate end of transmission
		delayMicroseconds(T3_5);
		PORTC &= (~(1 << PINC6)); //LOW
		//read response
		arrSize=0;
		int lastSize=0;
		delay(100);
		u32StartTime = millis();

		unsigned char overflowFlag = 0;

		while (NeoSerial.available())
		{
				// The maximum number of bytes is limited to the serial buffer size of 128 bytes
				// If more bytes is received than the BUFFER_SIZE the overflow flag will be set and the
				// serial buffer will be red untill all the data is cleared from the receive buffer,
				// while the slave is still responding.
				if (overflowFlag){
					NeoSerial.read();
				}
				else
				{
					if (arrSize == BUFFER_SIZE)
						overflowFlag = 1;

					arr[arrSize] = NeoSerial.read();
					arrSize++;
				}

		    delayMicroseconds(T1_5); // inter character time out
		}
		if (overflowFlag){
			zn.writeLog(0,100,"ERR overflow modbus sdSessid:%i sdMsgId:%i",sdSessid,sdMsgId);
		}
		zn.write(SUBDEV);
		zn.writeInt(sdMsgId);
		zn.writeInt(arrSize+2);
		zn.writeInt(sdSessid);
		zn.write(arr,arrSize);

		return;
	}
	zn.msg(ACK,1);
	zn.write(cmd);
}
static void handleRxChar( uint8_t c )
{
	arr[arrSize]=c;
	arrSize++;
}

char oper[3] = {'=','<','>'};

void setPin(uint8_t i){
	if(trigger[i].out<OUT_COUNT){
		uint8_t pin = out_pins[trigger[i].out];
		uint8_t val = trigger[i].out_dir;
		if(digitalRead(pin)!=val){
			zn.writeLog(0,1,"setPin:%i val:%i",pin,val);
			zn.sendEvent(0,i,"Output changed #:%i state:%i on value:%i",trigger[i].out,val,trigger[i].val);
			digitalWrite(pin,val);
		}
		if(val){
			bitSet(out_s, trigger[i].out);
		}else{
			bitClear(out_s, trigger[i].out);
		}
	}else{
		zn.sendEvent(0,i,"Trigger #:%i on %i %c %i",i,trigger[i].val,oper[trigger[i].oper], readings[trigger[i].input]);
	}
}

void trigger_out() {
	for (uint8_t i = 0; i < TRIG_COUNT; i++) {
		if(trigger[i].oper>0 && trigger[i].oper<255 && trigger[i].input < READ_COUNT){
			if(trigger[i].oper==OPER_EQUAL && trigger[i].val==readings[trigger[i].input] ){
				setPin(i);
			}if(trigger[i].oper==OPER_GREATER && trigger[i].val<readings[trigger[i].input] ){
				setPin(i);
			}if(trigger[i].oper==OPER_LESS && trigger[i].val>readings[trigger[i].input] ){
				setPin(i);
			}
		}
	}
}


// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3435


void read_inputs() {
	digitalWrite(20,LOW); // tur on voltage to inputs
	delay(400);//charge caps
	for (int i = 0; i < IN_COUNT; i++) {
		///=====NTC10K=======================================================
		if(in_type[i]==NTC10K){
			analogReference(DEFAULT);
			int res = in_res[i];
			if(res==0){
				res=BCOEFFICIENT;
			}
			readings[i * 2] = zn.readThermistor(in_pin[i],res);
//#if DEBUG
//	zn.writeLog(0,1,"NTC10k :%i  temp:%i",i,readings[i * 2]);
//#endif
		}else if(in_type[i]==INPUT_DS){
			OneWire oneWire(in_pin[i]);
			DallasTemperature sensor_inhouse(&oneWire);
			sensor_inhouse.begin();
			if(sensor_inhouse.getDeviceCount()){
				sensor_inhouse.requestTemperatures();
				int rez = sensor_inhouse.getTempCByIndex(0)*10;
				if(rez !=-1270){
					readings[i * 2] = rez;
				}
			}

//			DS1820 dsb(in_pin[i]);
//			if(dsb.deviceFamily()){
//				readings[i * 2] =(dsb.getTemperatureHiRes()*10);
//			}

//			DS18B20 dsb(in_pin[i]);
//			float ds=0;
//			int rez = dsb.getTemperature(ds);
//			if(rez==0){
//				readings[i * 2] =ds *10;//(dsb.getTemperatureHiRes()*10);
//			}else{
//#if DEBUG
//	Tr("DS18err:");
//	Trln(rez);
//#endif
//			}

//#if DEBUG
//	zn.writeLog(1,"DS:%i val:%i",in_pin[i],readings[i * 2]);
//#endif
		}else if(in_type[i]==INPUT_TH_11){
			DHT dht(in_pin[i], DHT11);
			dht.begin();
			int t = dht.readTemperature() * 10;
			int h = dht.readHumidity() * 10;
			if(t>0){
				readings[i * 2] = t;
				readings[i * 2+1] = h;
			}
//#if DEBUG
//	zn.writeLog(0,1,"DS:%i humy:%i temp:%i",in_pin[i],readings[i * 2], readings[i * 2+1]);
//#endif
		}else if(in_type[i]==INPUT_TH_22){
			DHT dht(in_pin[i], DHT22);
			dht.begin();
			int t = dht.readTemperature() * 10;
			int h = dht.readHumidity() * 10;
			if(t>0){
				readings[i * 2] = t;
				readings[i * 2+1] = h;
			}
//#if DEBUG
//	zn.debugL("DS:%i humy:%i temp:%i ",in_pin[i],readings[i * 2], readings[i * 2+1]);
//#endif
		}else if(in_type[i]==COUNTER){
		}else if(in_type[i]==VOLTAGE){
			analogReference(INTERNAL2V56);
			//10v - 30k-10k = 2,5v
			//5v - 10k - 10k = 2,5v
			int res = in_res[i];
			if(res==0){
				res=25;
			}
			readings[i * 2]  = zn.readAnalogVolts(in_pin[i],res);
//#if DEBUG
//	zn.writeLog(0,1,"voltage:%i voltage:%i ",in_pin[i], readings[i * 2]);
//#endif
		}
	}
	readings[16] = zn.readVcc();
#if DEBUG
	Tr(readings[16]);
	Trln('v');
#endif

	digitalWrite(20,HIGH); // turn off voltage to inputs
}


void PinChangeInterruptEvent(0)(void) {
	readings[0]++;
}
void PinChangeInterruptEvent(1)(void) {
	readings[2]++;
}
void PinChangeInterruptEvent(2)(void) {
	readings[4]++;
}
void PinChangeInterruptEvent(3)(void) {
	readings[6]++;
}
void PinChangeInterruptEvent(4)(void) {
	readings[8]++;
}
void PinChangeInterruptEvent(5)(void) {
	readings[10]++;
}
void PinChangeInterruptEvent(6)(void) {
	readings[12]++;
}
void PinChangeInterruptEvent(7)(void) {
	readings[14]++;
}


uint32_t lastConnectionTime = 0;

void connAndSent(){
	hasAck = false;
	zn.turnOnGsm();
	//wait for ack
	while(!hasAck){
		zn.ether_loop();
	}
	//wait for config messages
	for(int i=0;i<50;i++){
		delay(100);
		zn.ether_loop();
	}
	read_inputs();
	trigger_out();
	sendBinTrap();
	delay(300);

}

void stopGsm(){
	if(hasAck){
		zn.write(DISCONN);
	}
	zn.stopS();
	zn.gsmOff();
	hasAck = false;
}

void loop() {
#ifdef POWERSAVE
	if(wdt_8sec){
		if(!hasAck){
			connAndSent();
			if(wdt_8sec==0){
				return;
			}
		}
		stopGsm();
		f_wdt=wdt_8sec;
#if DEBUG
	Tr("tsec:");
	Trln(wdt_8sec*8);
#endif
		wdt_reset();
		while(f_wdt>0){
			enterSleep();
		}
	}else{
#endif
#if DEBUG
		//checkDebug();
#endif
		zn.turnOnGsm();
		zn.ether_loop();
		if((millis() - lastConnectionTime > zn.postingInterval)) {
			read_inputs();
			trigger_out();
			if(zn.checkConn()){
				if(hasAck){
					sendBinTrap();
				}
			}else{
				hasAck = false;
			}
			lastConnectionTime = millis();

			//enable this if you want to reset device id
			//if(digitalRead(2)){
			//	zn.resetDevId();
			//}
		}
#ifdef POWERSAVE
	}
#endif
}


void setup() {
 pinMode(20,OUTPUT);
//PC6 output
DDRC |= (1 << PINC6);
PORTC &= (~(1 << PINC6)); //LOW

 //digitalWrite(20,LOW);
 digitalWrite(20,HIGH); // tur off voltage to inputs
 for(int i=0;i<OUT_COUNT;i++){
	  pinMode(out_pins[i], OUTPUT);
	  digitalWrite(out_pins[i],LOW);
  }
 // put your setup code here, to run once:
#if DEBUG
 beginDebug();
 //checkDebug();
#else
 NeoSerial.begin(baud);
 //NeoSerial.attachInterrupt( handleRxChar );
#endif
#ifdef POWERSAVE
  initPowerSave();
#endif
  //zn.resetDevId();
  zn.begin(dataReceived, cack, ack);
  //in seconds

  readConf();
  read_inputs();
  trigger_out();
}


int main(void) {
//	CLKPR = (1<<CLKPCE);
//	 CLKPR = 0x1; // Divide by 2 = 8mhz

	init();

//	uint8_t oldSREG = SREG;
//	noInterrupts();
//	SREG = oldSREG;

	setup();

	while (true) {
		loop();
	}
}
