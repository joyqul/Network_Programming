#include <iostream>
#include <vector>
#include <queue>
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
#include <pthread.h>
#include "../lib/readline.h"

#define QUE_SIZE 10

using namespace std;

static void bail(string on_what) {
    cerr << strerror(errno) << ": " << on_what << "\n";
}

static void str_echo(int myclient) {
    ssize_t n;
    char line[BUF_SIZE];
    bzero(line, BUF_SIZE);

    while (1) {
        if ( (n = readline(myclient, line, BUF_SIZE)) == 0) {
            return; /* connection closed by other end */
        }
        int check = write(myclient, line, sizeof(line));
        /* error */
        if (check == -1) {
            bail("write() in str_echo");
            return;
        }
    }
}

static void * doit(void *arg) {
    pthread_detach(pthread_self()); // detech myself immidiately
    long long myarg = (long long)arg;
    str_echo(myarg);
    close(myarg);
    return (NULL);
}

int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp) {

    int listenfd, n;
    const int on = 1;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
        cout << "tcp_listen error for " << host << ", " << serv << ": " << gai_strerror(n) << endl;
    }
    ressave = res;

    do {
        listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listenfd < 0)
            continue;       /* error, try next one */

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
            break;          /* success */

        close(listenfd);    /* bind error, close and try next one */
    } while ( (res = res->ai_next) != NULL);

    if (res == NULL)    /* errno from final socket() or bind() */ {
        cout << "tcp_listen error for " << host << ", " << serv << endl;
    }

    listen(listenfd, QUE_SIZE);

    if (addrlenp)
        *addrlenp = res->ai_addrlen;    /* return size of protocol address */

    freeaddrinfo(ressave);

    return(listenfd);
}

int main(int argc, char* argv[]) {
    int listenfd, connfd;
    pthread_t tid;
    socklen_t addrlen, len;
    struct sockaddr *cliaddr;

    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1], &addrlen);
    }
    else if (argc == 3) {
        listenfd = tcp_listen(argv[1], argv[2], &addrlen);
    }
    else {
        cout << "Usage: tcpserv01 [<host>] <service or port>" << endl;
        return 0;
    }

    cliaddr = (struct sockaddr *)malloc(addrlen);

    while (1) {
        len = addrlen;
        connfd = accept(listenfd, cliaddr, &len);
        pthread_create(&tid, NULL, &doit, (void *) connfd);
    }
}
