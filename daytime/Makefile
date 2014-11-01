CC = clang++
CFLAGS = -Wall

all: daytime.cpp server readline.cpp readline.h
	$(CC) $(CFLAGS) -o daytime daytime.cpp readline.cpp
server: daytimeServer.cpp
	$(CC) $(CFLAGS) -o daytimeServer daytimeServer.cpp
