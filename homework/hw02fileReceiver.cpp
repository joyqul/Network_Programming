#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "../lib/readline.h"

#define NPACK 10

using namespace std;

void bail(char *s) {
    perror(s);
    exit(1);
}

int main(int argc, char* argv[]) {

    /* give the server port in command line's first argc */
    string server_port;
    if (argc == 1) {
        cout << "Using default port: 44444" << endl;
        server_port = "44444";
    }
    else {
        server_port = argv[1];
    }

    /* time to create socket */
    int listen_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (listen_socket < 0) {
        bail("socket");
    }

    /* create a server socket address */
    struct sockaddr_in server_socket_info; // in netinet/in.h
    bzero(&server_socket_info, sizeof(server_socket_info)); // init to all zero
    server_socket_info.sin_family = AF_INET;
    server_socket_info.sin_port = htons(atoi(server_port.c_str()));
    server_socket_info.sin_addr.s_addr = INADDR_ANY;
    
    /* bind the server address */
    int check = bind(listen_socket, (struct sockaddr *)&server_socket_info, sizeof(server_socket_info));
    if (check < 0) {
        bail("bind");
    }

    struct sockaddr_in client_socket_info; // in netinet/in.h
    socklen_t slen = sizeof(client_socket_info);
    char buf[BUF_SIZE];

    for (int i=0; i<NPACK; i++) {
        if (recvfrom(listen_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&client_socket_info, &slen)==-1) {
            bail("recvfrom()");
        }
        printf("Received packet from %s:%d\nData: %s\n\n", 
                inet_ntoa(client_socket_info.sin_addr), ntohs(client_socket_info.sin_port), buf);
    }

    close(listen_socket);
    return 0;
}
