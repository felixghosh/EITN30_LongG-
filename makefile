.PHONY: all mobileUnit baseStation

mobileUnit:
	g++ src/mobileUnit.cpp src/tun.cpp -lpthread -lrf24 -g
	sudo ./a.out

baseStation:
	g++ src/baseStation.cpp src/tun.cpp -lpthread -lrf24 -g
	sudo ./a.out

clean:
	rm a.out