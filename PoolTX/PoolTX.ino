
/*
*
* Name:		PoolTX.ino
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

/* pull down resistor
*  Pull-Down Resistor. 
*/

#include <EEPROM.h>
#include <SoftwareSerial.h>
#define ENABLE_DEBUG
#define RUN_NOW		11
#define RUN_NIGHT	21
#define STOP	10
#define PUMP_RUN_NOW 111
#define PUMP_RUN_NIGHT 211
#define STOP_PUMP 100

uint8_t btnRun = 10;  //start stop buton
uint8_t btnNight = 2; // night start stop button
uint8_t wtrSen = 3;
bool buttonState = false;
uint8_t push = 0;
uint8_t nightPush = 0;
bool nightButtonState = false;
bool isFull = false;
bool runFlag = 0;
bool nightFlag = 0;
uint8_t commandType = 0;
SoftwareSerial rf(8, 9);
uint8_t messageTmp=0;
uint8_t LastMessage=0;
uint8_t pumpStatus = 0;



void setup()
{

	rf.begin(9600);
	Serial.begin(9600);

	pinMode(btnRun, INPUT);
	pinMode(btnNight, INPUT);

	pinMode(A0, OUTPUT);  // Water LED
	pinMode(A1, OUTPUT);  // Led night 
	pinMode(A2, OUTPUT);  // Led RUN
	pinMode(A3, OUTPUT);  // led Power
	pinMode(wtrSen, INPUT);
	digitalWrite(A3, HIGH);

	#ifdef ENABLE_DEBUG;
		Serial.println(EEPROM.read(9));
		Serial.println(isFull);
		Serial.println(EEPROM.read(11));
	#endif // !ENABLE_DEBUG;
	Serial.println(F("Starting...."));

	delay(2000);


	
}

void loop()
{
		messageTmp = checkMessage();

		delay(500);
		switch (messageTmp) {
		case PUMP_RUN_NIGHT:
			EEPROM.write(3, PUMP_RUN_NIGHT);

			Serial.println("case 211");
			break;
		case STOP_PUMP:
			EEPROM.write(3, 100);
			Serial.println("case 100");
			break;
		case PUMP_RUN_NOW:
			EEPROM.write(3, PUMP_RUN_NOW);
			Serial.println("case 111");
			break;
		default:
			break;
		}






		LastMessage = EEPROM.read(3);
	
			if (LastMessage == PUMP_RUN_NOW) {
				#ifdef ENABLE_DEBUG
							Serial.print("LastMessage: "); Serial.println(LastMessage);
							Serial.println("LED HIGH");
				#endif // ENABLE_DEBUG
				digitalWrite(A2, HIGH);
				delay(500);
				digitalWrite(A2, LOW);
				LastMessage = 0;
				delay(500);
			}
			else {
				#ifdef ENABLE_DEBUG
							Serial.print("LastMessage: "); Serial.println(LastMessage);
							Serial.println("LED LOW");
				#endif // ENABLE_DEBUG


				digitalWrite(A2, LOW);

			}
			if (LastMessage == PUMP_RUN_NIGHT) {
				#ifdef ENABLE_DEBUG
							Serial.print("LastMessage: "); Serial.println(LastMessage);
							Serial.println("LED HIGH");
				#endif // ENABLE_DEBUG
				digitalWrite(A2, HIGH);
				delay(500);
				digitalWrite(A2, LOW);
				LastMessage = 0;
				delay(500);
			}
			else {
				#ifdef ENABLE_DEBUG
							Serial.print("LastMessage: "); Serial.println(LastMessage);
							Serial.println("LED LOW");
				#endif // ENABLE_DEBUG
				;
				digitalWrite(A2, LOW);
			}


			if (LastMessage == STOP_PUMP) {
				#ifdef ENABLE_DEBUG
							Serial.print("LastMessage: "); Serial.println(LastMessage);
							Serial.println("Run and Night LED is LOW");
				#endif // ENABLE_DEBUG
				digitalWrite(A2, LOW);
				digitalWrite(A1, LOW);

			}
	

		isFull = digitalRead(wtrSen);
		if (isFull) {
			waterLevel();
		}
		else {
			digitalWrite(A0, LOW);
		}

		buttonState = digitalRead(btnRun);
		if (buttonState) {
			RunNow();
		}

		nightButtonState = digitalRead(btnNight);
		if (nightButtonState) {
			RunNight();
		}



	}



	
		
		
		
/*
 SendCommand();
 Sending Command to Pump
*/	

void SendCommand(uint8_t status) {
	rf.write(status);
	switch (status)
	{
	case RUN_NOW:
	//	EEPROM.write(2, RUN_NOW);
		break;
	case RUN_NIGHT:
		//EEPROM.write(2, RUN_NIGHT);
		break;
	case STOP:
		//EEPROM.write(2, STOP);
		break;
	default:
		break;
	}
	
	#ifdef ENABLE_DEBUG;
		Serial.print(F("command : ")); Serial.println(status); //ENABLE_DEBUG;
		Serial.println("");
	#endif // ENABLE_DEBUG;
	rf.flush();
	Serial.flush();
}

uint8_t checkMessage() {
	int msg = 0;
	if (rf.available()>=1) {
		msg = rf.read();
		return msg;
	}
	else {
		return 0;
	}
	rf.flush();
}


