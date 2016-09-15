/*
* Name:		PumpRX.ino
* Created:	24/07/2016 14:46
* Author:	Yusuf KALSEN
* URL: http://github.com/wusiv
*
* This project for my little Walnut orchard.
*
* Electric Price  ;
* NORMAL(Kw/$)		:0,1452 $
* EXPENSIVE(Kw/$)	:0,2179 $ (+ 50% than Normal)
* OFF-PRICE (Kw/$)	:0,0927 $ (- 36% than Normal )
*
* 06:00 - 16:59 - is NORMAL
* 17:00 - 21:59 - is EXPENSIVE
* 22:00 - 05:59 - is OFF-PRICE
*
*
*
*
* Arduino Pool Water Level is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Arduino Pool Water Level is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with Arduino Pool Water Level. If not, see <http://www.gnu.org/licenses/>.
*
*
*
*
*
*/



#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>



//#define ENABLE_DEBUG
#define RUN_NOW		11
#define RUN_NIGHT	21
#define STOP	10
#define PUMP_RUN_NOW 111
#define PUMP_RUN_NIGHT 211
#define STOP_PUMP	100


SoftwareSerial rf(8, 9); // TX -RX Port



RTC_DS1307 RTC;

//uint8_t devRelayAlter = 8; //alternate relay port
uint8_t devRelay = 7;


uint8_t startHour = 22; // pump start hour 0-23 (22:00-06:00 electric price per KW is  %50 off-price  for my country )
uint8_t startMin = 6; // pump start Minute 0-59 

/*Pool full time  */
uint8_t workHour = 0; // Working Time (Hour)
uint8_t workMin = 1; // Working Time (Minute) (1-60)

#ifdef ENABLE_DEBUG
unsigned long milStart; // start millis
unsigned long taskStopTime; // 
unsigned long taskDuration; //
#endif // ENABLE_DEBUG



//data&time vars.

uint8_t hourNow;
uint8_t minNow;
uint8_t dayNow;
uint8_t monthNow;
uint16_t yearNow;
uint8_t secNow;


// date time vars end



uint8_t taskType = 0; // task type RUN_NOW= Run now, RUN_NIGHT =RunNight
uint8_t pumpStatus = 0; // 111=Pump is Runing PUMP_RUN_NIGHT=Pump is runing for night
uint8_t commandType = 0;
uint16_t value = 0;





void setup() {
	Serial.begin(9600);
	rf.begin(9600);
	Wire.begin();
	digitalWrite(devRelay, HIGH); //relay close (for No(normally open) connection )
	pinMode(devRelay, OUTPUT);
	RTC.begin();
	//taskDuration = (calcSec(workHour, workMin) * 1000); // Working Time (Hour,Minute) to millis
	//RTC.adjust(DateTime(2016, 07, 16, 21, 17, 40));
}

void loop() {
	digitalWrite(devRelay, HIGH);
	value = CheckMessage();
	pumpStatus = EEPROM.read(20);
	taskType = EEPROM.read(24);

	Serial.print(F("Pump Status : ")); Serial.println(pumpStatus);
	Serial.print(F("Task Type : ")); Serial.println(taskType);
	Serial.print(F("Value : ")); Serial.println(value);

	DateTime now = RTC.now();

	hourNow = now.hour();
	minNow = now.minute();
	dayNow = now.day();
	monthNow = now.month();
	yearNow = now.year();
	secNow = now.second();

	Serial.println(F("\n#### I N F O #########################"));
	#ifdef ENABLE_DEBUG
		
		Serial.print(F("# lastMillis: "));
		Serial.println(EEPROMReadlong(1));
		Serial.print(F("# TaskStopTime: "));
		Serial.println(EEPROMReadlong(5));
	#endif // ENABLE_DEBUG


	Serial.print(F("# Last work EndTime: "));
	Serial.print(EEPROM.read(10)); Serial.print(F("-")); Serial.print(EEPROM.read(12)); Serial.print(F("-")); Serial.print(EEPROM.read(14));
	Serial.print(F(" / ")); Serial.print(EEPROM.read(16)); Serial.print(F(":")); Serial.println(EEPROM.read(18));
	Serial.println(F("######################################"));
	Serial.println(F("******* D E B U G *************"));
	Serial.print(F("* Date: "));
	Serial.print(dayNow, DEC);
	Serial.print(F(".")); Serial.print(monthNow, DEC);
	Serial.print(F(".")); Serial.print(yearNow, DEC);
	Serial.print(F(" / "));
	Serial.print(hourNow, DEC);
	Serial.print(F(":")); Serial.print(minNow, DEC);
	Serial.print(F(":")); Serial.println(secNow, DEC);

	#ifdef ENABLE_DEBUG
		Serial.print(F("* taskDuration: "));
		Serial.println(taskDuration);
	#endif // ENABLE_DEBUG


	Serial.print(F("* Current Millis: "));
	Serial.println(millis());
	Serial.println(F("******************************\n"));





	if ((value == STOP) || (taskType == STOP)) {

	
		SendMessage(STOP);
		StopPump();
		#ifdef ENABLE_DEBUG
				Serial.println(F(" - Pump is OFF"));
		#endif // ENABLE_DEBUG

	}


	if ((value == RUN_NOW) || (taskType == RUN_NOW)) {

		SendMessage(RUN_NOW);

		
		RunNow();
		#ifdef ENABLE_DEBUG
			Serial.println(F("Loop RUN_NOW STAGE 1: "));
		#endif // ENABLE_DEBUG
	}



	if ((value == RUN_NIGHT) || (taskType == RUN_NIGHT)) {

		SendMessage(RUN_NIGHT);
		
	
	
		switch (hourNow){

		case 22:
			if (minNow >= 5) {
				NightRun();
				break;
			}
			break;
		case 23:
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			#ifdef ENABLE_DEBUG
				Serial.println(F("Loop HOUR IS TRUE STAGE 1: "));
			#endif // ENABLE_DEBUG
			NightRun();
			break;
		case 5:
			if (minNow >= 5) {
				break;
			}
			break;

		default:
			break;
		}

		#ifdef ENABLE_DEBUG
				Serial.println(F(" - Pump is waiting for Night Run"));
		#endif // ENABLE_DEBUG
	}


}

