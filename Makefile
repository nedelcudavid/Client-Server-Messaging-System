CC = g++
CFLAGS = -Wall -std=c++11

all: subscriber server

subscriber: subscriber.cpp other.cpp
	$(CC) $(CFLAGS) -o subscriber subscriber.cpp other.cpp utils_comm.c

server: server.cpp other.cpp
	$(CC) $(CFLAGS) -o server server.cpp other.cpp utils_comm.c

clean:
	rm -f subscriber server