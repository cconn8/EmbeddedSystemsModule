/*
 * Device.h
 *
 *  Created on: 25 Feb 2021
 *      Author: colm
 */

#ifndef Device_H_
#define Device_H_

class Device {

private:
	int secs, mins, hours;
	char buffer[19]; //store i2c registers
public:

	Device();
	virtual int decToBcd(int b);
	virtual int getTime();
	virtual int setTime();
	virtual int getTemp();
	virtual int bcdToDec(char b);
	virtual int setAlarm1();
	virtual ~Device();
};

#endif /* Device_H_ */
