#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../lib/readline.h"

using namespace std;

static void bail(string on_what) {
    cerr << strerror(errno) << ": " << on_what << "\n";
}

static void client(FILE* fp, int my_socket) {
    fd_set read_set;
    FD_ZERO(&read_set);
    int max_fd = my_socket + 1;
    int stdin_eof = 0;
    
    while (1) {
        FD_SET(my_socket, &read_set);
        if (stdin_eof == 0) {
            FD_SET(fileno(fp), &read_set);
        }

        // check if update
        if (fileno(fp) + 1 > max_fd) max_fd = fileno(fp) + 1;

        // select
        select(max_fd, &read_set, NULL, NULL, NULL);

        // if there's data from server
        if ( FD_ISSET(my_socket, &read_set) ) {
            int check;
            char line[BUF_SIZE];
            bzero(line, BUF_SIZE);
            if ( (check = read(my_socket, line, BUF_SIZE)) == 0 ) {
                if (stdin_eof == 1) return;
                bail("server terminated permutaurely");
                return;
            }
            else {
                cout << line;
            }
        }

        // stdin
        if ( FD_ISSET(fileno(fp), &read_set) ) {
            string message;
            getline(cin, message);
            if (strcmp(message.c_str(), "/exit") == 0) return;
        }
    }
}

int main (int argc, char* argv[]) {

    /* give server address in command line */
    string server_addr, server_port, clinet_name;
    if (argc != 4) {
        cout << "Usage: ./client.exe <SERVER IP> <SERVER PORT> <username>" << endl;
    }
    server_addr = argv[1];
    server_port = argv[2];
    clinet_name = "name ";
    clinet_name = clinet_name + argv[3] + "\n\n";
    
    /* create a server socket address */
    struct sockaddr_in socket_info; // in netinet/in.h
    bzero(&socket_info, sizeof(socket_info)); // init to all zero
    socket_info.sin_family = AF_INET;
    socket_info.sin_port = htons(atoi(server_port.c_str()));
    socket_info.sin_addr.s_addr = inet_addr(server_addr.c_str()); // in arpa/inet.h

    /* check if address is 255.255.255.255*/
    if (socket_info.sin_addr.s_addr == INADDR_NONE) {
        bail("Bad address.");
        return 1;
    }
    
    /* time to create socket */
    int my_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (my_socket == -1) {
        bail("socket()");
        return 1;
    }
    
    /* connect to the server */
    int check = connect(my_socket, (struct sockaddr *) &socket_info, sizeof(socket_info));
    if (check == -1) {
        bail("connect(2)");
        return 1;
    }
    write(my_socket, clinet_name.c_str(), clinet_name.size());

    /* read the info */
    client(stdin, my_socket);
    close(my_socket);
    return 0;
}
