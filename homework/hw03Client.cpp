#include <iostream>
#include <fstream>
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

static inline void load_bar(int now, int total, int width) {
    printf("Progress : [");
    
    double ratio = width/(double)total;
    int count = ratio*now;

    for (int i = 0; i < count; ++i) {
        printf("#");
    }

    for (int i = count; i < width; ++i) {
        printf(" ");
    }

    cout << "]\r" << flush;
}

static inline long get_file_len(ifstream* input) {
    input->seekg(0, input->end);
    long len = input->tellg();
    input->seekg(0);
    return len;
}

static void download(string message, int my_socket) {
    
}

static void upload(string message, int my_socket) {
    string filename;
    for (int i = 5; i < message.size(); ++i) {
        if (message[i] == '\n' || message[i] == '\r') break;
        filename += message[i];
    }

    ifstream input(filename.c_str(), ios::binary);
    if (input.fail()) {
        cerr << "No such file: " << filename << endl;
        return;
    }

    /* file size */
    long filesize = get_file_len(&input);
    int redundent = filesize%(BUF_SIZE-0);
    long packet_num = filesize/(BUF_SIZE-0) + 1;
    char buf[BUF_SIZE];
    bzero(buf, BUF_SIZE);
    sprintf(buf, "SENDFILE%s\r%d\r%ld\n", filename.c_str(), redundent, packet_num); /* redundent */
    /* trans file */
    write(my_socket, buf, BUF_SIZE);

    filesize = filesize/BUF_SIZE;
    for (int i = 0; i < filesize; ++i) {
        bzero(buf, BUF_SIZE);
        input.read(buf, BUF_SIZE);
        write(my_socket, buf, BUF_SIZE);
        if (i % 2 == 0) {
            load_bar(i, filesize+1, 15);
        }
    }
    bzero(buf, BUF_SIZE);
    input.read(buf, redundent);
    input.close();
    write(my_socket, buf, redundent);
    load_bar(filesize+1, filesize+1, 15);

    printf("\nUpload %s complete!\n", filename.c_str());
}

static void client_sleep(string message) {
    printf("Client starts to sleep\n");
    int sec = 0;
    for (int i = 7; i < message.size(); ++i) {
        char ch = message[i];
        if ( ch > '9' || ch < '0') break;
        sec = sec*10 + ch - '0';
    }

    for (int i = 1; i <= sec; ++i) {
        sleep(1);
        printf("Sleep %d\n", i);
    }
    printf("Client wakes up\n");
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

            char sleep_cmp[7], put_cmp[5];
            strncpy(put_cmp, message.c_str(), 5);
            put_cmp[4] = '\0';
            strncpy(sleep_cmp, message.c_str(), 7);
            sleep_cmp[6] = '\0';
            if (strcmp(sleep_cmp, "/sleep") == 0) {
                client_sleep(message);
            }
            else if (strcmp(put_cmp, "/put") == 0) {
                upload(message, my_socket);
            }
        }
    }
}

int main (int argc, char* argv[]) {

    /* give server address in command line */
    string server_addr, server_port, client_name;
    if (argc != 4) {
        cout << "Usage: ./client.exe <SERVER IP> <SERVER PORT> <username>" << endl;
    }
    server_addr = argv[1];
    server_port = argv[2];
    client_name = "name ";
    client_name = client_name + argv[3] + "\n";
    
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
    write(my_socket, client_name.c_str(), client_name.size());

    /* read the info */
    client(stdin, my_socket);
    close(my_socket);
    return 0;
}
