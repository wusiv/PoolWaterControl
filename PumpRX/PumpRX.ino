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



#include <RCSwitch.h>

RCSwitch rc = RCSwitch();

void setup() {
	Serial.begin(9600);
	rc.enableReceive(0);  
}

void loop() {
	if (rc.available()) {

		int value = rc.getReceivedValue();
		Serial.print("Value :");
		Serial.print(value);
		switch (value)
		{
		case 10:
			Serial.println(" - Pump is OFF");
			break;
		case 11:
			Serial.println(" - Pump is ON");
			break;
		default:
			break;
		}
	}

	rc.resetAvailable();
}
