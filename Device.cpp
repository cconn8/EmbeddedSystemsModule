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
	this->secs = 5;
	this->mins = 5;
	this->hours = 5;
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

	cout<<"Seconds Register " << buffer[0] << " :  " << bcdToDec(buffer[0]) << "\n";
	cout<<"Minutes Register " << buffer[1] << " :  " << bcdToDec(buffer[1]) << "\n" ;
	cout<<"Hours Register " << buffer[2] << " :  " << bcdToDec(buffer[2]) << "\n";



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

	char writeBuffer[4];
	writeBuffer[0] = 0x00; //initial address (superficial write (slave address) - blank values
	writeBuffer[1] = 0x01;  //should write 1 to reg 1
	writeBuffer[2] = 0x01;
	writeBuffer[3] = 0x01;

	//initial write to SET
	if(write(file, writeBuffer, 4) != 4){
		cout<<"Failed to write to device .. "<<endl;
		return 5;
	}

	if(read(file, buffer, 19) != 19){
		cout<<"Failed to read in buffer " << endl;
		return 1;
	}

	cout<<"Seconds register " <<  buffer[0] << " :  " << bcdToDec(buffer[1]) <<endl;
	cout<<"Minutes register " << buffer[1] << " :  " << bcdToDec(buffer[2]) <<endl;
	cout<<"Hours register " << buffer[2] << " :  " << bcdToDec(buffer[3]) <<endl;


	close(file);
	return 0;
}
Device::~Device() {
	// TODO Auto-generated destructor stub
}



int main(){

	Device test;
	test.getTime();
	test.setTime();


	return 0;
}
