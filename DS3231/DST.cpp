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

//#define DEV_ADDRESS		0x68  //Set device address in memory


//#Define i2cBUSNumber 1

DST::DST(int i2cBUS, char deviceAddress){

		this->BUSNumber = i2cBUS;
		this->I2CAddress = deviceAddress;
		getTimeDate();
		setTimeDate(22,22,22);


}

int DST::bcdToDec(char b) {
		return (b/16)*10 + (b%16);
}

INT DST::decToBcd(int val){
  return (int)( (val/10*16) + (val%10) );
}

int DST::getTimeDate() {
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

		if(ioctl(file, I2C_SLAVE, I2CAddress) < 0){   //file refers to a yes/no here - i.e. if (file) bus is open and return is 0
			cout<<"Failed to connect to the device at " << I2CAddress << " " << endl;
			return(2);    //Why return 2 here?
		}


		//initialise i2c comms stop/start by sending the first address in write mode
		char buf[1] = {0x00};  //not used for anything other than starting transfer - set first register to 0x00

		if(write(file, buf, 1) != 1){
			cout<<"Failed to reset the register address " << endl;
		}

		cout<<"Initialised registers to 0x00"<<endl;

		//sets number of bytes the the full length of i2c registers (19)
		int numBytes = I2C_LENGTH;
		int readBytes = read(file, this->buffer, numBytes);
		if(readBytes == -1){
			cout<<"Failure to read byte stream "<<endl;
		}

		cout<<"First register is "<<dataBuf[0] <<endl;

//not reading/converting values properly
		this->sec = bcdToDec(dataBuf[SEC_ADDR]);
		this->min = bcdToDec(dataBuf[MIN_ADDR]);
		this->hour = bcdToDec(dataBuf[HOUR_ADDR]);
		this->day = bcdToDec(dataBuf[DAY_ADDR]);
		this->date = bcdToDec(dataBuf[DATE_ADDR]);
		this->year = bcdToDec(dataBuf[YEAR_ADDR]);

		cout<<"The time is: " << this->hour << ":" << this->min << ":" << this->sec <<endl;
		cout<<"The date is: " << this->day << "/" << this->date << "/" << this->year <<endl;
	
		close(file);

		return 0;
}


int DST::setTimeDate(int hr, int min, int sec){ //, int day, int date, int year

	char dataBuf[MAX_BUF];
	snprintf(dataBuf, sizeof(dataBuf), "/dev/i2c-1");   //append the writeBuffer to the BUS


	int file;
	//open BUS in read/write mode
	if((file = open(dataBuf, O_RDWR)) < 0){
		cout<<"Failed to open bus "<<endl;
		return 1;
	}

	if(ioctl(file, I2C_SLAVE, I2CAddress) < 0){
		cout<<"Failed to connect to sensor at address 0x68"<<endl;
		return(2);
	}

	cout<<"Test decToBcd conversion - Decimal 10 is BCD ["<<decToBcd(10)<<"] "<<endl;


//Not writing properly
	char writeBuf[4];
	writeBuf[0] = {0x00};   //first initialise the first register address before writing the data
	writeBuf[1] = decToBcd(22); //sec
	writeBuf[2] = decToBcd(22); //min
	writeBuf[3] = decToBcd(22); //hour
	// writeBuf[4] = decToBcd(22); //day
	// writeBuf[5] = decToBcd(22); //date
	// writeBuf[6] = decToBcd(22); //month
	// writeBuf[7] = decToBcd(22); //year

	if(write(file, writeBuf, 4) != 4){
		cout<<"Failed to write to registers "<<endl;
		return(5);
	}

	cout<<"Registers set - First register is "<< dataBuf[0]<<endl;

	close(file);
	cout<<"Bus released"<<endl;

	this->getTimeDate();

}

DST::~DST(){
	cout<<"Deconstructor called, memory released!"<<endl;
}

int main(){

	DST test(1, 0x68);

	return 0;
}

