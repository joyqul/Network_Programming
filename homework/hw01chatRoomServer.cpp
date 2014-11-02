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
#define MAX_CLIENT 2

using namespace std;

struct CLIENT {
    int id;
    string name, ip, port;
} client[MAX_CLIENT];

static void bail(string on_what) {
    cerr << strerror(errno) << ": " << on_what << "\n";
}

static int check_server_capacity(int myclient) {
    for (int client_entry = 0; client_entry < MAX_CLIENT; ++client_entry) {
        if (client[client_entry].id < 0) {
            client[client_entry].id = myclient;
            client[client_entry].name = "anonymous";
            return client_entry;
        }
    }
    string message = "Too many client, so buzy :/\n";
    write(myclient, message.c_str(), message.size());
    close(myclient);
    return -1;
}

static void broadcast(string message, int max_fd) {
    for (int i = 0; i < MAX_CLIENT; ++i) {
        if (client[i].id < 0) continue;
        write(client[i].id, message.c_str(), message.size());
    }
}

static void show_who(int me, int max_fd) {
    string message;
    for (int i = 0; i < MAX_CLIENT; ++i) {
        if (client[i].id < 0) continue;
        message = message + "[Server] " + client[i].name + " " +
            client[i].ip + "/" + client[i].port;
        if (i == me) {
            message = message + " <-me";
        }
        message = message + "\n";
    }
    write(client[me].id, message.c_str(), message.size());
}

int main (int argc, char* argv[]) {
    /* instruction */
    char who[] = "who";

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

    /* initialize read and working set */
    fd_set read_set, working_set;
    FD_ZERO(&read_set);
    FD_SET(listen_socket, &read_set);   // check listen socket
    int max_fd = listen_socket + 1;

    /* initialize timeout */
    struct timeval timeout_value;

    /* initialize to client */
    for (int i = 0i; i < MAX_CLIENT; ++i) {
        client[i].id = -1; // -1 means available
    }

    /* start the server loop */
    struct sockaddr_in client_socket_info; // in netinet/in.h
    while (1) {
        /* copy the read_set to the working_set */
        FD_ZERO(&working_set);
        for (int i = 0; i < max_fd; ++i) {
            if (FD_ISSET(i, &read_set)) {
                FD_SET(i, &working_set);
            }
        }
        
        /* check ready */
        int nready = select(max_fd, &working_set, NULL, NULL, &timeout_value);
        if (nready == -1) {
            bail("select(2)");
            return 1;
        }
        else if (!nready) {
            cout << "Timeout.\n";
            continue;
        }


        /* check if a connect has occured */
        if (FD_ISSET(listen_socket, &working_set)) {

            /* wait for a connect */
            socklen_t client_socket_info_len = sizeof(client_socket_info);
            int myclient = accept(listen_socket, (struct sockaddr *) &client_socket_info, &client_socket_info_len);
            if (myclient == -1) {
                bail("accept(2)");
            }

            /* check if exceeded server capacity, if not full, add it to client */
            int client_id = check_server_capacity(myclient);
            if (client_id == -1 && --nready <= 0) continue;
            if (client_id == -1) break;

            /* set connect to the set */
            FD_SET(myclient, &read_set);
            if (myclient + 1 > max_fd) max_fd = myclient + 1;

            /* write welcome message */
            string welcome_message = "[Server] Hello, anonymous! From: ";
            /* client's address */
            char client_addr_buff[BUF_SiZE];
            inet_ntop(AF_INET, &client_socket_info.sin_addr, client_addr_buff, sizeof(client_addr_buff));
            client[client_id].ip = client_addr_buff;
            /* client's port */
            int client_port_num = ntohs(client_socket_info.sin_port);
            char client_port_buff[BUF_SiZE];
            sprintf(client_port_buff, "%d", client_port_num);
            client[client_id].port = client_port_buff;
            welcome_message = welcome_message + client[client_id].ip + "/" + client[client_id].port + "\n";
            write(myclient, welcome_message.c_str(), welcome_message.size());

            /* to other client */
            broadcast("[Server] Someone is coming!\n", max_fd);

            /* no more descriptors */
            if (--nready <= 0) continue;
        }

        /* readline */
        char line[BUF_SiZE];
        bzero(line, BUF_SiZE);
        for (int i = 0; i < max_fd; ++i) {
            int myclient = client[i].id;
            if ( myclient < 0) continue;
            if (FD_ISSET(myclient, &working_set)) {
                /* connection closed by client */
                if ( (check = readline(myclient, line, BUF_SiZE)) == 0 ) {
                    close(myclient);
                    FD_CLR(myclient, &read_set);
                    client[i].id = -1;
                    string message = client[i].name + "is offline.\n";
                    broadcast(message, max_fd);
                    break;
                }
                else {
                    char cmp[BUF_SiZE];
                    strncpy(cmp, line, 3);
                    if ( strcmp(cmp, who) == 0 ) {
                        show_who(i, max_fd);
                    }
                    
                    else {
                        string message = client[i].name + " yell " + line;
                        broadcast(message, max_fd);
                    }
                }
            }
        }

        /* reduce i */
        for (int i = max_fd - 1; i >= 0; --i) {
            if (FD_ISSET(i, &read_set)) {
                break;
            }
            max_fd = i + 1;
        }

    }

    return 0;
}