/*
	waterLevel()
	pool water Control 
	
*/

void waterLevel() {
	delay(1000);
	
	#ifdef ENABLE_DEBUG;
		Serial.println(F("=====Water level==========="));//ENABLE_DEBUG;
	#endif // !ENABLE_DEBUG;


	SendCommand(STOP);
	
	digitalWrite(A0, HIGH);

	#ifdef ENABLE_DEBUG;
		Serial.print("= IN night: "); Serial.println(nightFlag);//ENABLE_DEBUG;
		Serial.print("= IN RUN: "); Serial.println(runFlag);//ENABLE_DEBUG;
	#endif // !ENABLE_DEBUG;


	uint8_t tmpRun = EEPROM.read(11);
	if (tmpRun) {
		#ifdef ENABLE_DEBUG;
				Serial.println("= push mod 2"); // ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG;
		

		EEPROM.write(11, 0);
		digitalWrite(A2, LOW); //run
	}
	uint8_t tmpNight = EEPROM.read(9);
	if (tmpNight) {
		#ifdef ENABLE_DEBUG;
				Serial.println("= night push mod 2"); // ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG;
				
		EEPROM.write(9, 0);
		digitalWrite(A2, LOW); // night

	}
		#ifdef ENABLE_DEBUG;

			Serial.print("= Exit RUN: "); Serial.println(runFlag); //ENABLE_DEBUG;
			Serial.print("= Exit night: "); Serial.println(nightFlag); //ENABLE_DEBUG;
			Serial.println("=============================\n"); //ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG;
}
	/*
	
	RunNow()
	Run pump 
	
	*/
void RunNow() {
	if ((!(nightFlag)) && (!(isFull))) {
		#ifdef ENABLE_DEBUG;
				Serial.println("##### BUTTON RUN #####");//ENABLE_DEBUG;
				Serial.print("\n# buttonState :"); Serial.println(buttonState);//ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG;
		delay(300);
		push = EEPROM.read(11);
		if (push >= 1) {
			push = 0;
			EEPROM.write(11, push);
		}
		else {

			push++;
			EEPROM.write(11, push);

		}

		runFlag = (push % 2);

		if (runFlag == 0) { //Close Pump
			digitalWrite(A2, LOW);

			SendCommand(STOP);
			#ifdef ENABLE_DEBUG;
						Serial.print("# Push : "); Serial.print(push); Serial.print(" MOD: "); Serial.println(runFlag);//ENABLE_DEBUG;
			#endif // !ENABLE_DEBUG;

		}

		else { // Run Pump


			SendCommand(RUN_NOW);

			if (LastMessage == PUMP_RUN_NOW) {
				#ifdef ENABLE_DEBUG;
								Serial.print("MessageType "); Serial.print(messageTmp); 
				#endif // !ENABLE_DEBUG;
				digitalWrite(A2, HIGH);

			}
			
			#ifdef ENABLE_DEBUG;
						Serial.print("# Push : "); Serial.print(push); Serial.print(" MOD: "); Serial.println(runFlag);//ENABLE_DEBUG;
			#endif // !ENABLE_DEBUG;

		}
		#ifdef ENABLE_DEBUG;


				Serial.println("#########################\n");//ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG;

	}
	

}


/*
 RunNight()
 Run Pump for night 

*/
void RunNight() {


	if ((!(runFlag)) && (!(isFull))) {
		#ifdef ENABLE_DEBUG;
				Serial.println("\n******night BUTTON*******"); //ENABLE_DEBUG;
				Serial.print("\n* nightButtonState :"); Serial.println(nightButtonState); //ENABLE_DEBUG;
		#endif // !ENABLE_DEBUG; 

		delay(300);
		nightPush = EEPROM.read(9);
			if (nightPush >= 1) {
				nightPush = 0;
				EEPROM.write(9, nightPush);
			}
			else {

				nightPush++;
				EEPROM.write(9, nightPush);
			}
		

		nightFlag = (nightPush % 2);
		
		if (nightFlag == 0) {
			digitalWrite(A2, LOW);

			
				SendCommand(STOP);
			
				

			#ifdef ENABLE_DEBUG;
						//Serial.println((nightPush % 2));
						Serial.print("* nightPush : "); Serial.print(nightPush); Serial.print(" MOD : "); Serial.println(nightFlag); //ENABLE_DEBUG;
			#endif // !ENABLE_DEBUG;


		}
		else {
			
			SendCommand(RUN_NIGHT);
			delay(2000);
			//messageTmp = checkMessage();
			
			Serial.print("messageTmp2: "); Serial.println(messageTmp);
			if ((messageTmp == RUN_NIGHT)) {
				digitalWrite(A2, HIGH);
				
			}

			#ifdef ENABLE_DEBUG;
						Serial.print("* nightPush : "); Serial.print(nightPush); Serial.print(" MOD : "); Serial.println(nightFlag); //ENABLE_DEBUG;
			#endif // !ENABLE_DEBUG;


		}
	#ifdef ENABLE_DEBUG;
			Serial.println("*************************\n"); //ENABLE_DEBUG;
	#endif // ENABLE_DEBUG;



	}




	}

			
