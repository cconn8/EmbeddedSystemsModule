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
//	openBus();
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
	return (b/10)*16 + (b%10);
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

	cout<<"BUS ready!"<<endl;
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
		setCtrl[0] = 0x0E;  					//sets pointer to control register address - values follow
		if(alarm == 1){ setCtrl[1] = 0x05; }  	//00000101 sets control reg to all zeros except INTCN & A1IE (alarm 1)
		else { setCtrl[1] = 0x06; } 			//00000110 sets control reg to all zeros except INTCN & A2IE (alarm 2)
		setCtrl[2] = 0x88; 						//10001000 clears the alarm flag bits to zero (A1F set to 1 on match)
		
		if(write(this->file, setCtrl, 2) != 2){
			cout<<"Failed to set alarm control register .. "<<endl;
			return 3;
		}

		cout<<"Alarm "<< alarm <<" control bits set!"<<endl;
		return 0;
}

int Device::setCtrlReg(){
	
	char set[2];		//set control register to initial values
	set[0] = 0x0e; 		//pointer to control register
	set[1] = 0x1c; 		//ensure register is reset to values on start up

	if(write(this->file, set, 2) != 2){
		cout<<"Failed to reset control register for sqTest!"<<endl;
		return 3;
	}

	return 0;
}

int Device::readFullBuffer(){
	if(read(this->file, this->buffer, REGISTERS) != REGISTERS){
		cout<<"Failed to read registers to buffer!"<<endl;
		return 1;
	}
	return 0;
}

int Device::send(char const* buffer){
	if(write(this->file, buffer, sizeof(buffer)) != sizeof(buffer)){
		cout<<"Failed to write " << sizeof(buffer) << " bytes to device!" <<endl;
		return 1;
	}
	return 0;
}

int Device::getTime(){

	openBus();
	cout<<"\nDevice reads as follows..."<<endl;

	snprintf(this->buffer, sizeof(this->buffer),"/dev/i2c-1"); 	//attaches to bus

	setSlave(DS3231_ADDR); 										//open communication with sensor at address 0x68				
	reset(); 													//set first register in write mode to start

	if(read(this->file, this->buffer, 19) != 19){ 				//read in all registers to memory
		cout<<"Failed to read from device"<<endl;
		return 2;
	}

	cout<<"Time is HR : MIN : SEC"<<endl;
	cout<<"The RTC time is - "<< bcdToDec(this->buffer[2]) << ":" <<bcdToDec(this->buffer[1]) << ":" << bcdToDec(this->buffer[0]) << "\n" <<  endl;

	cout<<"Date is  WeekDay / Date / Year " <<endl;
	cout<<"The RTC date is - "<< bcdToDec(this->buffer[5]) << "/" <<bcdToDec(this->buffer[4]) << "/" << bcdToDec(this->buffer[3]) << "\n" << endl;

	close(this->file);
	cout<<"- Bus Closed! - \n"<<endl;
	return 0;
};



int Device::setTime(int hours, int mins, int secs){

	this->hours = hours;
	this->mins = mins;
	this->secs = secs;

	openBus();  						//method opens connection in SLAVE 
	cout<<"\nSetting time on the RTC..."<<endl;
	snprintf(this->buffer, sizeof(this->buffer), "/dev/i2c-1");
	
	setSlave(DS3231_ADDR); 				//open communication with sensor at address 0x68 in SLAVE mode				
	reset(); 							//set first register in write mode to start

	char writeBuffer[4];  
	writeBuffer[0] = 0x00; 				//initial address (superficial write (slave address) - blank values
	writeBuffer[1] = decToBcd(secs); 	//seconds reg - is in fact register 0 but due to slave set it is bumped to index 1 - write auto incremented
	writeBuffer[2] = decToBcd(mins); 	//mins
	writeBuffer[3] = decToBcd(hours); 	//hour - 7 sets 0 & 3x 1's in highest niblle for 12hr mode and PM

	send(writeBuffer);					//initial write to SET
	reset(); 							//reset in pointer to first register in between read/writes	
	readFullBuffer();					//Read in all registers to memory

	cout << "Time set \nWeekday : "<< bcdToDec(this->buffer[5]) <<"\nDate : "<< bcdToDec(this->buffer[6]) << "\nMonth : "<< bcdToDec(this->buffer[7]) << "\n\nTime in Hrs : Mins : Secs \n"<< bcdToDec(this->buffer[2]) << " : " << bcdToDec(this->buffer[1]) << " : " << bcdToDec(this->buffer[0]) << endl;

	close(this->file);
	cout<<"- Bus Closed! - \n"<<endl;
	return 0;
}


int Device::setDate(int day, int date, int month, int year){

	this->day = day;
	this->date = date;
	this->month = month;
	this->year = year;
	openBus();
	cout<<"\nSetting Date on the RTC..."<<endl;

	snprintf(this->buffer, sizeof(this->buffer), "/dev/i2c-1");
	
	setSlave(DS3231_ADDR); 					//open communication with sensor at address 0x68				
	reset(); 								//superficial (blank) write to reset the pointer to first reg

	char writeBuffer[5];  
	writeBuffer[0] = 0x03; 					//points to address of first reg in use
	writeBuffer[1] = decToBcd(day); 		//day [0x04] - value
	writeBuffer[2] = decToBcd(date); 		//date [0x05]- value
	writeBuffer[3] = decToBcd(month); 		//month [0x06]- value
	writeBuffer[4] = decToBcd(year); 		//Year [0x07]- value
	
	send(writeBuffer);						//initial write to SET	
	reset(); 								//reset in pointer to first register in between read/writes	
	readFullBuffer();   					//Reads 19 registers to memory to be pulled from 

	cout << "Time set \nWeekday : "<< bcdToDec(this->buffer[5]) <<"\nDate : "<< bcdToDec(this->buffer[6]) << "\nMonth : "<< bcdToDec(this->buffer[7]) << "\n\nTime in Hrs : Mins : Secs \n"<< bcdToDec(this->buffer[2]) << " : " << bcdToDec(this->buffer[1]) << " : " << bcdToDec(this->buffer[0]) << endl;

	close(this->file);
	cout<<"- Bus Closed! - \n"<<endl;
	return 0;
}

