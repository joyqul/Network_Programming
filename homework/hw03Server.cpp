#include <iostream>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "../lib/readline.h"
#define QUE_SIZE 10
#define MAX_CLIENT 20

using namespace std;

struct STORE {
    string filename;
    int owner_client;
};

vector<STORE> storehouse;

struct CLIENT {
    int id;
    string name, ip, port;
    int filesize, redundent, now;
    ofstream upload;
    bool uploading, downloading;
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
    for (int i = 0; i < max_fd; ++i) {
        if (client[i].id < 0) continue;
        write(client[i].id, message.c_str(), message.size());
    }
}

static void change_name(int me, string target, int max_fd) {
    string new_name, message;

    for (int i = 5; i < target.size()-1; ++i) {
        new_name += target[i];
    }

    /* write welcome message */
    string welcome_message = "Welcome to the dropbox-like server! : " + new_name + "\n";
    write(client[me].id, welcome_message.c_str(), welcome_message.size());
    client[me].name = new_name;
    client[me].uploading = false;
    client[me].downloading = false;
}

static void init_upload(int me, string message) {
    STORE tmp;
    int i;
    for (i = 8; i < message.size(); ++i) {
        if (message[i] == '\0') break;
        if (message[i] == '\r') break;
        if (message[i] == '\n') break;
        tmp.filename += message[i];
    }
    cout << tmp.filename << endl;

    ++i;
    int redundent = 0;
    for (; i < message.size(); ++i) {
        if (message[i] == '\0') break;
        if (message[i] == '\r') break;
        if (message[i] == '\n') break;
        redundent = redundent*10 + message[i] - '0';
    }

    ++i;
    int filesize = 0;
    for (; i < message.size(); ++i) {
        if (message[i] == '\0') break;
        if (message[i] == '\r') break;
        if (message[i] == '\n') break;
        filesize = filesize*10 + message[i] - '0';
    }

    tmp.owner_client = me;
    storehouse.push_back(tmp);
    client[me].now = 0;
    client[me].uploading = true;
    client[me].redundent = redundent;
    client[me].filesize = filesize;
    client[me].upload.open(tmp.filename.c_str(), ios::binary);

    cerr << redundent << " " << filesize << endl;
}

static void upload(int me, char line[]) {
    cout << "UPLOAD\n";
    if (client[me].now == client[me].filesize - 1) {
        client[me].upload.write(line, client[me].redundent*sizeof(char));
        client[me].upload.close();
        client[me].uploading = false;
        cout << "finish" << endl;
        return;
    }
    ++client[me].now;
    client[me].upload.write(line, (BUF_SIZE)*sizeof(char));
}

int set_nonblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

int main (int argc, char* argv[]) {
    
    string server_addr = "127.0.0.1";

    /* give the server port in command line's second argc */
    string server_port = "44444"; // default port
    if (argc == 2) {
        server_port = argv[1];
    }
    else {
        printf("Using default port: 44444\n");
    }

    /* time to create socket */
    int listen_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
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

    /* set non-blocking */
    set_nonblocking(listen_socket);

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
    for (int i = 0; i < MAX_CLIENT; ++i) {
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
            int new_id = check_server_capacity(myclient);
            if (new_id < 0 && --nready <= 0) continue;
            if (new_id < 0) break;

            /* set connect to the set */
            FD_SET(myclient, &read_set);
            if (myclient + 1 > max_fd) max_fd = myclient + 1;


            /* client's address */
            char client_addr_buff[BUF_SIZE];
            inet_ntop(AF_INET, &client_socket_info.sin_addr, client_addr_buff, sizeof(client_addr_buff));
            client[new_id].ip = client_addr_buff;
            /* client's port */
            int client_port_num = ntohs(client_socket_info.sin_port);
            char client_port_buff[BUF_SIZE];
            sprintf(client_port_buff, "%d", client_port_num);
            client[new_id].port = client_port_buff;

            /* no more descriptors */
            if (--nready <= 0) continue;
        }

        /* readline */
        char line[BUF_SIZE];
        bzero(line, BUF_SIZE);
        for (int i = 0; i < MAX_CLIENT; ++i) {
            int myclient = client[i].id;
            if ( myclient < 0) continue;
            if (FD_ISSET(myclient, &working_set)) {
                /* connection closed by client */
                if ( (check = read(myclient, line, BUF_SIZE)) == 0 ) {
                    close(myclient);
                    FD_CLR(myclient, &read_set);
                    client[i].id = -1;
                    string message = client[i].name + " is offline.\n";
                    broadcast(message, max_fd);
                    break;
                }
                else {
                    char name_cmp[BUF_SIZE], send_file[BUF_SIZE];
                    strncpy(name_cmp, line, 4); name_cmp[4] = '\0';
                    strncpy(send_file, line, 8); name_cmp[8] = '\0';
                    if (strcmp(name_cmp, "name") == 0) {
                        string target = line;
                        change_name(i, target, max_fd);
                    }
                    else if (strcmp(send_file, "SENDFILE") == 0) {
                        init_upload(i, line);
                    }
                    else if (client[i].uploading) {
                        upload(i, line);
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
