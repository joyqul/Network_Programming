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
#define BUF_SiZE 256

using namespace std;

static void bail(string on_what) {
    cerr << strerror(errno) << ": " << on_what << "\n";
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
    
    /* check datetime tcp service */
    struct servent *service_entry = getservbyname("daytime", "tcp"); // in netdb.h
    if (service_entry == NULL) {
        cerr << "Unknown service: daytime tcp\n";
        return 1;
    }

    /* create a server socket address */
    struct sockaddr_in socket_info; // in netinet/in.h
    bzero(&socket_info, sizeof(socket_info)); // init to all zero
    socket_info.sin_family = AF_INET;
    socket_info.sin_port = service_entry->s_port;
    socket_info.sin_addr.s_addr = inet_addr(server_addr.c_str()); // in arpa/inet.h

    /* check if address is 255.255.255.255*/
    if (socket_info.sin_addr.s_addr == INADDR_NONE) {
        bail("Bad address.");
        return 1;
    }
    
    /* time to create socket */
    int daytime_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (daytime_socket == -1) {
        bail("socket()");
        return 1;
    }
    
    /* connect to the server */
    int check = connect(daytime_socket, (struct sockaddr *) &socket_info, sizeof(socket_info));
    if (check == -1) {
        bail("connect(2)");
        return 1;
    }

    /* read the info */
    char daytime_buf[BUF_SiZE];
    bzero(daytime_buf, BUF_SiZE);
    check = read(daytime_socket, &daytime_buf, BUF_SiZE-1);
    if (check == -1) {
        bail("read(2)");
        return 1;
    }
    
    cout << "Date & time is: \n" << daytime_buf << endl;
    close(daytime_socket);
    return 0;
}
