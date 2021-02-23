/*
 * DST.h
 *
 *  Created on: 22 Feb 2021
 *      Author: colm
 */

#ifndef DST_H_
#define DST_H_
#define MAX_BUF 19    //Defines how big inBuffer should be -- 19 registers (19bytes)
#define I2C_LENGTH 0x13  //Defines length of i2c - 0x13 is 19 in Hex
//using namespace std;


class DST {

	private:
		int sec;
		int min;
		int hour;
		int BUSNumber;
		int  I2CAddress;
		char buffer[I2C_LENGTH];

	public:
		DST(int i2cBUS, int deviceAddress);

		int bcdToDec(char b);
		virtual int getTime();
		virtual ~DST();
};

#endif /* DST_H_ */
