#include <iostream>
#include <fstream>
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

#define FILENAME "transfile"

using namespace std;

void bail(string s) {
    perror(s.c_str());
    exit(1);
}

int main(int argc, char* argv[]) {

    /* give the my port in command line's first argc */
    string my_port;
    if (argc == 1) {
        cout << "Using default port: 44444" << endl;
        my_port = "44444";
    }
    else {
        my_port = argv[1];
    }

    /* time to create socket */
    int listen_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (listen_socket < 0) {
        bail("socket");
    }

    /* create a my socket address */
    struct sockaddr_in my_socket_info; // in netinet/in.h
    bzero(&my_socket_info, sizeof(my_socket_info)); // init to all zero
    my_socket_info.sin_family = AF_INET;
    my_socket_info.sin_port = htons(atoi(my_port.c_str()));
    my_socket_info.sin_addr.s_addr = INADDR_ANY;
    
    /* bind the my address */
    int check = bind(listen_socket, (struct sockaddr *)&my_socket_info, sizeof(my_socket_info));
    if (check < 0) {
        bail("bind");
    }

    struct sockaddr_in connect_socket_info; // in netinet/in.h
    socklen_t slen = sizeof(connect_socket_info);
    char buf[BUF_SIZE];

    /* get redundent */
    int test = recvfrom(listen_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, &slen);
    if (test < 0) {
        bail("recvfrom()");
    }
    int redundent = atoi(buf);
    
    /* get packet_num */
    test = recvfrom(listen_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, &slen);
    if (test < 0) {
        bail("recvfrom()");
    }
    long packet_num = atol(buf);

    /* open file */
    ofstream output;
    output.open("receivefile", ios::binary);
    
    for (int i = 0; i < packet_num; ++i) {
        cout << i << endl;
        test = recvfrom(listen_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, &slen);
        if (test < 0) {
            bail("recvfrom()");
        }
        else if (i == packet_num-1) {
            cout << "LAST" << endl;
            cout << redundent << endl;
            output.write(buf, redundent*sizeof(char));
        }
        else {
            output.write(buf, BUF_SIZE*sizeof(char));
        }
    }

    output.close();
    close(listen_socket);
    return 0;
}
