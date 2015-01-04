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
#include "../lib/files.h"

using namespace std;

struct DOWNLOAD {
    long filesize;
    int redundent, now;
    bool downloading;
    string filename;
    ofstream fp;
} downloader;

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

static void download(char message[], int my_socket) {
    if (!downloader.downloading) {
        downloader.filename = "";
        string tmp(message);
        get_file_description(tmp, downloader.filename, downloader.redundent, downloader.filesize);
        cout << "Downloading file : " << downloader.filename << endl;
        downloader.downloading = true;
        downloader.now = 0;
        downloader.fp.open(downloader.filename.c_str(), ios::out | ios::binary);
        return;
    }
    if (downloader.now == downloader.filesize - 1) {
        downloader.fp.write(message, downloader.redundent*sizeof(char));
        downloader.fp.close();
        downloader.downloading = false;
        load_bar(downloader.now+1, downloader.filesize, 15);
        printf("\nDownload %s complete!\n", downloader.filename.c_str());
        return;
    }
    ++downloader.now;
    downloader.fp.write(message, (BUF_SIZE)*sizeof(char));
    load_bar(downloader.now, downloader.filesize, 15);
}

static void upload(string message, int my_socket) {
    string filename;
    for (int i = 5; i < message.size(); ++i) {
        if (message[i] == '\n' || message[i] == '\r') break;
        filename += message[i];
    }

    ifstream input(filename.c_str(), ios::in | ios::binary);
    if (input.fail()) {
        cerr << "No such file: " << filename << endl;
        return;
    }
    cout << "Uploading file : " << filename << endl;

    /* file size */
    char buf[BUF_SIZE];
    long packet_num;
    int redundent;
    generate_file_description(&input, buf, filename, redundent, packet_num);

    /* trans file */
    write(my_socket, buf, BUF_SIZE);

    for (int i = 0; i < packet_num-1; ++i) {
        bzero(buf, BUF_SIZE);
        input.read(buf, BUF_SIZE);
        write(my_socket, buf, BUF_SIZE);
        load_bar(i, packet_num, 15);
    }
    bzero(buf, BUF_SIZE);
    input.read(buf, redundent);
    input.close();
    write(my_socket, buf, redundent);
    load_bar(packet_num, packet_num, 15);

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
                char send_file[9];
                strncpy(send_file, line, 8); send_file[8] = '\0';
                if (strcmp(send_file, "SENDFILE") == 0 || downloader.downloading) {
                    download(line, my_socket);
                }
                else {
                    //cout << "qq" << endl;
                    cout << line;
                }
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
    downloader.downloading = false;

    /* read the info */
    client(stdin, my_socket);
    close(my_socket);
    return 0;
}
