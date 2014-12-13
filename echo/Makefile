CC = clang++
CFLAGS = -Wall

tcp: echoServer.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o echoServer echoServer.cpp ../lib/readline.cpp

udp: echoServerUDP.cpp echoClientUDP.cpp ../lib/readline.cpp ../lib/readline.h
	$(CC) $(CFLAGS) -o echoServer echoServerUDP.cpp ../lib/readline.cpp
	$(CC) $(CFLAGS) -o echoClient echoClientUDP.cpp ../lib/readline.cpp
