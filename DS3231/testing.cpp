#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <iostream>

#define BUFFER_SIZE 19 //
#define SLAVE_ADDR 0X68

using namespace std;

static int file;

//converts decimal  to (bcd) time to write to registers
uint8_t decToBcd(int val){
	return (uint8_t)((val/10)*16 + (val%10));
}

//converts binary encoded decimal (bcd) time from the registers to normal decimal time
int bcdToDec(char b) { 
	return (b/16)*10 + (b%16); 
}

void setTime(uint8_t secs, uint8_t min, uint8_t hour){
	uint8_t reg[3];
	reg[0] = decToBcd(secs);
	reg[1] = decToBcd(min);
	reg[3] = decToBcd(hour);
	write(file, reg, 3);

	cout<<"Successful write to registers"<<endl;

}

int main(){

   cout<<"Starting the DS3231 test application"<<endl;  
   if((file=open("/dev/i2c-1", O_RDWR)) < 0){  //opens a file object in kernal space VFS to handle interactions - kernal space open takes a pointer to file inode and pointer of file object
      perror("failed to open the bus\n");
      return 1;
   }

   //0X68 is the slave address -  first register is 00h
   if(ioctl(file, I2C_SLAVE, 0x68) < 0){
      perror("Failed to connect to the sensor\n");
      return 1;
   }

   setTime(22,22,22);

return 0;
}

