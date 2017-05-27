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
#pragma region Variables

//#define ENABLE_DEBUG
#define RUN_NOW		11
#define RUN_NIGHT	21
#define STOP	10
#define PUMP_RUN_NOW 111
#define PUMP_RUN_NIGHT 211
#define PUMP_WAIT_NIGHT 210
#define STOP_PUMP	100
#define STATUS_LED A1

SoftwareSerial rf(8, 9); // TX -RX Port

DS3231 RTC;

//uint8_t devRelayAlter = 8; //alternate relay port
uint8_t devRelay = 7;

uint8_t startHour = 22; // pump start hour 0-23 (22:00-06:00 electric price per KW is  %50 off-price  for my country )
uint8_t startMin = 6; // pump start Minute 0-59

/*Pool full time  */
uint8_t workHour = 3; // Working Time (Hour)
uint8_t workMin = 30; // Working Time (Minute) (1-60)

unsigned long currentMillis; // start millis
unsigned long preMillis; //


unsigned long taskDuration; //
bool isPreWork;
bool isEnergyError;
unsigned long  readMem = 0;
uint32_t currentWork = 0;

//data&time vars.

uint8_t hourNow;
uint8_t minNow;
uint8_t dayNow;
uint8_t monthNow;
uint16_t yearNow;
uint8_t secNow;

// date time vars end

uint8_t taskType = 0; // task type RUN_NOW= Run now, RUN_NIGHT =RunNight
uint8_t pumpStatus = 0; // 111=Pump is Runing day task,  PUMP_RUN_NIGHT=Pump is runing for night task
uint8_t commandType = 0;
uint16_t value = 0;
uint16_t ledInterval = 1000; // Led Interval
bool stateOfStatusLed = true;

#pragma endregion

void setup() {
	Serial.begin(9600);
	rf.begin(9600);
	Wire.begin();
	digitalWrite(devRelay, HIGH); //relay close (for No(normally open) connection )
	pinMode(devRelay, OUTPUT);
	pinMode(STATUS_LED, OUTPUT);    // Status PinOut

	RTC.begin();
	Serial.print("RTC IS RUNING: "); Serial.println(RTC.isrunning());

	taskDuration = (calcSec(workHour, workMin)); // Working Time (Hour,Minute) to millis
	//RTC.adjust(DateTime(2017, 05, 25, 21, 58, 15));

	digitalWrite(STATUS_LED, stateOfStatusLed);
	isEnergyError = CheckEnergyFailure();
}

void loop() {
	TimeRefresh();
	digitalWrite(devRelay, HIGH);
	value = CheckMessage();
	pumpStatus = EEPROM.read(20);
	taskType = EEPROM.read(24);

	Serial.print(F("Pump Status : ")); Serial.println(pumpStatus);
	Serial.print(F("Task Type : ")); Serial.println(taskType);
	Serial.print(F("Value : ")); Serial.println(value);

	Serial.println(F("\n#### I N F O #########################"));

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
		SendMessage(STOP_PUMP);
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

	//	Serial.println("RUN_NIGHT-STAGE 1: ");
	NightRunOrganizer();
}

/*
for night Run
*/

