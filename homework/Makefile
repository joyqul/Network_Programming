CC = clang++
CFLAGS = -Wall

all: sig timeout set

sig: hw02fileServerSIG.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o fileServerSIG hw02fileServerSIG.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o fileClientSIG hw02fileClient.cpp ../lib/readline.cpp

timeout: hw02fileServerTimeout.cpp hw02fileClient.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o fileServerTimeout hw02fileServerTimeout.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o fileClientTimeout hw02fileClient.cpp ../lib/readline.cpp

set: hw02fileServerSet.cpp hw02fileClient.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o fileServerSet hw02fileServerSet.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o fileClientSet hw02fileClient.cpp ../lib/readline.cpp

chatServer: hw01chatRoomServer.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o chatServer hw01chatRoomServer.cpp ../lib/readline.cpp

chatClient: hw01chatRoomClient.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o chatClient hw01chatRoomClient.cpp ../lib/readline.cpp
