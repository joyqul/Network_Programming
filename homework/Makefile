CC = clang++
CFLAGS = -Wall

all: server client

server: hw03Server.cpp ../lib/readline.cpp ../lib/readline.h ../lib/files.cpp ../lib/files.h
	$(CC) $(CFLAGS) -o server.exe hw03Server.cpp ../lib/readline.cpp ../lib/files.cpp

client: hw03Client.cpp ../lib/readline.cpp ../lib/readline.h ../lib/files.cpp ../lib/files.h
	$(CC) $(CFLAGS) -o client.exe hw03Client.cpp ../lib/readline.cpp ../lib/files.cpp

sig: hw02fileReceiver.cpp hw02fileSenderSIG.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o fileReceiverSIG hw02fileReceiver.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o fileSenderSIG hw02fileSenderSIG.cpp ../lib/readline.cpp

timeout: hw02fileReceiver.cpp hw02fileSenderSelect.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o fileReceiverSelect hw02fileReceiver.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o fileSenderSelect hw02fileSenderSelect.cpp ../lib/readline.cpp

set: hw02fileReceiver.cpp hw02fileSenderSet.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o fileReceiverSet hw02fileReceiver.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o fileSenderSet hw02fileSenderSet.cpp ../lib/readline.cpp

chatServer: hw01chatRoomServer.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o chatServer hw01chatRoomServer.cpp ../lib/readline.cpp

chatClient: hw01chatRoomClient.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o chatClient hw01chatRoomClient.cpp ../lib/readline.cpp
