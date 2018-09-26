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

#include "ZiinodeGsm.h"
#include "HttpClient.h"
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <NeoHWSerial/NeoHWSerial.h>

#if USE_SSL
	#define PORT 9797
	#define HTTP_PORT 433
#else
	#define PORT 8787
	#define HTTP_PORT 80
#endif

GSM gsmAccess(false);        // GSM access: include a 'true' parameter for debug enabled
GPRS gprsAccess;  // GPRS access

String IMEI = "";

// serial monitor result messages
String errortext = "ERROR";


ZiinodeGsm::ZiinodeGsm(){
	cmd=0;
	zsize=-1;
	cnt = 1;
	notConnected = true;
	_clientBuffer = (ByteBuffer*)malloc(sizeof(ByteBuffer));
	_clientBuffer->init(MAX_SIZE);
	postingInterval = 5000;
	hasAddr = false;
#if USE_SSL
	hasAddr = true;
	strncpy(address,ADDR,16);
	_hasDevId = true;
	strncpy(_devid,APIKEY,8);
#else
	_hasDevId = true;
	strncpy(_devid,APIKEY,8);
//	_hasDevId = false;
//	strncpy(_devid,APITYPE,8);
#endif
}

// messages for serial monitor response
String oktext = "OK";

// APN data
#define GPRS_APN       "data.tele2.lv" // replace your GPRS APN
#define GPRS_LOGIN     "wap"    // replace with your GPRS login
#define GPRS_PASSWORD  "wap" // replace with your GPRS password
#define PINNUMBER ""


boolean ZiinodeGsm::turnOnGsm(){
    if(!getModemState()){
    	notConnected = true;
    }
	if(!notConnected){
    	return true;
    }
#if DEBUG
	Tr("Conn GSM...");
#endif
    while(notConnected)
    {
      if(gsmAccess.begin()==GSM_READY){
#if DEBUG
    	  Tr("Conn GPRS...");
#endif
    	  if (gprsAccess.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY) {
    	     notConnected = false;
    	  }
 	  }
      else{
#if DEBUG
    	  Trln("GSM Not ready");
#endif
    	  delay(1000);
      }
    }

    return true;
}

void ZiinodeGsm::gsmRestart(){
	notConnected = true;
	turnOnGsm();
}

void ZiinodeGsm::gsmOff(){
	notConnected = true;
	gsmAccess.HWoff();
}

int ZiinodeGsm::getModemState(){
	return gsmAccess.getModemState();
}

void ZiinodeGsm::begin(TOnDataHandler handler, TOnConnAckHandler cack, TOnAckHandler ack){
  _onDataHandler = handler;
  _cack = cack;
  _ack = ack;
  if(!_hasDevId){
	if(read_eeprom_byte(0)==55){
		read_eeprom_array(1,_devid,8);
	    _hasDevId = true;
	}else{
		strncpy(_devid,APITYPE,8);
	}
  }
  if(!hasAddr){
	if(read_eeprom_byte(10)==55){
		read_eeprom_array(11,address,16);
		hasAddr = true;
	}
  }
  gsmAccess.init();
  turnOnGsm();
}


void ZiinodeGsm::sendNet(){
//	_client.write(CMD_NET);
//	IPAddress ip =_client.localIP();
//	IPAddress gw =_client.gatewayIP();
//	IPAddress sm =_client.subnetMask();
//	IPAddress dns =_client.dnsIP();
//	net_t net = {{ip[0],ip[1],ip[2],ip[3]},{gw[0],gw[1],gw[2],gw[3]},{sm[0],sm[1],sm[2],sm[3]},{dns[0],dns[1],dns[2],dns[3]}};
//	writeBody((const byte*)&net,16);
}


size_t ZiinodeGsm::write(uint8_t b)
{
    return _client.write(b);
}

size_t ZiinodeGsm::write(const uint8_t *buf, size_t size){
	return _client.write(buf,size);
}

void  ZiinodeGsm::writeInt(int in){
    byte *pointer = (byte *)&in;
    _client.write(pointer[1]);
    _client.write(pointer[0]);
}


void  ZiinodeGsm::writeIntE(int in){
    byte *pointer = (byte *)&in;
    _client.write(pointer[0]);
    _client.write(pointer[1]);
}

void ZiinodeGsm::writeUint32(uint32_t in){
    byte *pointer = (byte *)&in;
    _client.write(pointer[3]);
    _client.write(pointer[2]);
    _client.write(pointer[1]);
    _client.write(pointer[0]);
}

