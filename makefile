.PHONY: all mobileUnit baseStation

mobileUnit:
	g++ -O3 src/mobileUnit.cpp src/tun.cpp src/ip.cpp src/transbuf.cpp -lpthread -lrf24 -g
	sudo ./a.out

baseStation:
	g++ -O3 src/baseStation.cpp src/tun.cpp src/ip.cpp src/transbuf.cpp -lpthread -lrf24 -g
	sudo ./a.out

server:
	gcc -O3 -g src/ftp/server.c -o server -lpthread

client:
	gcc -O3 -g src/ftp/client.c -o client

clean:
	rm a.out