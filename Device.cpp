/*
 * Device.cpp
 *
 *  Created on: 25 Feb 2021
 *      Author: colm
 */

#include "Device.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

using namespace std;

Device::Device() {

	//irrelevant / just to get rid of squiggly lines
//	this->secs = 5;
//	this->mins = 5;
//	this->hours = 5;
}


int Device::decToBcd(int b){
	return (b/10)*16 + (b/10);
}

int Device::bcdToDec(char b) {
		return (b/16)*10 + (b%16);
}

int Device::getTime(){

	cout<<"\nReading from device..."<<endl;

	char buffer[19]; //register length
	snprintf(buffer, sizeof(buffer),"/dev/i2c-1"); //attaches to bus

	int file;
	if((file = open(buffer, O_RDWR)) < 0){   //file is a value return
		cout<<"Failed to open bus!"<<endl;
		return 1;
	}

	if (ioctl(file, I2C_SLAVE, 0x68) < 0){
		cout<<"Failed to connect with DS3231 at address 0x68"<<endl;
		return 1;
	}

	//set first register in write mode to start
	char setBuffer[1] = { 0x00 };  //address

	if(write(file, setBuffer, 1) != 1){
		cout<<"Failed to set the first register.."<<endl;
		return 2;
	}


	if(read(file, buffer, 19) != 19){
		cout<<"Failed to read from device"<<endl;
		return 2;
	}

//	cout<<"Seconds Register [ " << buffer[0] << " ] :  " << bcdToDec(buffer[0]) << "\n";
//	cout<<"Minutes Register  [ " << buffer[1] << " ] :  " << bcdToDec(buffer[1]) << "\n" ;
//	cout<<"Hours Register  [ " << buffer[2] << " ] :  " << bcdToDec(buffer[2]) << "\n\n";

	cout<<"Time is hr:min:sec"<<endl;
	cout<<"The RTC time is - "<< bcdToDec(buffer[2]) << ":" <<bcdToDec(buffer[1]) << ":" << bcdToDec(buffer[0]) << "\n" <<  endl;

	cout<<"Date is  WeekDay / Date / Year " <<endl;
	cout<<"The RTC date is - "<< bcdToDec(buffer[5]) << "/" <<bcdToDec(buffer[4]) << "/" << bcdToDec(buffer[3]) << "\n" << endl;

	close(file);
	return 0;
};



int Device::setTime(){

	cout<<"\nSetting device values..."<<endl;

	char buffer[19];
	snprintf(buffer, sizeof(buffer), "/dev/i2c-1");

	int file;
	if((file = open(buffer, O_RDWR)) < 0){
		cout<<"Failed to open Bus"<<endl;
		return 1;
	}

	if (ioctl (file, I2C_SLAVE, 0x68) < 0 ){
		cout<<"Failed to connect to DS3231.."<<endl;
		return 1;
	}

	char writeBuffer[7];  
	writeBuffer[0] = 0x00; //initial address (superficial write (slave address) - blank values
	writeBuffer[1] = 0x00; //seconds reg - is in fact register 0 but due to slave set it is bumped to index 1 - write auto incremented
	writeBuffer[2] = 0x00; //mins
	writeBuffer[3] = 0x00; //hour - 7 sets 0 & 3x 1's in highest niblle for 12hr mode and PM
	writeBuffer[4] = 0x01; //day - Friday
	writeBuffer[5] = 0x01; //date
	writeBuffer[6] = 0x01; //month

	//initial write to SET
	if(write(file, writeBuffer, 7) != 7){
		cout<<"Failed to write to device .. "<<endl;
		return 5;
	}


	char restart[1] = {0x00};
	if(write(file, restart, 1) != 1){
	cout<<"failed to reset before reading the set values"<<endl;
	return 2;
	}

	if(read(file, buffer, 19) != 19){
		cout<<"Failed to read in buffer " << endl;
		return 1;
	}

//	cout << "Seconds register [ " <<  buffer[0] << " ] set to :  " << bcdToDec(buffer[0]) <<endl;
//	cout << "Minutes register [ " << buffer[1] << " ] set to :  " << bcdToDec(buffer[1]) <<endl;
//	cout << "Hours register [ " << buffer[2] << " ] set to :  " << bcdToDec(buffer[2]) <<endl;
//	cout << "Day register [ " << buffer[3]  << " ] set to : " << bcdToDec(buffer[3]) << endl;
//	cout << "Date register [ " << buffer[4] << " ] set to : " << bcdToDec(buffer[4]) << endl;
//	cout << "Month register [ " << buffer[5] << " ] set to : " << bcdToDec(buffer[5]) << endl;

	cout << "Time set \nWeekday : "<< bcdToDec(buffer[3]) <<"\nDate : "<< bcdToDec(buffer[4]) << "\nMonth : "<< bcdToDec(buffer[5]) << "\n\nTime in Hrs : Mins : Secs \n"<< bcdToDec(buffer[2]) << " : " << bcdToDec(buffer[1]) << " : " << bcdToDec(buffer[0]) << endl;


//	getTime();

	close(file);
	return 0;
}