int Device::getTemp(){

	openBus();
	snprintf(this->buffer, 19, "/dev/i2c-1");	

	setSlave(DS3231_ADDR); 							//open communication with sensor at address 0x68				
	reset(); 										//set first register in write mode to start
	readFullBuffer();								//Reads 19 registers to memory to be pulled from 

	//16 bit temperature var						//creates 16bit variable to store temp
	short temp =  this->buffer[0x11]; 				//16 bits required to manipulate 2 bytes 0x11 & 0x12 for temp
	temp = temp + ((this->buffer[0x12]>>6)*0.25); 	//shifted 6 to right - only 2 MSBs are relevant§
	float temperature = bcdToDec(temp);

	cout<< "The temperature is: " << temperature << "°C"<<endl;

	close(this->file);
	cout<<"- Bus Closed! - \n"<<endl;
	return 0;

}


int Device::setAlarm1(){

	openBus();	
    //snprintf(this->buffer, sizeof(this->buffer), "/dev/i2c-1");

	setSlave(DS3231_ADDR); //open communication with sensor at address 0x68	

	//Set INTCN and A1IE bits to logic one in the control register
	//ensure alarm flags are flushed to 0 in control status register
	setCtrlBits(1);			

	//set alarm clock for 12:30:10 with all 0's in bit 7 (A1Mx bits) and 1 in DY/DT bit
	//to trigger alarm when day, hours and minutes match
	char writeBuffer[5];
	writeBuffer[0] = 0x07; 		//sets pointer to first alarm register @ 0x07 
	writeBuffer[1] = 0x10; 		//A1 secs
	writeBuffer[2] = 0x00; 		//A1 mins
	writeBuffer[3] = 0x00; 		//A1 hours (
	writeBuffer[4] = 0x41; 		//A1 day - 4 (0100) sets DY/DT to 1 making alarm trigger by day)

	send(writeBuffer);			//writes the writeBuffer to the device using i2c write method
	cout<<"Alarm set!"<<endl;
	
	reset();					//Reset pointer to 0x00 in between read/writes
	readFullBuffer();  			//read all registers from device to memory

	//Shift Day bits right 6 
	cout<<"Alarm set for -  Day " << bcdToDec((this->buffer[10]>>6)) << " at " << bcdToDec(this->buffer[9]) << " : " << bcdToDec(this->buffer[8]) << " : " << bcdToDec(this->buffer[7]) <<  endl;

	getTime();
	close(file);
	cout<<"- Bus Closed!\n"<<endl;
	return 0;
}

int Device::alarmTest(){  //triggers alarm once per second

	openBus();
	setSlave(DS3231_ADDR); //open communication with sensor at address 0x68	

	//Set INTCN and A1IE bits to logic one in the control register
	//ensure alarm flags are flushed to 0 in control status register
	setCtrlBits(1);		//takes alarm number as argument
	char writeBuffer[5];
	writeBuffer[0] = 0x07; 				//sets pointer to first alarm register @ 0x07 
	writeBuffer[1] = decToBcd(66); 		//A1 secs - Hex 66 sets 1000 0010 sending 1 to Alarm mask bit in each alarm
	writeBuffer[2] = decToBcd(66); 		//A1 mins - this triggers an alarm per second
	writeBuffer[3] = decToBcd(66);  	//A1 hours 
	writeBuffer[4] = decToBcd(66);  	//A1 day 

	send(writeBuffer);
	close(file);
	return 0;
}


int Device::sqTest(){

	openBus();

	cout<<"Testing square wave"<<endl;
	snprintf(this->buffer, sizeof(this->buffer), "/dev/i2c-1");

	setSlave(DS3231_ADDR); 				//open communication with sensor at address 0x68				
	reset(); 							//set first register in write mode to start
	setCtrlReg();						//ensure control register is reset to original bits (contains write to ensure values in reg are known before set)
	reset();							//reset pointer to 0x00 in between read/writes
	readFullBuffer();  					//read all registers from device to memory

	reset();
	char ctrl = this->buffer[0x0E];		//initial - 0001 1100
	ctrl = ctrl ^ (0x07 << 3);  		//shifts 111 3 to the left and XORs to make buts 3,4,5 [0s]

	char sendBits[2];
	sendBits[0] = 0x0E;  				//point to control register at 0x0e
	sendBits[1] = ctrl;  				//send XOR'd vaues

	send(sendBits);						//uses i2c write to check and send the "sendBits" data
	setCtrlReg();  						//reset control register original values


	cout<<"successful write to device"<<endl;
	
	close(this->file);
	cout<<"- Bus Closed! -\n"<<endl;
	return 0;


}
Device::~Device(){
}


int main(){

	Device test;
//	test.setTime(22,22,22);
//	test.getTemp();
	test.sqTest();
	test.alarmTest();
//	test.setAlarm1();
	return 0;
}
