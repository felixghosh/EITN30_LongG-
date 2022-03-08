.PHONY: all mobileUnit baseStation

mobileUnit:
	g++ -O3 src/mobileUnit.cpp src/tun.cpp src/ip.cpp src/transbuf.cpp -lpthread -lrf24 -g
	sudo ./a.out

baseStation:
	g++ -O3 src/baseStation.cpp src/tun.cpp src/ip.cpp src/transbuf.cpp -lpthread -lrf24 -g
	sudo ./a.out

server:
	gcc -O3 -g server.c -o server

client:
	gcc -O3 -g client.c -o client

clean:
	rm a.out