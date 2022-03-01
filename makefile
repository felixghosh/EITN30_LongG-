.PHONY: all mobileUnit baseStation

mobileUnit:
	g++ src/mobileUnit.cpp -lpthread -lrf24 -g
	sudo ./a.out

baseStation:
	g++ src/baseStation.cpp -lpthread -lrf24 -g
	sudo ./a.out

tun:
	g++ src/tun.cpp -lcap -g
	sudo ./a.out&
	sudo /sbin/ifconfig lg0 up
	sudo ip addr add 10.0.0.1/8 dev lg0
clean:
	rm a.out