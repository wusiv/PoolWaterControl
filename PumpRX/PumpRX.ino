/*
* Name:		PumpRX.ino
* Created:	24/07/2016 14:46
* Author:	Yusuf KALSEN
* URL: http://github.com/wusiv

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



#include <SoftwareSerial.h>
#define DEBUG;

SoftwareSerial rf(8, 9);
int value =10;
void setup() {
	Serial.begin(9600);
	rf.begin(9600);
}

void loop() {

	if (rf.available()) {
		value = rf.read();
		rf.flush();
	}

#ifdef DEBUG
	Serial.print("Value : "); Serial.println(value);
#endif // DEBUG


switch (value)
	{
	case 10:
#ifdef DEBUG
		Serial.println(" - Pump is OFF");
#endif // DEBUG

		break;

	case 11:
#ifdef DEBUG
		Serial.println(" - Pump is ON");
#endif // DEBUG
		break;

	case 21:
#ifdef DEBUG
		Serial.println(" - Pump is waiting for Night Run");
#endif // DEBUG
		break;


	default:
		break;
	}
}

