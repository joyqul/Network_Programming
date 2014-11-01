CC = clang++
CFLAGS = -Wall

all: daytime.cpp server ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o daytime daytime.cpp ../lib/readline.cpp
server: daytimeServer.cpp
	$(CC) $(CFLAGS) -o daytimeServer daytimeServer.cpp