void ZiinodeGsm::writeInt64(int64_t in){
    byte *pointer = (byte *)&in;
    _client.write(pointer[7]);
    _client.write(pointer[6]);
    _client.write(pointer[5]);
    _client.write(pointer[4]);
    _client.write(pointer[3]);
    _client.write(pointer[2]);
    _client.write(pointer[1]);
    _client.write(pointer[0]);
}

char* log_buf;
char lb[255];

//ANNOTATION
void ZiinodeGsm::sendEvent(int64_t time, int code, const char *fmt, ...){
	if(!log_buf){
		log_buf = lb;
	}
	va_list va;
	va_start (va, fmt);
	int size = vsprintf (log_buf, fmt, va);
	va_end (va);
	if(_client.connected()){
		msg(ANNOTATION,size+2+8);
		writeInt64(time);
		writeInt(code);
		//writeInt(size);
		_client.write((const uint8_t *)log_buf,size);
	}
}

void ZiinodeGsm::writeLog(int64_t time, int code, const char *fmt, ...){
	if(!log_buf){
		log_buf = lb;
	}
	va_list va;
	va_start (va, fmt);
	int size = vsprintf (log_buf, fmt, va);
	va_end (va);
	if(_client.connected()){
		msg(LOG,size+2+8);
		writeInt64(time);
		writeInt(code);
		//writeInt(size);
		_client.write((const uint8_t *)log_buf,size);
	}
}


void  ZiinodeGsm::writeBody(uint8_t cmd, const uint8_t *buf, int size) {
	msg(cmd,size);
	_client.write(buf,size);
}

unsigned long ul_LastComm = 0UL;
boolean ZiinodeGsm::checkConn(){
	if(!hasAddr){
		return false;
	}
	if(!_client.connected() || (ul_LastComm!=0 && (millis() - ul_LastComm) > ((postingInterval*3)+10000))){
	 stop();
	}
	return hasAddr;

}
void lastComm(){
	ul_LastComm = millis();
}


int ZiinodeGsm::msg(uint8_t cmd, int size){
	_client.write(cmd);
	int c = ncnt();
	writeInt(c);
	writeInt(size);
	return c;
}
int ZiinodeGsm::ncnt(){
	cnt++;
	return cnt;
}


void  ZiinodeGsm::sendInterval(){
	msg(CMD_POSTING_INTERVAL,4);
	writeUint32(postingInterval);
}

uint8_t ZiinodeGsm::connected(){
	if(!hasAddr){
		return 0;
	}
	return _client && _client.connected();
}


// Number of milliseconds to wait without receiving any data before we give up
const uint16_t kNetworkTimeout = 60000;
// Number of milliseconds to wait if no data is available before trying again
const uint16_t kNetworkDelay = 100;

//const char *apitype = APITYPE;
//const char *apikey = APIKEY;

void ZiinodeGsm::stop(){
#if DEBUG
	Trln("stop..");
#endif
	hasAddr = false;
#if USE_SSL
	hasAddr = true;
#endif
	_client.stop();
	cmd=0;
	zsize=-1;
	_clientBuffer->clear();
}

void ZiinodeGsm::stopS(){
#if DEBUG
	Trln("stop..");
#endif
	_client.stop();
	cmd=0;
	zsize=-1;
	_clientBuffer->clear();
}


void ZiinodeGsm::setDevId(char* did) {
  _hasDevId=true;
  strncpy(_devid,did,8);
#if DEBUG
	Tr("set devid:");
	Trln(_devid);
#endif
  write_eeprom_byte(0, 55);
  write_eeprom_array(1,_devid,8);
}
char response[16];

void ZiinodeGsm::setAddress(char * addr){
  hasAddr = true;
  memset(address, 0, sizeof(address));
  strncpy(address,addr,16);
#if DEBUG
	Tr("set addr:");
	Trln(address);
#endif
  write_eeprom_byte(10, 55);
  write_eeprom_array(11,address,16);
}

void ZiinodeGsm::resetDevId() {
#if DEBUG
	 Trln("reset devid");
#endif
  _hasDevId=false;
  strncpy(_devid,APITYPE,8);
  write_eeprom_byte(0, 255);
  //write_eeprom_array(1,_devid.c_str(),8);
}

unsigned long timeoutStart;

