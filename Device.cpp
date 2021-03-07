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

	cout<<"Starting communication to DS3231..."<<endl;
//openBus();
//	getTime();

}

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
int Device::setAlmCtrlBits( int alarm){

	if(alarm < 1 || alarm > 2){
		cout<<"You must specify which alarm"<<endl;
		return 1;
	}
		char setCtrl[3];
		setCtrl[0] = 0x0E;  					//sets pointer to control register address - values follow
		if(alarm == 1){ setCtrl[1] = 0x1D; }  	//00011101 sets control reg to all zeros except rs1,rs2, INTCN & A1IE (alarm 1)
			else { setCtrl[1] = 0x06; } 			//00000110 sets control reg to all zeros except INTCN & A2IE (alarm 2)
		setCtrl[2] = 0x88; 						//10001000 clears the alarm flag bits to zero (A1F set to 1 on match)
		
		if(write(this->file, setCtrl, 2) != 2){
			cout<<"Failed to set alarm control register .. "<<endl;
			return 3;
		}
		cout<<"Alarm "<< alarm <<" control bits set!"<<endl;
		return 0;
}

int Device::flushAlmFlags(){
	char flush[2];
	flush[0] = 0x0f;		//address of control status register with Alarm Flags
	flush[1] = 0x88;

	if(write(file, flush, 2) != 2){
		cout<<"Failed to flush flags"<<endl;
		return 1;
	}
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
													//Used for setting higher bits (6,7,8) in certain registers i.e. Aalrm masks
													//without impacting the alarm times
int Device::shiftXOR(int val, int shift){			//takes a value as input and shifts to leeft
	char temp = 0x01;
	val = val ^ (temp<<shift);						//shifts left by shift values
	return val;										//XORs the two 8 bits
}


int Device::getTime(){

	openBus();
	cout<<"\nDevice reads as follows..."<<endl;

	setSlave(DS3231_ADDR); 										//open communication with sensor at address 0x68				
	reset(); 													//set first register in write mode to start
	readFullBuffer();											//reads in all registers to memory

	cout<<"The RTC time in hr : min : sec is"<<endl;
	cout<<"- " << bcdToDec(this->buffer[2]) << ":" <<bcdToDec(this->buffer[1]) << ":" << bcdToDec(this->buffer[0]) << "\n" <<  endl;

	//cout<<"Date is  WeekDay - Date  Year " <<endl;
	cout<<"The RTC date is: "<< endl;
	cout<<"Day : " << bcdToDec(this->buffer[3]) << "\nDate : " << bcdToDec(this->buffer[4]) << "\nMonth : " << bcdToDec(this->buffer[5]) << "\nYear : " << bcdToDec(this->buffer[6]) << endl;

	close(this->file);
	return 0;
};



int Device::setTime(int hours, int mins, int secs){

	this->hours = hours;
	this->mins = mins;
	this->secs = secs;

	openBus();  						//method opens connection in SLAVE 
	cout<<"\nSetting time on the RTC..."<<endl;
	
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

	cout<< "\nTime set [Hrs : Mins : Secs]\n"<< bcdToDec(this->buffer[2]) << " : " << bcdToDec(this->buffer[1]) << " : " << bcdToDec(this->buffer[0]) << endl;

	close(this->file);
	return 0;
}


int Device::setDate(int day, int date, int month, int year){

	this->day = day;
	this->date = date;
	this->month = month;
	this->year = year;
	openBus();
	cout<<"\nSetting Date on the RTC..."<<endl;
	
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
	return 0;
}

int Device::getTemp(){

	openBus();	

	setSlave(DS3231_ADDR); 							//open communication with sensor at address 0x68				
	reset(); 										//set first register in write mode to start
	readFullBuffer();								//Reads 19 registers to memory to be pulled from 

	//16 bit temperature var						//creates 16bit variable to store temp
	short temp =  this->buffer[0x11]; 				//16 bits required to manipulate 2 bytes 0x11 & 0x12 for temp
	temp = temp + ((this->buffer[0x12]>>6)*0.25); 	//shifted 6 to right - only 2 MSBs are relevant§
	float temperature = bcdToDec(temp);

	cout<< "The temperature is: " << temperature << "°C"<<endl;

	close(this->file);
	return 0;

}

int Device::setAlarm(int alarm){

	openBus();
	setSlave(DS3231_ADDR);

	if(alarm == 1){

		//SET ALARM FOR ONCE PER SECOND BY SETTING MASK BITS TO 1 USING shiftXOR method
		//Set INTCN and A1IE bits to logic one in the control register
		//ensure alarm flags are flushed to 0 in control status register
		setAlmCtrlBits(1);		//takes alarm number as argument
		reset();
		char writeBuffer[5];		
		writeBuffer[0] = ALM1_SEC_ADDR;		//sets pointer to first alarm reg 0x07
		writeBuffer[1] = decToBcd(10); 		//A1 secs 
		writeBuffer[2] = decToBcd(30);	 	//A1 mins
		writeBuffer[3] = decToBcd(12); 	 	//A1 hours
		writeBuffer[4] = decToBcd(1); 		//A1 day 

		send(writeBuffer);					//writes the writeBuffer to the device using i2c write method
		reset();							//Reset pointer to 0x00 in between read/writes
		readFullBuffer();  					//read all registers from device to memory

		//Shift Day bits right 6 
		cout<<"Alarm 1 set for -  Day " << bcdToDec((this->buffer[10]>>6)) << " at " << bcdToDec(this->buffer[9]) << " : " << 
		bcdToDec(this->buffer[8]) << " : " << bcdToDec(this->buffer[7]) <<  endl;
		

		for(int i=0; i<10; i++){
			cout<<"Alarm!"<<endl;
			sleep(1);
			flushAlmFlags();					//bring alarm flag back to zero for next match
			sleep(1);
		
		}

	close(file);

	} else if(alarm == 2){

		//SET ALARM FOR ONCE PER MINUTE -ALARM MASKS [1,1,0] - USING shiftXOR to set DY/DT to 1 for Date
		//Set INTCN and A2IE bits to logic one in the control register
		//ensure alarm flags are flushed to 0 in control status register
		setAlmCtrlBits(2);							//takes alarm number as argument
		reset();
		char writeBuffer[4];	
		writeBuffer[0] = ALM2_MIN_ADDR;				//sets pointer to first alarm reg 0x07
		writeBuffer[1] = decToBcd(31); 				//mins
		writeBuffer[2] = decToBcd(12); 				//A1 mins
		writeBuffer[3] = shiftXOR(decToBcd(15), 6); //A1 Date mode enabled by shifting 1 << 6 positions and ^ for high DY/DT
		writeBuffer[4] = 0x41; 						//A1 day - 4 (0100) sets DY/DT to 1 making alarm trigger by day)

		send(writeBuffer);							//writes the writeBuffer to the device using i2c write method
		reset();									//Reset pointer to 0x00 in between read/writes
		readFullBuffer();  							//read all registers from device to memory

		//Shift Day bits right 6 
		cout<<"Alarm 2 set for -  Day " << bcdToDec((this->buffer[13]>>6)) << " at " << bcdToDec(this->buffer[12]) << " hrs : " << 
		bcdToDec(this->buffer[11]) << " mins " << endl;
		
		flushAlmFlags();							//bring alarm flag back to zero for next match
		close(file);
	}

		return 0;
}

int Device::alarmTest(){  							//triggers alarm once per second

	openBus();
	setSlave(DS3231_ADDR); 							//open communication with sensor at address 0x68	
	reset();

	//Set INTCN and A1IE bits to logic one in the control register
	//ensure alarm flags are flushed to 0 in control status register
	setAlmCtrlBits(1);						//takes alarm number as argument
	reset();

		char writeBuffer[5];
		writeBuffer[0] = 0x07; 				//sets pointer to first alarm register @ 0x07 
		writeBuffer[1] = decToBcd(86); 		//A1 secs - Hex 66 sets 1000 0010 sending 1 to Alarm mask bit in each alarm
		writeBuffer[2] = decToBcd(86); 		//A1 mins - this triggers an alarm per second
		writeBuffer[3] = decToBcd(86);  		//A1 hours 
		writeBuffer[4] = decToBcd(86);  		//A1 day 

		send(writeBuffer);
		
		for(int i=0; i<10; i++){			//Tests alarm every second by flushing the flag to see does it trigger
			cout<<"ALARM!"<<endl;
			sleep(1);
			flushAlmFlags();				//returns AF1 and AF2 back to zero LED should come back on seeing as INTCN is high
			sleep(1);						//Once a flag goes high it stays high until flushed again so this is required
		}

	return 0;
}


int Device::sqTest(){

	openBus();
	cout<<"Testing square wave"<<endl;

	setSlave(DS3231_ADDR); 				//open communication with sensor at address 0x68				
	reset(); 							//set first register in write mode to start
	setCtrlReg();						//ensure control register is reset to original bits (contains write to ensure values in reg are known before set)
	reset();							//reset pointer to 0x00 in between read/writes
	readFullBuffer();  					//read all registers from device to memory

	reset();
	char ctrl = this->buffer[0x0E];		//initial - 0001 1100
	ctrl = ctrl ^ (0x07 << 3);  		//shifts 111 3 to the left and XORs to make btts 3,4,5 [0s]

	char sendBits[2];
	sendBits[0] = 0x0E;  				//point to control register at 0x0e
	sendBits[1] = ctrl;  				//send XOR'd vaues

	send(sendBits);						//uses i2c write() to check and send the "sendBits" data
	//setCtrlReg();  					//reset control register original values


	cout<<"successful write to device"<<endl;	
	close(this->file);
	return 0;


}
Device::~Device(){
}


int main(int argc, char* args[]){

	string arg;
	int option;
	Device test;
	
	if(argc == 2){
		arg = args[1];

		if(arg == "Time" || arg == "time"){
			cout<<"1. Get Time/Date\n2. Set Time/Date\nEnter: ";
			cin>>option;

			if(option == 1){
				test.getTime();
			}
			else if(option == 2){
				test.setTime(12, 30, 30);					//Time takes hrs, minutes, seconds
				test.setDate(5, 20, 5, 21);					//Date is day, date, month, year		
			}
			else if(option > 2){ 
				cout<<"Invalid entry"<<endl;
				return 1;
			}
			return 0;
		} 
		
		if(arg == "Temperature" || arg == "temperature" || arg == "Temp" || arg == "temp"){
			test.getTemp();
			return 0;
		} 


		if(arg == "Alarm" || arg == "alarm"){
			cout<<"1. Set Alarm 1\n2. Set Alarm 2\nEnter: ";
			cin>>option;

			if(option == 1){
				test.setAlarm(1);
			}
			else if(option == 2){
				test.setAlarm(2);				
			}
			else if(option > 2){ 
				cout<<"Invalid entry"<<endl;
				return 1;
			}
			return 0;
		} 
		
		if(arg == "Test" || "test") {
			cout<<"1. Square Wave Test \n2. Alarm Interrupt Test \nEnter: ";
			cin>>option;

			if(option == 1) {
				test.sqTest();
			}
			else if(option == 2){
				test.alarmTest();
			}
			else if(option > 2){ 
				cout<<"Invalid entry"<<endl;
				return 1;
			}
			return 0;
		}
	} 
	else cout<<"Invalid argument(s) \nArguments are 1. [Time] 2. [Temp], 3. [Alarm] 4. [Test]"<<endl;
	return 1;

}

	// //if(args[0] == "Time" || args[0] == "time"){
	// 	Device test;
	// 	test.getTime();
	// //}

	// return 0;

// 	Device test;
// 	test.setTime(0,0,0);		//setTime takes args [Hrs, Mins, Secs]
//	test.getTemp();
//	test.setAlarm1();			//testing alarm values with masks (1) in AM1 - A1M3 and 0 in A1M4 to trigger on seconds match
//	test.alarmTest();
//	test.sqTest();				//sqTest sets RS1 & RS2 bits to 1 - setting frequency to 1Hz (once per second!) (use enums as args to make better)
//	test.setAlarm1();
// 	test.setAlarm(1);
// 	sleep(20);
// 	test.setAlarm(2);
//return 0;	
//}