void NightRunOrganizer() {
	if ((value == RUN_NIGHT) || (taskType == RUN_NIGHT)) {
		BlinkLed(); // Led Blink

		hourNow = 21;

		//Serial.println("RUN_NIGHT-STAGE 2: ");

		switch (hourNow) {
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
		case 6:
		case 7:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
			SendMessage(RUN_NIGHT);
			//Serial.println("RUN_NIGHT-STAGE 3: waiting for Night Run");
			#ifdef ENABLE_DEBUG
			Serial.println(F(" - Pump is waiting for Night Run"));
			#endif // ENABLE_DEBUG*
			break;

		default:

			break;
		}
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


/*
RunNow()
*/
void RunNow() {
	pumpStatus = EEPROM.read(20);
	taskType = EEPROM.read(24);
	#ifdef ENABLE_DEBUG
	Serial.println(F("RunNow()- STAGE 2: "));
	#endif // ENABLE_DEBUG

	if ((pumpStatus != PUMP_RUN_NIGHT) && (taskType == RUN_NOW)) {
		#ifdef ENABLE_DEBUG
		Serial.println(F("RunNow()-PUMP NIGHT NOT RUN STAGE 3: "));
		#endif // ENABLE_DEBUG

		digitalWrite(devRelay, LOW);

		for (uint8_t i = 0; i < 4; i++) {
			SendMessage(PUMP_RUN_NOW);
			delay(250);
		}
		for (;;) {
			//TimeRefresh();
			SafetyStop();
			BlinkLed();
			
			value = CheckMessage();
			pumpStatus = EEPROM.read(20);
			taskType = EEPROM.read(24);

			Serial.println(F("PUMP_IS_RUNNING-NOW_TIME"));
			Serial.print(F("* Date: "));
			Serial.print(dayNow, DEC);
			Serial.print(F(".")); Serial.print(monthNow, DEC);
			Serial.print(F(".")); Serial.print(yearNow, DEC);
			Serial.print(F(" / "));
			Serial.print(hourNow, DEC);
			Serial.print(F(":")); Serial.print(minNow, DEC);
			Serial.print(F(":")); Serial.println(secNow, DEC);
			EEPROM.write(10, dayNow);
			EEPROM.write(12, monthNow);
			EEPROM.write(14, (yearNow - 2000));
			EEPROM.write(16, hourNow);
			EEPROM.write(18, minNow);

			if ((value == STOP) || (currentWork >= taskDuration)) {
				StopPump();
				Serial.println(F("PUMP_IS_STOPING"));
				EEPROMWritelong(29, 0);
				currentWork = 0;
				isPreWork = true;
				isEnergyError = false;
				Serial.println("****NORMAL CALISMA BITTI***\n");
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


/*
BlinkLed()
Change Led Status  for Tasks
*/
void BlinkLed() {
	if ((value == RUN_NIGHT) || (taskType == RUN_NIGHT)) {
	//	unsigned long currentMil = 0;
	//	unsigned long previousMil = 0;
		

		/* Status Led BEGIN */
		currentMillis = millis();
		if ((unsigned long)(currentMillis - preMillis) >= ledInterval) {
			stateOfStatusLed = !stateOfStatusLed;
			digitalWrite(STATUS_LED, stateOfStatusLed);
			preMillis = millis();
		}
	}
	else {
		digitalWrite(STATUS_LED, HIGH);
	}
	/* Status Led END*/
}


/*
TimeRefresh()
Refresh all time 
*/
void TimeRefresh() {
	DateTime now = RTC.now();
	hourNow = now.hour();
	minNow = now.minute();
	dayNow = now.day();
	monthNow = now.month();
	yearNow = now.year();
	secNow = now.second();
}



/*
NightRun()


*/
void NightRun() {
	#ifdef ENABLE_DEBUG
	Serial.println(F("NightRun()--- STAGE 2: "));
	#endif // ENABLE_DEBUG

	#ifdef ENABLE_DEBUG
	Serial.print(F("Rec: ")); Serial.println(value);
	Serial.print(F("RUN_NIGHT: ")); Serial.println(RUN_NIGHT);
	#endif // ENABLE_DEBUG;

	for (uint8_t i = 0; i < 3; i++) {
		SendMessage(PUMP_RUN_NIGHT);
		delay(250);
	}

	pumpStatus = EEPROM.read(20);
	taskType = EEPROM.read(24);

	if ((pumpStatus == PUMP_RUN_NIGHT)) {
		#ifdef ENABLE_DEBUG
		Serial.println(F("nightRun()-START MINUTE STAGE 4: "));
		#endif // ENABLE_DEBUG

		for (;;) {
			TimeRefresh();
			SafetyStop();
			BlinkLed();
		

			Serial.println(F("PUMP_IS_RUNNING-NIGHT_TIME"));
			value = CheckMessage();

			#ifdef ENABLE_DEBUG
			Serial.println(F("= Pump is Running..."));
			#endif // ENABLE_DEBUG

			digitalWrite(devRelay, LOW);
			Serial.print(F("= Date: "));
			Serial.print(dayNow, DEC);
			Serial.print(F(".")); Serial.print(monthNow, DEC);
			Serial.print(F(".")); Serial.print(yearNow, DEC);
			Serial.print(F(" / "));
			Serial.print(hourNow, DEC);
			Serial.print(F(":")); Serial.print(minNow, DEC);
			Serial.print(F(":")); Serial.println(secNow, DEC);

			EEPROM.write(10, dayNow);
			EEPROM.write(12, monthNow);
			EEPROM.write(14, (yearNow - 2000));
			EEPROM.write(16, hourNow);
			EEPROM.write(18, minNow);
			SendMessage(PUMP_RUN_NIGHT);
			pumpStatus = EEPROM.read(20);
			taskType = EEPROM.read(24);

			if ((value == STOP) || (currentWork >= taskDuration)) {
				StopPump();
				Serial.println(F("PUMP_IS_STOPING"));
				EEPROMWritelong(29, 0);
				currentWork = 0;
				isPreWork = true;
				isEnergyError = false;
				Serial.println("****NORMAL CALISMA BITTI***\n");
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
		/*case PUMP_WAIT_NIGHT:
			rf.write(PUMP_WAIT_NIGHT);

			EEPROM.write(24, RUN_NIGHT); //task type
			break;*/
	case RUN_NOW:
		rf.write(RUN_NOW);
		//pumpStatus = PUMP_RUN_NOW;

		EEPROM.write(20, PUMP_RUN_NOW);
		EEPROM.write(24, RUN_NOW);

		rf.flush();
		break;
	case RUN_NIGHT:

		rf.write(PUMP_WAIT_NIGHT);

		EEPROM.write(20, PUMP_WAIT_NIGHT);// pump status
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

	//h = hh * 3600;
	//m = mm * 60;
	return (h + m);
}

void StopPump() {
	digitalWrite(devRelay, HIGH);
	taskType = 10;
	pumpStatus = 100;
	for (uint8_t i = 0; i < 5; i++) {
		SendMessage(STOP_PUMP);
		delay(500);
	}
	//EEPROM.write(20, STOP_PUMP);
	//EEPROM.write(24, STOP_PUMP);
	#ifdef ENABLE_DEBUG
	Serial.println(F("* Pump does NOT Work..."));
	#endif // ENABLE_DEBUG
}

/**
CheckEnergyFailure()
* For Energy or RF Failure  this function work
* my pool full time (3 hour 30min)
*/
bool CheckEnergyFailure() {
	readMem = EEPROMReadlong(29);
	if (readMem == 0) {
		isPreWork = true;
		isEnergyError = false;
		Serial.println("PREVIOUS_TASK_WAS_SUCCESFULL");
		return false;
	}
	else {
		isPreWork = false;
		Serial.println("PREVIOUS_TASK_NOT_SUCCESFULL");
		return true;
	}
	// return false;
}


/*
CurrentWorkCounter()
Counter
*/
void CurrentWorkCounter() {


	currentMillis = millis();

	if (((unsigned long)(currentMillis - preMillis) >= ledInterval) && (currentWork <= taskDuration)) {
		currentWork++;
		EEPROMWritelong(29, currentWork);
		preMillis = millis();
		#ifdef ENABLE_DEBUG
		Serial.print(F("currentWork:  ")); Serial.println(currentWork);
		#endif // ENABLE_DEBUG
	}
}

/**SafetyStart()
 * For Energy or RF Failure  this function work
 * my pool full time (3 hour 30min)
 *
*/
void SafetyStop() {
	if ((isPreWork)) {
		#ifdef ENABLE_DEBUG
			Serial.println(F("SafetyStart()-NORMAL WORK "));
		#endif // ENABLE_DEBUG
		}

	if (isEnergyError) {
		#ifdef ENABLE_DEBUG
			Serial.println(F("SafetyStart()-ERROR WORK "));
		#endif // ENABLE_DEBUG
		currentWork = readMem;
		Serial.print(F("ERROR-currentWork:  ")); Serial.println(currentWork);
		isEnergyError = false;
	}
	CurrentWorkCounter();
}