/*
CheckMessage()

Receiving Messages from Pool 
*/


uint16_t CheckMessage() {

	if (rf.available()) {
		uint16_t value = rf.read();
		rf.flush();
		return value;
	}
	return 0;
}

void RunNow() {
	pumpStatus = EEPROM.read(20);
	taskType = EEPROM.read(24);
	#ifdef ENABLE_DEBUG
		Serial.println(F("RunNow()- STAGE 2: "));
	#endif // ENABLE_DEBUG
	
	
	if ((pumpStatus != PUMP_RUN_NIGHT)&&(taskType==RUN_NOW)) {
		#ifdef ENABLE_DEBUG
				Serial.println(F("RunNow()-PUMP NIGHT NOT RUN STAGE 3: "));
		#endif // ENABLE_DEBUG

				
		digitalWrite(devRelay, LOW);
		for (uint8_t i = 0; i < 4; i++) {
			SendMessage(PUMP_RUN_NOW);
			delay(500);
		}
		for (;;) {
			value = CheckMessage();
			if (value == STOP) {
				SendMessage(STOP);
				break;
			}
		}
	}
	else {

		StopPump();
		#ifdef ENABLE_DEBUG
				Serial.print(F("Rec: ")); Serial.println(value);
				Serial.println(F("RunNow()-Else{} "));
		#endif // ENABLE_DEBUG
	}
}


void NightRun() {
	#ifdef ENABLE_DEBUG
		Serial.println(F("NightRun()--- STAGE 2: "));
	#endif // ENABLE_DEBUG

		
	#ifdef ENABLE_DEBUG
		Serial.print(F("Rec: ")); Serial.println(value);
		Serial.print(("RUN_NIGHT: ")); Serial.println(RUN_NIGHT);
	#endif // ENABLE_DEBUG;

	DateTime now = RTC.now();
	for (uint8_t i = 0; i < 5; i++) {
		SendMessage(PUMP_RUN_NIGHT);
	}
	
	pumpStatus = EEPROM.read(20);
	taskType = EEPROM.read(24);

	/*if ((now.hour() == startHour) || (pumpStatus == PUMP_RUN_NIGHT)) {
		Serial.println("nightRun()-START HOUR STAGE 3: ");
		milStart = (millis()); //First Run (millisecond)
		taskStopTime = milStart + taskDuration; // task End time calculation
		*/
		if (pumpStatus == PUMP_RUN_NIGHT) {
			#ifdef ENABLE_DEBUG
						Serial.println(F("nightRun()-START MINUTE STAGE 4: "));
			#endif // ENABLE_DEBUG
						
			
			for (;;) {
				hourNow = now.hour();
				minNow = now.minute();
				dayNow = now.day();
				monthNow = now.month();
				yearNow = now.year();
				secNow = now.second();
				delay(500);

				#ifdef ENABLE_DEBUG
					Serial.println(F("========== D E B U G ================="));
					Serial.print(F("= MilStart: "));
					Serial.println(milStart);
					Serial.print(F("= Current Millis: "));
					Serial.println(millis());
					Serial.print(F("= taskStopTime: "));
					Serial.println(taskStopTime);

				#endif // ENABLE_DEBUG

				value = CheckMessage();
				if (value != STOP) {
					#ifdef ENABLE_DEBUG

							Serial.println(F("= Pump is Running..."));
					#endif // ENABLE_DEBUG

					digitalWrite(devRelay, LOW);

					#ifdef ENABLE_DEBUG
						EEPROMWritelong(1, millis());
						EEPROMWritelong(5, taskStopTime);
					#endif // ENABLE_DEBUG


					EEPROM.write(10, dayNow);
					EEPROM.write(12, monthNow);
					EEPROM.write(14, (yearNow - 2000));
					EEPROM.write(16, hourNow);
					EEPROM.write(18, minNow);
					SendMessage(PUMP_RUN_NIGHT);
					

					}


				else {

					StopPump();
					break;

				}
			}
		}

}


