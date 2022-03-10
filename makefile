.PHONY: all mobileUnit baseStation

mobileUnit:
	g++ -O3 -g src/mobileUnit.cpp src/tun.cpp src/ip.cpp src/transbuf.cpp -lpthread -lrf24 
	sudo ./a.out

baseStation:
	g++ -O3 src/baseStation.cpp src/tun.cpp src/ip.cpp src/transbuf.cpp -lpthread -lrf24 -g
	sudo ./a.out

server:
	gcc -O3 src/ftp/server.c -o src/ftp/server -g
	./src/ftp/server

client:
	gcc -O3 src/ftp/client.c -o src/ftp/client -g
	./src/ftp/client

clean:
	rm a.out