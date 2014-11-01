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
#define BUF_SiZE 256
#define QUE_SIZE 10

using namespace std;

static void bail(string on_what) {
    cerr << strerror(errno) << ": " << on_what << "\n";
}

static void str_echo(int myclient) {
    ssize_t n;
    char line[BUF_SiZE];
    bzero(line, BUF_SiZE);

    while (1) {
        if ( (n = readline(myclient, line, BUF_SiZE)) == 0) {
            return; /* connection closed by other end */
        }
        int check = write(myclient, line, sizeof(line));
        /* error */
        if (check == -1) {
            bail("write() in str_echo");
            return;
        }
    }
}

int main (int argc, char* argv[]) {
    /* give server address in command line */
    string server_addr;
    if (argc >= 2) {
        server_addr = argv[1];
    }
    /* default is localhost */
    else {
        server_addr = "127.0.0.1";
    }
    
    /* give the server port in command line's second argc */
    string server_port = "44444"; // default port
    if (argc >= 3) {
        server_port = argv[2];
    }

    /* time to create socket */
    int listen_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_socket == -1) {
        bail("socket()");
        return 1;
    }

    /* create a server socket address */
    struct sockaddr_in server_socket_info; // in netinet/in.h
    bzero(&server_socket_info, sizeof(server_socket_info)); // init to all zero
    server_socket_info.sin_family = AF_INET;
    server_socket_info.sin_port = htons(atoi(server_port.c_str()));
    
    /* to judge if server_addr is INADDR_ANY */
    /* normal address */
    if (server_addr != "*") {
        server_socket_info.sin_addr.s_addr = inet_addr(server_addr.c_str());
        if (server_socket_info.sin_addr.s_addr == INADDR_NONE) {
            bail("Bad address");
            return 1;
        }
    }
    /* wild address */
    else {
        server_socket_info.sin_addr.s_addr = INADDR_ANY;
    }
    
    /* bind the server address */
    int check = bind(listen_socket, (struct sockaddr *)&server_socket_info, sizeof(server_socket_info));
    if (check == -1) {
        bail("bind(2)");
        return 1;
    }

    /* make it a listrning socket */
    check = listen(listen_socket, QUE_SIZE);
    if (check == -1) {
        bail("listen(2)");
        return 1;
    }

    signal(SIGCHLD, SIG_IGN);
    /* start the server loop */
    struct sockaddr_in client_socket_info; // in netinet/in.h
    while (1) {
        /* wait for a connect */
        socklen_t client_socket_info_len = sizeof(client_socket_info);
        int myclient = accept(listen_socket, (struct sockaddr *) &client_socket_info, &client_socket_info_len);
        if (myclient == -1) {
            bail("accept(2)");
            return 1;
        }

        /* fork() */
        pid_t childpid = 0;
        if ((childpid == fork()) == 0) { /* child process */
            close(listen_socket);
            str_echo(myclient);
            return 0;
        }
        else if (childpid == -1) {
            bail("fork()");
            return 1;
        }
        close(myclient);
    }
    return 0;
}
