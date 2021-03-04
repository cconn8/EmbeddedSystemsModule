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

	cout<<"Starting communication to DS3231"<<endl;
	openBus();
	getTime();

}

/*char Device::weekDay(int weekday){
	char day[20];
	switch (day){
		case 1:
		day = "Monday \0";
		break;
		case 2:
		day = "Tuesday \0";
		break;
		case 3:
		day = "Wednesday \0";
		break;
		case 4:
		day = "Thursday\0";
		break;
		case 5:
		day = "Friday\0";
		break;
		case 6:
		day = "Saturday\0";
		break;
		case 7:
		day = "Sunday\0";
		break;
	}

	return day;
}*/

int Device::decToBcd(int b){
	return (b/10)*16 + (b/10);
}

int Device::bcdToDec(char b) {
		return (b/16)*10 + (b%16);
}

int Device::openBus(){

	snprintf(this->buffer, sizeof(this->buffer), "/dev/i2c-1");
	//opens bus in read/write and returns 0 on success [-1 on fail]
	this->file = open(this->buffer, O_RDWR);
	if(file < 0) { 
		cout<<"Failed to open bus!"<<endl;
		return 1;
		}

	cout<<" BUS ready!"<<endl;
	return 0;
}

int Device::setSlave( char slaveAddress){
	if(ioctl(this->file, I2C_SLAVE, DS3231_ADDR) < 0){
		cout<<"Failed to connect to sensor!"<<endl;
		return 1;
	}

	return 0;
}

int Device::reset(){
	char setBuffer[1] = {0x00};
	if(write(this->file, setBuffer, 1) != 1){
		cout<<"Failed to reset address pointer"<<endl;
		return 2;
	}

	return 0;
}

//This ensures INTCN & A1IE/A2IE bits are set to 1
//takes alarm number
int Device::setCtrlBits( int alarm){

	if(alarm < 1 || alarm > 2){
		cout<<"You must specify which alarm"<<endl;
		return 1;
	}

		char setCtrl[3];
		setCtrl[0] = 0x0E;  //sets pointer to control register address - values follow
		if(alarm == 1){
			setCtrl[1] = 0x05;  // 00000101 sets control reg to all zeros except INTCN & A1IE
		} else {
			setCtrl[1] = 0x06;  // 00000110 sets control reg to all zeros except INTCN & A2IE
		}
		setCtrl[2] = 0x88; // 10001000 clears the alarm flag bits to zero (A1F set to 1 on match)
		
		if(write(this->file, setCtrl, 2) != 2){
			cout<<"Failed to set alarm control register .. "<<endl;
			return 3;
		}

		cout<<"Alarm "<< alarm <<" control bits set!"<<endl;
		return 0;
}

int Device::setCtrlReg(){
	//set control register to initial values
	char set[2];
	set[0] = 0x0e; //pointer to control register
	set[1] = 0x1c; //ensure register is reset to values on start up

	if(write(this->file, set, 2) != 2){
		cout<<"Failed to reset control register for sqTest!"<<endl;
		return 3;
	}

	return 0;
}

int Device::getTime(){

	openBus();
	cout<<"\nDevice Reads..."<<endl;

	//char buffer[REGISTERS]; //Size of total number of registers (19)
	snprintf(this->buffer, sizeof(this->buffer),"/dev/i2c-1"); //attaches to bus

	setSlave(DS3231_ADDR); //open communication with sensor at address 0x68				
	reset(); //set first register in write mode to start

	//read in all registers to memory
	if(read(this->file, this->buffer, 19) != 19){
		cout<<"Failed to read from device"<<endl;
		return 2;
	}

	cout<<"Time is HR : MIN : SEC"<<endl;
	cout<<"The RTC time is - "<< bcdToDec(this->buffer[2]) << ":" <<bcdToDec(this->buffer[1]) << ":" << bcdToDec(this->buffer[0]) << "\n" <<  endl;

	cout<<"Date is  WeekDay / Date / Year " <<endl;
	cout<<"The RTC date is - "<< bcdToDec(this->buffer[5]) << "/" <<bcdToDec(this->buffer[4]) << "/" << bcdToDec(this->buffer[3]) << "\n" << endl;

	close(this->file);
	cout<<"\n- Bus Closed! - \n"<<endl;
	return 0;
};



