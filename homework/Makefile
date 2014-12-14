CC = clang++
CFLAGS = -Wall

all: sig timeout set

sig: hw02fileReceiverSIG.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o fileReceiverSIG hw02fileReceiverSIG.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o fileSenderSIG hw02fileSender.cpp ../lib/readline.cpp

timeout: hw02fileReceiverTimeout.cpp hw02fileSender.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o fileReceiverTimeout hw02fileReceiverTimeout.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o fileSenderTimeout hw02fileSender.cpp ../lib/readline.cpp

set: hw02fileReceiverSet.cpp hw02fileSender.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o fileReceiverSet hw02fileReceiverSet.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o fileSenderSet hw02fileSender.cpp ../lib/readline.cpp

chatServer: hw01chatRoomServer.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o chatServer hw01chatRoomServer.cpp ../lib/readline.cpp

chatClient: hw01chatRoomClient.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o chatClient hw01chatRoomClient.cpp ../lib/readline.cpp