int16_t ZiinodeGsm::getInt(){
	int ret;
    byte *pointer = (byte *)&ret;
	pointer[0] =  _client.read();
	pointer[1] =  _client.read();
	return ret;
}
//AT+QSSLCFG="sslversion",0,3
//AT+QSSLCFG="sslversion",0,3
//AT+QSSLCFG="seclevel",0,0
//AT+QSSLCFG="ciphersuite",0,"0X0035"
//AT+QSSLOPEN=0,0,"www.zerver.io",443,1
//AT+QSSLOPEN=0,0,"107.178.244.34",443,1
//AT+QSSLCLOSE=0
int cfCnt = 0;
void ZiinodeGsm::ether_loop() {
	while(!hasAddr) {
		char path[40];
	    sprintf(path,"/api/v1/node/host/%s%s%s",APITYPE,_devid,PIN);

#if DEBUG
		Trln("[HTTP] begin...");
		Trln(path);
//		Trln(APITYPE);
//		Trln(_devid);
//		Trln(PIN);
#endif
		// start connection and send HTTP header
		int err =0;
		//_client.stop();
		HttpClient http(_client);
		err = http.get("www.zerver.io", HTTP_PORT, path);
		  if (err == 0)
		  {

			err = http.responseStatusCode();
			if (err == 200)
			{
//				#if DEBUG
//					Tr("Got status code:");
//					Trln(err);
//				#endif

			  // Usually you'd check that the response code is 200 or a
			  // similar "success" code (200-299) before carrying on,
			  // but we'll print out whatever response we get

			  err = http.skipResponseHeaders();
			  if (err >= 0)
			  {
		        int bodyLen = http.contentLength();
				#if DEBUG
					Tr("Content length is:");
					Trln(bodyLen);
					Tr("available:");
					Trln(http.available());
				#endif
				cfCnt = 0;
		        // Now we've got to the body, so we can print it out
		        timeoutStart = millis();
		        // Whilst we haven't timed out & haven't reached the end of the body
		        int rec=0;
		        memset(response, 0, sizeof(response));

		        while ( (http.connected() || http.available()) &&
		               ((millis() - timeoutStart) < kNetworkTimeout) )
		        {
		        	if (http.available() && rec<bodyLen) {
		        		while(http.available() && rec<bodyLen){
		        			response[rec]=(char)http.read();
		        			rec++;
		        		}
		        		if(rec == 4 && bodyLen == 4 && response[0]==response[1] && response[1]==response[2] && response[2]==response[3] && response[3]==0){
#if DEBUG
	Trln("wait for reg, got 0.0.0.0 ");
#endif
							http.stop();
							hasAddr = false;
							delay(5000);
							return;
		        		}else if(rec == bodyLen){
#if DEBUG
   Tr("hd:");
   Tr(_hasDevId);
   Tr(" resp:");
   Trln(response);
#endif
							http.stop();
							if(!_hasDevId){
								setDevId(response);
								return;
							}
							setAddress(response);
#if DEBUG
	Trln("con0?");
#endif
							if(_client.connect(address,PORT)){
								ul_LastComm = 0UL;
								write(ENQ);
								write((const byte*)APITYPE,8);
								write((const byte*)_devid,8);
								write((const byte*)PIN,4);
								writeInt(VERSION);
#if DEBUG
Trln("con0OK");
#endif
								return;
							}else{
#if DEBUG
Trln("FAIL..0");
#endif
								cfCnt++;
								if(cfCnt>2){
									hasAddr = false;
									cfCnt = 0;
								}
								gsmRestart();
								return;

							}
#if DEBUG
	Trln("conn timeout");
#endif
						    hasAddr = false;
		        		}
		        	}
		            else
		            {
#if DEBUG
	Trln("delay...");
#endif
						delay(kNetworkDelay);
		            }
		        }
#if DEBUG
	Trln("http read timeout");
#endif
			  }else{
				#if DEBUG
					Tr("Failed to skip response headers");
					Trln(err);
				#endif
			  }
			}else {
			#if DEBUG
				Tr("Getting response failed:");
				Trln(err);
			#endif
			}
		  }
		  else
		  {
			#if DEBUG
				Tr("Connect failed:::");
				Trln(err);
			#endif
			cfCnt++;
		  }
		  if(cfCnt>2){
			  cfCnt=0;
			  gsmRestart();
		  }else{
			  http.stop();
		  }
	}
	if (connected()){

		if(_client.available() > 0){
			if(cmd==0){
				lastComm();
				cmd = _client.read();
				zsize = -1;
				if(cmd==CONN_ACK){
					if( _cack != 0 ){
						_cack();
					}
					cmd = 0;
					return;
				}
			}
			if(cmd==ACK && _client.available() > 1){
				int aa = getInt();
				if( _ack != 0 ){
					_ack(aa);
				}
				cmd = 0;
				return;
			}else if(cmd!=0 && _client.available() > 1 && zsize==-1 && _clientBuffer->getSize()==0){
				zsize = getInt();
				timeoutStart = millis();
				if(zsize>MAX_SIZE){
#if DEBUG
			Tr("cmd:");
			Tr(cmd);
			Tr(" av:");
			Tr(_client.available());
			Tr(" SKIP:");
			Trln(zsize);
			while(_client.available()){
				Tr('|');
				Tr(_client.read());
			}
			Trln();
#endif
					cmd=0;
					zsize=-1;
					_client.flush();
					return;
				}
#if DEBUG
			Tr("cmd:");
			Tr(cmd);
			Tr(" av:");
			Tr(_client.available());
			Tr(" s:");
			Trln(zsize);
#endif
			}
			while(zsize>0 && _client.available() > 0  && _clientBuffer->getCapacity()>0 && ((millis() - timeoutStart) < 2000)) {
				_clientBuffer->put(_client.read());
				zsize--;
			}

			if(zsize>0 && ((millis() - timeoutStart) > 2000)){
#if DEBUG
			Tr("tiemout:");
			Trln(cmd);
#endif
				cmd=0;
				zsize=-1;
				_clientBuffer->clear();
			}
			if(cmd!=0 && zsize==0){
				//packet ready
#if DEBUG
			Tr("packet:");
			Trln(_clientBuffer->getSize());
#endif
				if( _onDataHandler != 0 ){
					_onDataHandler(cmd,_clientBuffer);
				}else{
					//
				}
				_clientBuffer->clear();
				cmd=0;
				zsize=-1;
			}
		}
	}else if(address[0]>0){
#if DEBUG
Trln("con1?");
#endif
		if(_client.connect(address,PORT)){
			ul_LastComm = 0UL;
			write(ENQ);
			write((const byte*)APITYPE,8);
			write((const byte*)_devid,8);
			write((const byte*)PIN,4);
			writeInt(VERSION);
#if DEBUG
Trln("	con1OK");
#endif
		}else{
#if DEBUG
Trln("FAIL..1");
#endif
			cfCnt++;
			if(cfCnt>2){
				cfCnt = 0;
				hasAddr = false;
			}
			gsmRestart();

		}

	}
}


// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// the value of the 'other' resistor
#define SERIESRESISTOR 10000
//========================================

int samples[NUMSAMPLES];

float readAnalog(int adcpin){
	uint8_t i;
	float average;
// take N samples in a row, with a slight delay
	for (i = 0; i < NUMSAMPLES; i++) {
		samples[i] = analogRead(adcpin);
		delay(10);
	}
// average all the samples out
	average = 0;
	for (i = 0; i < NUMSAMPLES; i++) {
		average += samples[i];
	}
	return average /= NUMSAMPLES;
}

int ZiinodeGsm::readThermistor(int adcpin, int bconf) {
	float average = readAnalog(adcpin);
//#if DEBUG
//	Serial.print("Average analog reading ");
//	Serial.println(average);
//#endif
	if (average == 0)
		return 0;
// convert the value to resistance
	average = 1023 / average - 1;
	average = SERIESRESISTOR / average;
//#if DEBUG
//	Serial.print("Thermistor resistance ");
//	Serial.println(average);
//#endif
	float steinhart;
	steinhart = average / THERMISTORNOMINAL; // (R/Ro)
	steinhart = log(steinhart); // ln(R/Ro)
	steinhart /= bconf; // 1/B * ln(R/Ro)
	steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
	steinhart = 1.0 / steinhart; // Invert
	steinhart -= 273.15; // convert to C
//#if DEBUG
//	//Serial.print("Temperature ");
//	Serial.print(steinhart);
//	//Serial.println(" *C");
//#endif
//delay(1000);
	return steinhart * 10;
}


int ZiinodeGsm::readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  int res = 1125300L/result/100; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return res; // Vcc in millivolts
}

//analogReference(INTERNAL2V56);
//10v - 30k-10k = 2,5v
//5v - 10k - 10k = 2,5v
#define NUM_READS 40
float ZiinodeGsm::readAnalogVolts(int sensorpin, int voltage){
   // read multiple values and sort them to take the mode
	int sortedValues[NUM_READS];
   for(int i=0;i<NUM_READS;i++){
     int value = analogRead(sensorpin);
     int j;
     if(value<sortedValues[0] || i==0){
        j=0; //insert at first position
     }
     else{
       for(j=1;j<i;j++){
          if(sortedValues[j-1]<=value && sortedValues[j]>=value){
            // j is insert position
            break;
          }
       }
     }
     for(int k=i;k>j;k--){
       // move all values higher than current reading up one position
       sortedValues[k]=sortedValues[k-1];
     }
     sortedValues[j]=value; //insert current reading
   }
   //return scaled mode of 10 values
   float returnval = 0;
   for(int i=NUM_READS/2-3;i<(NUM_READS/2+3);i++){
     returnval +=sortedValues[i];
   }
   returnval = returnval/6;
   return returnval*voltage/1023;
}
