#include <iostream>
#include <fstream>
#include <queue>
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

#define FILENAME "receivefile"
#define MAX 100000
#define INDEX_SIZE 32

using namespace std;

void bail(string s) {
    perror(s.c_str());
    exit(1);
}

struct SEGS {
    int id;
    char buf[BUF_SIZE-INDEX_SIZE];
};

class cmp {
public:
    bool operator() (const SEGS& a, const SEGS& b) const {
        return a.id > b.id;
    }
};

priority_queue<SEGS, vector<SEGS>, cmp> segments;

void write_file(int redundent) {
    /* open file */
    ofstream output;
    output.open(FILENAME, ios::binary);
    
    while (segments.size()-1) {
        SEGS now = segments.top();
        segments.pop();
        cout << "id: " << now.id << endl;
        output.write(now.buf, (BUF_SIZE-INDEX_SIZE)*sizeof(char));
    }

    SEGS now = segments.top();
    segments.pop();
    output.write(now.buf, redundent*sizeof(char));
    output.close();
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
    char buf[BUF_SIZE-INDEX_SIZE+1], indexbuf[INDEX_SIZE+1], readbuf[BUF_SIZE];

    int redundent = 0;
    long packet_num = MAX;
    for (int i = 0; i < packet_num; ++i) {
        int test = recvfrom(listen_socket, readbuf, BUF_SIZE, 0, (struct sockaddr *)&connect_socket_info, &slen);
        if (test < 0) {
            bail("recvfrom()");
        }
        /* get packet id */
        memcpy(indexbuf, readbuf, INDEX_SIZE);
        indexbuf[INDEX_SIZE] = '\0';
        int index = atoi(indexbuf);
        cout << "index: " << index << endl;

        /* get data */
        memcpy(buf, readbuf+INDEX_SIZE, BUF_SIZE-INDEX_SIZE);
        /* get redundent */
        if (index == 0) { 
            redundent = atoi(buf);
        }
        /* get packet_num */
        else if (index == 1) {
            packet_num = atol(buf);
            cout << "pa: " << packet_num << endl;
        }
        else {
            SEGS tmp;
            tmp.id = index;
            memcpy(tmp.buf, buf, BUF_SIZE-INDEX_SIZE);
            segments.push(tmp);
        }
    }

    cout << "re: " << redundent << endl;
    write_file(redundent);
    close(listen_socket);
    return 0;
}
