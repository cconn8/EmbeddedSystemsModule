/*
 * DST.cpp
 *
 *  Created on: 22 Feb 2021
 *      Author: colm
 */

#include "DST.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

using namespace std;


//Define register addresses
#define SEC_ADDR	0x00
#define MIN_ADDR	0x01
#define HOUR_ADDR	0x02
#define DAY_ADDR	0x03
#define DATE_ADDR	0x04
#define MONTH_ADDR	0x05
#define YEAR_ADDR	0x06
#define TEMP_ADDR_MSB	0x11
#define TEMP_ADDR_LSB	0x12

#define DEV_ADDRESS		0x68  //Set device address in memory


//#Define i2cBUSNumber 1

DST::DST(int i2cBUS, char deviceAddress){

		this->BUSNumber = i2cBUS;
		this->I2CAddress = deviceAddress;
		getTime();


}

int DST::bcdToDec(char b) {
		return (b/16)*10 + (b%16);
}



int DST::getTime() {
		//read full register to a buffer
		//display bcdToDec buffer values at time address

		char dataBuf[MAX_BUF];  //create a buffer to store data from all registers (19 registers in total)
		snprintf(dataBuf, sizeof(dataBuf), "/dev/i2c-1");	//appends "dataBuf" to the bus - from here interactions are with dataBuf

		//Open the bus [file here is just a fail safe test]
		int file;    //**What is the purpose of file here? As an int? if it opens value should be 0 (true).. must be a check

		if((file = open(dataBuf, O_RDWR)) < 0){	  //if((file = open("/dev/i2c-1", O_RDWR)) < 0){ - using dataBuf instead because it should be linked to device addr
			cout<<"Failed to open bus i2c-1 "<<endl;
			return(1);   //returning 1 exits the programme
		}

		if(ioctl(file, I2C_SLAVE, DEV_ADDRESS) < 0){   //file refers to a yes/no here - i.e. if (file) bus is open and return is 0
			cout<<"Failed to connect to the device at " << DEV_ADDRESS << " " << endl;
			return(2);    //Why return 2 here?
		}


		//initialise i2c comms stop/start by sending the first address in write mode
		char buf[1] = {0x00};  //not used for anything other than starting transfer

		if(write(buf, DEV_ADDRESS, 1) != 1){
			cout<<"Failed to reset the register address " << endl;
		}

		int numBytes = I2C_LENGTH;
		int readBytes = read(file, this->buffer, numBytes);
		if(readBytes == -1){
			cout<<"Failure to read byte stream "<<endl;
		}

		this->sec = bcdToDec(dataBuf[SEC_ADDR]);
		this->min = bcdToDec(dataBuf[MIN_ADDR]);
		this->hour = bcdToDec(dataBuf[HOUR_ADDR]);

		cout<<"The time is: " << this->hour << " : " << this->min << " : " << this->sec <<endl;

		return 0;
}


DST::~DST(){
	cout<<"deconstructor called, memory released!"<<endl;
}

int main(){

	DST test(1, 0x68);



	return 0;
}

