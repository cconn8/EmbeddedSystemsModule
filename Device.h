/*
 * Device.h
 *
 *  Created on: 25 Feb 2021
 *      Author: colm
 */

#ifndef Device_H_
#define Device_H_
#define DS3231_ADDR 0X68
#define REGISTERS 19

class Device {

private:
	int file, hours, mins, secs, day, month, date, year;
	char buffer[REGISTERS]; //store i2c registers
public:

	Device();
	//virtual char weekDay(int weekday);
	virtual int decToBcd(int b);
	virtual int bcdToDec(char b);
	virtual int openBus();
	virtual int setSlave(char slaveAddress);
	virtual int reset();
	virtual int setCtrlBits(int alarm);
	virtual int setCtrlReg();
	virtual int getTime();
	virtual int setTime(int hours, int mins, int secs);
	virtual int setDate(int Day, int Date, int Month, int Year);
	virtual int getTemp();
	virtual int setAlarm1();
	virtual int sqTest();
	virtual int readFullBuffer();
	virtual int send(char const* buffer); 			//wraps i2c check/write method to be used throughout
	virtual int alarmTest();
	virtual ~Device();
};

#endif /* Device_H_ */