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

#define PORT 44444
#define SRV_IP "127.0.0.1"
#define FILENAME "transfile"

using namespace std;

void bail(string s) {
    perror(s.c_str());
    exit(1);
}

inline long get_file_len(ifstream* input) {
    input->seekg(0, input->end);
    long len = input->tellg();
    input->seekg(0);
    return len;
}

int main(int argc, char* argv[]) {

    /* give the port in command line's first argc */
    string connect_port;
    if (argc == 1) {
        cout << "Connect to default port: 44444" << endl;
        connect_port = "44444";
    }
    else {
        connect_port = argv[1];
    }

    /* time to create socket */
    int connect_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connect_socket < 0) {
        bail("socket");
    }

    struct sockaddr_in connect_socket_info; // in netinet/in.h
    bzero(&connect_socket_info, sizeof(connect_socket_info)); // init to all zero
    socklen_t slen = sizeof(connect_socket_info);
    connect_socket_info.sin_family = AF_INET;
    connect_socket_info.sin_port = htons(PORT);

    if (inet_aton(SRV_IP, &connect_socket_info.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    /* open file */
    ifstream input(FILENAME, ios::binary);
    if (input.fail()) {
        bail("file opening failed");
    }

    /*send redundant*/
    char buf[BUF_SIZE];
    long filesize = get_file_len(&input);
    int redundant = filesize%BUF_SIZE;
    sprintf(buf, "%d", redundant);
    if (sendto(connect_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, slen) == -1) {
        bail("sendto()");
    }

    /* send file size */
    filesize = filesize/BUF_SIZE + 1;
    sprintf(buf, "%ld", filesize);
    if (sendto(connect_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, slen) == -1) {
        bail("sendto()");
    }

    /* send file */
    for (long i = 0; i < filesize; ++i) {
        if (i == filesize-1) input.read(buf, redundant);
        else input.read(buf, BUF_SIZE);
        int check = sendto(connect_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, slen);
        if (check == -1) {
            bail("sendto()");
        }
    }

    input.close();
    close(connect_socket);
    return 0;
}
