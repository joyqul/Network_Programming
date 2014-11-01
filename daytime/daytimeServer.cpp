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
#define BUF_SiZE 256
#define QUE_SIZE 10

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
    
    /* give the server port in command line's second argc */
    string server_port = "44444"; // default port
    if (argc >= 3) {
        server_port = argv[2];
    }

    /* time to create socket */
    int daytime_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (daytime_socket == -1) {
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
    int check = bind(daytime_socket, (struct sockaddr *)&server_socket_info, sizeof(server_socket_info));
    if (check == -1) {
        bail("bind(2)");
        return 1;
    }

    /* make it a listrning socket */
    check = listen(daytime_socket, QUE_SIZE);
    if (check == -1) {
        bail("listen(2)");
        return 1;
    }

    /* start the server loop */
    struct sockaddr_in client_socket_info; // in netinet/in.h
    while (1) {
        /* wait for a connect */
        socklen_t client_socket_info_len = sizeof(client_socket_info);
        int myclient = accept(daytime_socket, (struct sockaddr *) &client_socket_info, &client_socket_info_len);
        if (myclient == -1) {
            bail("accept(2)");
            return 1;
        }
        
        /* generate a time stamp */
        time_t timestamp;
        time(&timestamp);
        /* client's address */
        char client_addr_buff[BUF_SiZE];
        inet_ntop(AF_INET, &client_socket_info.sin_addr, client_addr_buff, sizeof(client_addr_buff));
        string client_addr= client_addr_buff;
        /* client's port */
        int client_port_num = ntohs(client_socket_info.sin_port);
        char client_port_buff[BUF_SiZE];
        sprintf(client_port_buff, "%d", client_port_num);
        string client_port = client_port_buff;
        /* output string */
        string pre_str = "Hi, " + client_addr + ":" + client_port + "\n";
        pre_str = pre_str + "This is joyqul's daytime server (i__i)\nThe Time is: \n";
        char daytime_buf[BUF_SiZE];
        strftime(daytime_buf, sizeof(daytime_buf), 
                "%A %b %d %H:%M:%S %Y (i__i)\n",
                localtime(&timestamp));

        string output = pre_str + daytime_buf + "That's it!\nByee!\n";
        /* write result to the client */
        check = write(myclient, output.c_str(), output.size());
        if (check == -1) {
            bail("write(2)");
            return 1;
        }

        close(myclient);
    }
    return 0;
}
