/*  
 *  fan_pi_dht11 Fan controller for Raspberry Pi with dht11
 *
 *  Copyright (C) 2016 EverPi - everpi[at]tsar[dot]in
 *  blog.everpi.net 
 * 
 *  This file is part of fan_pi_dht11.
 *
 *  fan_pi is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published b
y
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fan_pi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fan_pi_dht11.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <wiringPi.h>

#define HELP \
        "\n  \033[1mFan Controller with dht11 by blog.everpi.net\n\n" \
        "\tUsage: [TEMP_MIN] [TEMP_RANGE] [option]\n" \
        "\t         -d\tdaemon\033[0m\n\n"

#define PWM_MIN 500 // max 1024

int little_atoi(char *par){
	
	if(par[1]){	
		return ( (par[0] - 48)*10 + (par[1] - 48) );
	}else   return (par[0] - 48);
}

int get_dht11_temp();

int main(int argc, char *argv[]){
	
	int temp = 0;
	int pwm = PWM_MIN;	
	int celsius_step = 0;
	int TEMP_MIN = 0;
	int TEMP_RANGE = 0;

        if(argc < 3){
    
            printf("%s",HELP);
            return 0;
    	}

	if(argc == 4 && argv[3][1] == 'd') daemon(0,0);

	errno = 0;	

	TEMP_MIN = little_atoi(argv[1]);
	TEMP_RANGE = little_atoi(argv[2]);

	printf("TEMP_MIN:%d - TEMP_RANGE:%d\n",TEMP_MIN,TEMP_RANGE);

	if(TEMP_MIN >= 85 || 
	   TEMP_RANGE > (85-TEMP_MIN)){
		fprintf(stdout,"Out of range\n");
		return -1;
	} 
	
  	wiringPiSetup();
  	pinMode(1,PWM_OUTPUT);	
	
	setuid(65534);
	setgid(65534);
	
	pwmWrite(1,pwm);	

	celsius_step = ((1024-PWM_MIN)/(TEMP_RANGE));
	
	while(1){
		
		temp = get_dht11_temp();
		
		if(temp >= TEMP_MIN){
		
			pwm = PWM_MIN+(((temp)-TEMP_MIN)*celsius_step);
			
			if(pwm > 1024) pwm = 1024;	

		}else pwm = 0;
                
		pwmWrite(1,pwm);
			
		
                if(argc < 4){
                        printf("Temp: %dC\nPwm: %d\n\n",temp,pwm);
                }
		
		sleep(1);
	
	}

}

int dht11_read(int pin){

    int bits = 42;
    unsigned char data = 0;
    unsigned int dht11_data = 0;
    unsigned int parity = 0;
    int waithigh = 0;
    struct timespec ntime = {.tv_sec = 0,
			     .tv_nsec = 1000L };   

    unsigned int waitvalues[42] = {0};
 
        pinMode(pin,OUTPUT);
        digitalWrite(pin,LOW);
        delay(11);
        digitalWrite(pin,HIGH);
        pinMode(pin,INPUT);    
	

    while(bits){
       
	while(1){
		data = digitalRead(pin);
		
		if(data == 1) break;
		
	}
		
	waithigh = 0;	
	
	while(digitalRead(pin) == 1){
		
		delayMicroseconds(1);
		waithigh++;
		if(waithigh > 5000) break;
	}
	
	waitvalues[bits] = waithigh;
	
	if(waithigh > 20){
		data = 1;
	}else data = 0;
	
	if(bits <= 8) parity = (parity<<1) | data;
	else
	dht11_data = ( (dht11_data<<1) | data );
	
	bits--;	
    }
	
	if(parity == ( ((dht11_data&0xff00)>>8) + 
		        (dht11_data>>24) )	){
	
		
		return dht11_data;
	}else{
		return 0;
	}
 
}

int get_dht11_temp(){
	
	unsigned int data = 0;

	while(!data){

		data = dht11_read(7);	
	}
	
	return ((data&0xff00)>>8);
}
