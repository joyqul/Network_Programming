#include <iostream>
#include <fstream>
#include <vector>
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

#define INDEX_SIZE 32
#define PORT 44444
#define SRV_IP "127.0.0.1"
#define FILENAME "transfile"
#define FREQUENCY 444
#define WINDOW_SIZE 16

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

struct PACKET {
    char buf[BUF_SIZE];
};

vector<PACKET> packet_vec;

int readable_timeo(int  fd, int sec) {
    fd_set rset;
    struct timeval tv;
    
    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    
    tv.tv_sec = sec;
    tv.tv_usec = 50;
    
    return (select(fd+1, &rset, NULL, NULL, &tv));
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

    int index = 0;
    /*send redundant*/
    char buf[BUF_SIZE], readbuf[BUF_SIZE-INDEX_SIZE], ackbuf[INDEX_SIZE];
    long filesize = get_file_len(&input);
    int redundant = filesize%(BUF_SIZE-INDEX_SIZE);
    sprintf(buf, "%032d%d\0", index++, redundant);

    PACKET store;
    memcpy(store.buf, buf, BUF_SIZE);
    packet_vec.push_back(store);
    if (sendto(connect_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, slen) == -1) {
        bail("sendto()");
    }

    /* send file size */
    filesize = filesize/(BUF_SIZE-INDEX_SIZE) + 1;
    sprintf(buf, "%032d%ld\0", index++, filesize+2);

    memcpy(store.buf, buf, BUF_SIZE);
    packet_vec.push_back(store);
    if (sendto(connect_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, slen) == -1) {
        bail("sendto()");
    }

    int lastest = -1;
    /* send file */
    for (long i = 0; i < filesize; ++i) {

        /* read file */
        if (i == filesize-1) input.read(readbuf, redundant);
        else input.read(readbuf, BUF_SIZE-INDEX_SIZE);
        sprintf(buf, "%032d", index++);
        memcpy(buf+INDEX_SIZE, readbuf, BUF_SIZE-INDEX_SIZE);

        /* store the message */
        memcpy(store.buf, buf, BUF_SIZE);
        packet_vec.push_back(store);
        int check = sendto(connect_socket, buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, slen);
        if (check == -1) {
            bail("sendto()");
        }

        if (i % FREQUENCY) continue;

        /* receive */
        int test;
        while (readable_timeo(connect_socket, 0) != 0) {
            test = recvfrom(connect_socket, ackbuf, INDEX_SIZE, 0, (struct sockaddr *)&connect_socket_info, &slen);
            if (test < 0) {
                bail("recv");
            }
            int ack_id = atoi(ackbuf);
            if (ack_id > lastest) lastest = ack_id;
        }

        /* retransmit */
        if (lastest < i - WINDOW_SIZE) {
            check = sendto(connect_socket, packet_vec[lastest+1].buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, slen);
            if (check == -1) {
                bail("sendto()");
            }
        }
    }

    while (lastest < filesize-1) {

        if (readable_timeo(connect_socket, 5) == 0) {
            cout << "finish sending" << endl;
            break;
        }

        int test;
        test = recvfrom(connect_socket, ackbuf, INDEX_SIZE, 0, (struct sockaddr *)&connect_socket_info, &slen);
        if (test < 0) {
            bail("recvfrom");
        }

        int ack_id = atoi(ackbuf);
        if (ack_id > lastest) lastest = ack_id;
        if (lastest == filesize-1) break;

        int check = sendto(connect_socket, packet_vec[lastest+1].buf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, slen);
        if (check < 0) {
            bail("sendto");
        }
    }

    input.close();
    close(connect_socket);
    return 0;
}
