.PHONY: all mobileUnit baseStation

mobileUnit:
	g++ src/mobileUnit.cpp src/tun.cpp src/ip.cpp src/transbuf.cpp -lpthread -lrf24 -g
	sudo ./a.out

baseStation:
	g++ src/baseStation.cpp src/tun.cpp src/ip.cpp src/transbuf.cpp -lpthread -lrf24 -g
	sudo ./a.out

tun:
	g++ -c src/tun.cpp -lcap -g tb.o -o tun.o
#	sudo ./tun.o&
#	sudo /sbin/ifconfig lg0 up
#	sudo ip addr add 10.0.0.1/8 dev lg0

ip:
	g++ -c src/ip.cpp -g -o ip.o

trans:
	g++ -c src/transbuf.cpp -g -o tb.o -lpthread

clean:
	rm a.out