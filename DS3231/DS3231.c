/*
 ============================================================================
 Name        : c.c
 Author      : Colm Conneely
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#define BUFFER_SIZE 19      //0x00 to 0x12

// the time is in the registers in encoded decimal form
int bcdToDec(char b) { return (b/16)*10 + (b%16); }

int main(){
   int file;

   printf("Starting the DS3231 test application\n");
   if((file=open("/dev/i2c-1", O_RDWR)) < 0){
      perror("failed to open the bus\n");
      return 1;
   }

   if(ioctl(file, I2C_SLAVE, 0x68) < 0){   //0x69 adds write bit to end (01101001)
      perror("Failed to connect to the sensor\n");
      return 1;
   }

	char writeBuffer[1] = {0x00}; //initialise the first register

   if(write(file, writeBuffer, 1)!=1){
      perror("Failed to reset the read address\n");
      return 1;
   }

	if(ioctl(file, I2C_SLAVE, 0x69)<0){  //0x69 should set  write bit (01101001)
		perror("Failed to set write address\n");
		return 1;
	}

	char hourBuffer[2] = {0x22};  //sets index 2 (0x02) to 22

if(write(file, hourBuffer, 1)!=1){
	perror("Failed to write hour register\n");
	return 1;
}

   char buf[BUFFER_SIZE];

   if(read(file, buf, BUFFER_SIZE)!=BUFFER_SIZE){
       perror("Failed to read in the buffer\n");
       return 1;
    }

	printf("The RTC time is %02d:%02d:%02d\n", bcdToDec(buf[2]), bcdToDec(buf[1]), bcdToDec(buf[0]));
//	printf("The bcd value is %02d\n",buf[0x11]);

	close(file);

	return 0;
}

