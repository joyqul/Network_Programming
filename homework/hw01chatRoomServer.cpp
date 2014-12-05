#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "../lib/readline.h"
#define BUF_SIZE 512
#define QUE_SIZE 10
#define MAX_CLIENT 20

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
    for (int i = 0; i < max_fd; ++i) {
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
            message += " <-me";
        }

        message += "\n";
    }
    write(client[me].id, message.c_str(), message.size());
}

static void change_name(int me, string target, int max_fd) {
    string new_name, message;

    // check size
    if (target.size() < 9 || target.size() > 19) {
        message = "[Server] ERROR: Username can only consists of 2~12 English letters\n";
        write(client[me].id, message.c_str(), message.size());
        return;
    }

    for (int i = 5; i < target.size()-2; ++i) {
        if (!isalpha(target[i])) {
            message = "[Server] ERROR: Username can only consists of 2~12 English letters\n";
            write(client[me].id, message.c_str(), message.size());
            return;
        }
        new_name += target[i];
    }

    // check format
    if (strcmp(new_name.c_str(), "anonymous") == 0) {
        message = "[Server] ERROR: Username cannot be anonymous.\n";
        write(client[me].id, message.c_str(), message.size());
        return;
    }

    // check if is used
    for (int i = 0; i < MAX_CLIENT; ++i) {
        if (client[i].id < 0) continue;
        if (i == me) continue;
        if (new_name == client[i].name) {
            message = "[Server] ERROR: " + new_name + "has been used by others.\n";
            write(client[me].id, message.c_str(), message.size());
            return;
        }
    }
    
    // change the name
    string pre_name = client[me].name;
    client[me].name = new_name;
    message = "[Server] You're now known as " + new_name + ".\n";
    write(client[me].id, message.c_str(), message.size());
    message = "[Server] " + pre_name + " is now known as " + new_name + ".\n";
    broadcast(message, max_fd);
}

static void tell_message(int me, string target, int max_fd) {
    string message;
    if (strcmp(client[me].name.c_str(), "anonymous") == 0) {
        message = "[Server] ERROR: You are anonymous.\n";
        write(client[me].id, message.c_str(), message.size());
        return;
    }
    // get the receiver name
    string receiver;
    int st;
    for (st = 5; st < target.size()-2; ++st) {
        if (!isalpha(target[st])) break;
        receiver += target[st];
    }

    // check if receiver is anonymous
    if (strcmp(receiver.c_str(), "anonymous") == 0) {
        message = "[Server] ERROR: The client to which you sent is anonymous.\n";
        write(client[me].id, message.c_str(), message.size());
        return;
    }

    // get the receiver id
    int receiver_id = -1;
    for (int i = 0; i < MAX_CLIENT; ++i) {
        if (client[i].id < 0) continue;
        if (i == me) continue;
        if (strcmp(receiver.c_str(), client[i].name.c_str()) == 0) {
            receiver_id = i;
            break;
        }
    }

    // check if receiver exist
    if (receiver_id == -1) {
        message = "[Server] ERROR: The receiver doesn't exist.\n";
        write(client[me].id, message.c_str(), message.size());
        return;
    }

    // send message
    for (; st < target.size()-2; ++st) {
        message += target[st];
    }

    message = "[Server] " + client[me].name + " tell you" + message + "\n";
    write(client[receiver_id].id, message.c_str(), message.size());
    message = "[Server] SUCCESS: Your message has been sent.\n";
    write(client[me].id, message.c_str(), message.size());
}

int main (int argc, char* argv[]) {
    /* check the command */
    char who[] = "who";
    char name[] = "name";
    char tell[] = "tell";
    char yell[] = "yell";

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

            /* write welcome message */
            string welcome_message = "[Server] Hello, anonymous! From: ";
            /* client's address */
            char client_addr_buff[BUF_SIZE];
            inet_ntop(AF_INET, &client_socket_info.sin_addr, client_addr_buff, sizeof(client_addr_buff));
            client[new_id].ip = client_addr_buff;
            /* client's port */
            int client_port_num = ntohs(client_socket_info.sin_port);
            char client_port_buff[BUF_SIZE];
            sprintf(client_port_buff, "%d", client_port_num);
            client[new_id].port = client_port_buff;
            welcome_message = welcome_message + client[new_id].ip + "/" + client[new_id].port + "\n";
            write(myclient, welcome_message.c_str(), welcome_message.size());

            /* to other client */
            broadcast("[Server] Someone is coming!\n", max_fd);

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
                if ( (check = readline(myclient, line, BUF_SIZE)) == 0 ) {
                    close(myclient);
                    FD_CLR(myclient, &read_set);
                    client[i].id = -1;
                    string message = client[i].name + "is offline.\n";
                    broadcast(message, max_fd);
                    break;
                }
                else {
                    char who_cmp[BUF_SIZE], name_cmp[BUF_SIZE], tell_cmp[BUF_SIZE], yell_cmp[BUF_SIZE];
                    strncpy(who_cmp, line, 3); who_cmp[3] = '\0';
                    strncpy(name_cmp, line, 4); name_cmp[4] = '\0';
                    strncpy(tell_cmp, line, 4); tell_cmp[4] = '\0';
                    strncpy(yell_cmp, line, 4); yell_cmp[4] = '\0';
                    if (strcmp(who_cmp, who) == 0) {
                        show_who(i, max_fd);
                    }
                    else if (strcmp(name_cmp, name) == 0) {
                        string target = line;
                        change_name(i, target, max_fd);
                    }
                    else if (strcmp(tell_cmp, tell) == 0) {
                        string target = line;
                        tell_message(i, target, max_fd);
                    }
                    else if (strcmp(yell_cmp, yell) == 0) {
                        string message = client[i].name + " yell " + line;
                        broadcast(message, max_fd);
                    }
                    else {
                        string message = "[Server] ERROR: Error command.\n";
                        write(myclient, message.c_str(), message.size());
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