int Device::getTemp(){


	char buffer[19];
	snprintf(buffer, 19, "/dev/i2c-1");	

	int file;
	
	if((file = open(buffer, O_RDWR)) < 0){
		cout<<"Failed to open bus"<<endl;
		return 1;
	}


	if(ioctl(file, I2C_SLAVE, 0x68) < 0){
		cout<<"Failed to connect to I2C sensor to read temp.." <<endl;
		return 1;
	}


	char resetBuf[1] = {0x00};
	
	if(write(file, resetBuf, 1) != 1){
		cout<<"Failed to initialise pointer to register 1"<<endl;
		return 2;
	}


	if(read(file, buffer, 19)!=19){
		cout<<"Failed to read in registers " <<endl;
		return 1;
	}


	//create 16bit variable to store temp
	short temp =  buffer[0x11]; //second 8 bit of 16 temp takes MSB of temp
	temp = temp + ((buffer[0x12]>>6)*0.25);
	float temperature = bcdToDec(temp);

	cout<< "The temperature is: " << temperature << " degrees"<<endl;


	close(file);


	return 0;

}


int Device::setAlarm1(){

        char buffer[19];
        snprintf(buffer, sizeof(buffer), "/dev/i2c-1");

        int file;
        if((file = open(buffer, O_RDWR)) < 0){
                cout<<"Failed to open Bus"<<endl;
                return 1;
        }

        if (ioctl (file, I2C_SLAVE, 0x68) < 0 ){
                cout<<"Failed to connect to DS3231.."<<endl;
                return 1;
	}

	/*set alarm clock for 12:30:10 with all 0's in bit 7 (A1Mx bits) and 1 in DY/DT bit
	to trigger alarm when day, hours and minutes match*/
	char writeBuffer[5];
	writeBuffer[0] = 0x07; //sets pointer to first alarm register @ 0x07 
	writeBuffer[1] = 0x10; //A1 secs
	writeBuffer[2] = 0x00; //A1 mins
	writeBuffer[3] = 0x00; //A1 hours (
	writeBuffer[4] = 0x41; //A1 day - 4 (0100) sets DY/DT to 1 making alarm trigger by day)

	if(write(file, writeBuffer, 5) != 5){
		cout<<"Failed to set alarm1 in write"<<endl;
		return 6;
	}

	cout<<"Alarm set!"<<endl;

//	resetPointer(file, 0x00);
	//Reset pointer
	char setPtr[1] = {0x00};
	if(write(file, setPtr, 1)!=1){
		cout<<"Failed to reset pointer"<<endl;
		return 2;
	}

	if(read(file, buffer, 19) != 19){
		cout<<"Failed to read in buffer"<<endl;
		return 1;
	}

	cout<<"Alarm set for -  Day " << bcdToDec((buffer[10]>>6)) << " at " << bcdToDec(buffer[9]) << " : " << bcdToDec(buffer[8]) << " : " << bcdToDec(buffer[7]) <<  endl;

	char setCtrl[3];
	setCtrl[0] = 0x0E;
	setCtrl[1] = 0x05;  // 00000101 sets control reg to all zeros except INTCN & A1IE
	setCtrl[2] = 0x88; // 10001000 clears the alarm flag bits to zero (A1F set to 1 on match)
	if(write(file, setCtrl, 2) != 2){
		cout<<"Failed to set alarm control register .. "<<endl;
		return 3;
	}

	cout<<"Control set"<<endl;

//	close(file);


	return 0;
}

int Device::sqTest(){

	char buffer[19];
	snprintf(buffer, 19, "/dev/i2c-1");


	int file;
	if((file = open(buffer, O_RDWR)) < 0){
		cout<<"Failed to open bus.."<<endl;
		return 1;
	}

	if(ioctl(file, I2C_SLAVE, 0x68) < 0){
		cout<<"Failed to connect to sensor for sqTest.. "<< endl;
		return 1;
	}

	char setPointer[1] = {0x00}; //set pointer to control register
	if(write(file, setPointer, 1) != 1){
		cout<<"Failed to reset registers for sq" <<endl;
		return 2;
	}

	if(read(file, buffer, 19) != 19){
		cout<< "Failed to read registers! " <<endl;
		return 1;
	}


	char ctrl = buffer[0x0E];
	ctrl = ctrl ^ (0x03 << 3);
	ctrl = ctrl ^ (0x01 << 2);

	char resetPointer[2];
	resetPointer[0] = 0x0E;
	resetPointer[1] = ctrl;


	if(write(file, resetPointer, 2) != 2){
		cout<<"Failed to write square values to device"<<endl;
		return 4;
	}

	cout<<"successful write to device"<<endl;

	close(file);

	return 0;


}

Device::~Device(){
}


int main(){

	Device test;
//	test.getTime();
//	test.setTime();
//	test.getTime();
//	test.getTemp();
//	test.sqTest();
	test.setTime();
	test.setAlarm1();
	return 0;
}