/*
This function will write a 4 byte (32bit) long to the eeprom at
the specified address to address + 3.
http://playground.arduino.cc/Code/EEPROMReadWriteLong
*/
void EEPROMWritelong(int address, long value)
{
	//Decomposition from a long to 4 bytes by using bitshift.
	//One = Most significant -> Four = Least significant byte
	byte four = (value & 0xFF);
	byte three = ((value >> 8) & 0xFF);
	byte two = ((value >> 16) & 0xFF);
	byte one = ((value >> 24) & 0xFF);
	//Write the 4 bytes into the eeprom memory.
	EEPROM.write(address, four);
	EEPROM.write(address + 1, three);
	EEPROM.write(address + 2, two);
	EEPROM.write(address + 3, one);
}
long EEPROMReadlong(long address)
{
	//Read the 4 bytes from the eeprom memory.
	long four = EEPROM.read(address);
	long three = EEPROM.read(address + 1);
	long two = EEPROM.read(address + 2);
	long one = EEPROM.read(address + 3);
	//Return the recomposed long by using bitshift.
	return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}


/*
 SendMessage()
 Sending Messages to Pool
*/
void SendMessage(uint8_t msg) {
	switch (msg)
	{
	case RUN_NOW:
		rf.write(RUN_NOW);
		//pumpStatus = PUMP_RUN_NOW;

		EEPROM.write(20, PUMP_RUN_NOW);
		EEPROM.write(24, RUN_NOW);


		rf.flush();
		break;
	case RUN_NIGHT:

		rf.write(RUN_NIGHT);


		EEPROM.write(24, RUN_NIGHT); //task type

		rf.flush();
		break;
	case PUMP_RUN_NIGHT:
		rf.write(PUMP_RUN_NIGHT); // pump is now runing for night task
		//pumpStatus = PUMP_RUN_NIGHT;

		EEPROM.write(20, PUMP_RUN_NIGHT);// pump status
		EEPROM.write(24, RUN_NIGHT); //task type


		rf.flush();
		break;
	case PUMP_RUN_NOW:
		rf.write(PUMP_RUN_NOW);
		//pumpStatus = PUMP_RUN_NOW;

		EEPROM.write(20, PUMP_RUN_NOW);
		EEPROM.write(24, RUN_NOW);


		rf.flush();
		break;
	case STOP_PUMP:
		rf.write(100);

		EEPROM.write(20, 100);
		EEPROM.write(24, 10);

		break;

	case STOP:
		rf.write(10);
		EEPROM.write(20, 100);
		EEPROM.write(24, 10);
		break;
	default:
		break;
	}
}

/**
Calc second
@INPUT: Hour,Minute (integer)
@OUT: second
*/

/*
long calcSec(uint8_t hh, uint8_t mm) {
	int m;
	int h;
	if ((hh > 0) && (hh <= 24)) {
		h = hh * 3600;
	}
	else {
		h = 0;
	}
	if ((mm >= 0) && (mm <= 59)) {
		m = mm * 60;
	}
	else {
		m = 0;
	}

	h = hh * 3600;
	m = mm * 60;
	return (h + m);
}*/

void StopPump() {
	digitalWrite(devRelay, HIGH);

	for (uint8_t i = 0; i < 5; i++) {
		SendMessage(100);
		delay(500);
	}
	EEPROM.write(20, 255);
	EEPROM.write(24, 255);
	#ifdef ENABLE_DEBUG
		Serial.println(F("* Pump does NOT Work..."));
	#endif // ENABLE_DEBUG

	
}