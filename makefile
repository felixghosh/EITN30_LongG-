.PHONY: all mobileUnit baseStation

mobileUnit: tun.o
	g++ src/mobileUnit.cpp tun.o -lpthread -lrf24 -g
	sudo ./a.out

baseStation: tun.o
	g++ src/baseStation.cpp tun.o -lpthread -lrf24 -g
	sudo ./a.out

tun.o:
	g++ -c src/tun.cpp -lcap -g -o tun.o
#	sudo ./tun.o&
#	sudo /sbin/ifconfig lg0 up
#	sudo ip addr add 10.0.0.1/8 dev lg0
clean:
	rm a.out