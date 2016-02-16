all:
	gcc fan_pi_dht11.c -o fan_pi_dht11 -lwiringPi

install:
	cp ./fan_pi_dht11 /usr/local/bin
