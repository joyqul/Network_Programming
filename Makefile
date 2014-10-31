CC = clang++
CFLAGS = -Wall

all: daytime.cpp
	$(CC) $(CFLAGS) -o daytime daytime.cpp