int Device::setTime(){

	openBus();
	cout<<"\nSetting device values..."<<endl;

	//char buffer[REGISTERS];
	snprintf(this->buffer, sizeof(this->buffer), "/dev/i2c-1");
	
	setSlave(DS3231_ADDR); //open communication with sensor at address 0x68				
	reset(); //set first register in write mode to start

	char writeBuffer[7];  
	writeBuffer[0] = 0x00; //initial address (superficial write (slave address) - blank values
	writeBuffer[1] = 0x00; //seconds reg - is in fact register 0 but due to slave set it is bumped to index 1 - write auto incremented
	writeBuffer[2] = 0x00; //mins
	writeBuffer[3] = 0x00; //hour - 7 sets 0 & 3x 1's in highest niblle for 12hr mode and PM
	writeBuffer[4] = 0x01; //day 
	writeBuffer[5] = 0x00; //date
	writeBuffer[6] = 0x00; //month

	//initial write to SET
	if(write(this->file, writeBuffer, 7) != 7){
		cout<<"Failed to write to device .. "<<endl;
		return 5;
	}

	//reset in pointer to first register in between read/writes		
	reset(); 

	if(read(this->file, this->buffer, 19) != 19){
		cout<<"Failed to read in buffer " << endl;
		return 1;
	}

	cout << "Time set \nWeekday : "<< bcdToDec(this->buffer[5]) <<"\nDate : "<< bcdToDec(this->buffer[6]) << "\nMonth : "<< bcdToDec(this->buffer[7]) << "\n\nTime in Hrs : Mins : Secs \n"<< bcdToDec(this->buffer[2]) << " : " << bcdToDec(this->buffer[1]) << " : " << bcdToDec(this->buffer[0]) << endl;

//	getTime();

	close(this->file);
	cout<<"\n- Bus Closed! - \n"<<endl;
	return 0;
}


int Device::getTemp(){

	openBus();
	//char buffer[REGISTERS];
	snprintf(this->buffer, 19, "/dev/i2c-1");	

	setSlave(DS3231_ADDR); //open communication with sensor at address 0x68				
	reset(); //set first register in write mode to start

	if(read(this->file, this->buffer, 19)!=19){
		cout<<"Failed to read in registers " <<endl;
		return 1;
	}

	//create 16bit variable to store temp
	short temp =  this->buffer[0x11]; //16 bits required to add the two 8's togeather
	temp = temp + ((this->buffer[0x12]>>6)*0.25); //shifted 6 to right - only 2 MSBs are relevant
	float temperature = bcdToDec(temp);

	cout<< "The temperature is: " << temperature << "°C"<<endl;

	close(this->file);
	cout<<"\n- Bus Closed! - \n"<<endl;
	return 0;

}


int Device::setAlarm1(){

	openBus();
    	snprintf(this->buffer, sizeof(this->buffer), "/dev/i2c-1");

	setSlave(DS3231_ADDR); //open communication with sensor at address 0x68	

	//Set INTCN and A1IE bits to logic one in the control register
	//ensure alarm flags are flushed to 0 in control status register
	setCtrlBits(1);			

	//set alarm clock for 12:30:10 with all 0's in bit 7 (A1Mx bits) and 1 in DY/DT bit
	//to trigger alarm when day, hours and minutes match
	char writeBuffer[5];
	writeBuffer[0] = 0x07; //sets pointer to first alarm register @ 0x07 
	writeBuffer[1] = 0x10; //A1 secs
	writeBuffer[2] = 0x00; //A1 mins
	writeBuffer[3] = 0x00; //A1 hours (
	writeBuffer[4] = 0x41; //A1 day - 4 (0100) sets DY/DT to 1 making alarm trigger by day)

	if(write(this->file, writeBuffer, 5) != 5){
		cout<<"Failed to set alarm1 in write"<<endl;
		return 6;
	}

	cout<<"Alarm set!"<<endl;

	//Reset pointer to 0x00 in between read/writes
	reset();

	if(read(this->file, this->buffer, 19) != 19){
		cout<<"Failed to read in buffer"<<endl;
		return 1;
	}

	//Shift Day bits right 6 
	cout<<"Alarm set for -  Day " << bcdToDec((this->buffer[10]>>6)) << " at " << bcdToDec(this->buffer[9]) << " : " << bcdToDec(this->buffer[8]) << " : " << bcdToDec(this->buffer[7]) <<  endl;

	getTime();
//	close(file);
//	cout<<"\n- Bus Closed! - \n"<<endl;
	return 0;
}


int Device::sqTest(){

	openBus();
	snprintf(this->buffer, sizeof(this->buffer), "/dev/i2c-1");

	setSlave(DS3231_ADDR); //open communication with sensor at address 0x68				
	reset(); //set first register in write mode to start

	//ensure control register is reset to original bits
	setCtrlReg();  //contains write

	//reset pointer to 0x00 in between read/writes
	reset();

	if(read(this->file, this->buffer, 19) != 19){
		cout<< "Failed to read registers! " <<endl;
		return 1;
	}

	for(int i = 0; i<10; i++){

		reset();

		char ctrl = this->buffer[0x0E];	//initial - 0001 1100
		ctrl = ctrl ^ (0x07 << 3);  //shifts 111 3 to the left and XORs to make buts 3,4,5 [0s]

		char sendBits[2];
		sendBits[0] = 0x0E;  //point to control register at 0x0e
		sendBits[1] = ctrl;  //send XOR'd vaues

		if(write(this->file, sendBits, 2) != 2){
			cout<<"Failed to write square values to device"<<endl;
			return 4;
		}	

		setCtrlReg();  //reset control register original values
	}


	cout<<"successful write to device"<<endl;
	
	close(this->file);
	cout<<"\n- Bus Closed! -\n"<<endl;
	return 0;


}


Device::~Device(){
}


int main(){

	Device test;
	test.setTime();
	test.getTemp();
//	test.sqTest();
	test.setAlarm1();
	return 0;
}
