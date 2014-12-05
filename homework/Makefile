CC = clang++
CFLAGS = -Wall

all: chatServer chatClient

chatServer: hw01chatRoomServer.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o chatServer hw01chatRoomServer.cpp ../lib/readline.cpp
chatClient: hw01chatRoomClient.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o chatClient hw01chatRoomClient.cpp ../lib/readline.cpp
