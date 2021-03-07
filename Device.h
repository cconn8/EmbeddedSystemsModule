/*
 * Device.h
 *
 *  Created on: 25 Feb 2021
 *      Author: colm
 */

#ifndef Device_H_
#define Device_H_
#define DS3231_ADDR 0x68
#define REGISTERS 19

#define SEC_ADDR 0x00
#define MIN_ADDR 0x01
#define HOUR_ADDR 0x02
#define DAY_ADDR 0x03
#define DATE_ADDR 0x04
#define MONTH_ADDR 0x05
#define YEAR_ADDR 0x06
#define ALM1_SEC_ADDR 0x07
#define ALM1_MIN_ADDR 0x08
#define ALM1_HR_ADDR 0x09
#define ALM1_DAYDATE_ADDR 0x0A
#define ALM2_MIN_ADDR 0x0B
#define ALM2_HR_ADDR 0x0C
#define ALM2_DAYDATE_ADDR 0x0D
#define CTRL_ADDR 0x0E
#define STATUS_ADDR 0x0F
#define TEMP_MSB_ADDR 0x11
#define TEMP_LSB_ADDR 0x12


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
	virtual int setAlmCtrlBits(int alarm);
	virtual int setCtrlReg();
	virtual int getTime();
	virtual int setTime(int hours, int mins, int secs);
	virtual int setDate(int Day, int Date, int Month, int Year);
	virtual int getTemp();
	virtual int sqTest();
	virtual int readFullBuffer();
	virtual int send(char const* buffer); 			//wraps i2c check/write method to be used throughout
	virtual int alarmTest();
	virtual int shiftXOR(int val, int shift);		//takes original val to be XORd and number of shifts to position a single bit (1) to
	virtual int flushAlmFlags();
	virtual int setAlarm(int alarm);
	virtual ~Device();
};

#endif /* Device_H_ */