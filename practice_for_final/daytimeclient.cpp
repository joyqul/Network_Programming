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
#include "../lib/readline.h"

using namespace std;

static void bail(string on_what) {
    cerr << strerror(errno) << ": " << on_what << "\n";
}

char* sock_ntop( const struct sockaddr *sa, socklen_t salen ) {
    char portstr[8];
    static char str[128]; /* Unix domain is largest */

    switch (sa->sa_family) {
        case AF_INET:
            {
                struct sockaddr_in *sin = (struct sockaddr_in *) sa;

                if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
                    return NULL;
                if (ntohs(sin->sin_port) != 0) {
                    snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
                    strcat(str, portstr);
                }
                return str;
            }
#ifdef IPV6
        case AF_INET6:
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;

                str[0] = '[';
                if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL)
                    return NULL;
                if (ntohs(sin6->sin6_port) != 0) {
                    snprintf(portstr, sizeof(portstr), "]:%d", ntohs(sin6->sin6_port));
                    strcat(str, portstr);
                    return str;
                }
                return str + 1;
            }
#endif
        default:
            snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
                    sa->sa_family, salen);
            return str;

    }
    return NULL;
}


int main (int argc, char **argv) {
    struct in_addr **pptr;
    struct in_addr *inetaddrp[2];
    struct in_addr inetaddr;
    struct hostent *hp;
    struct servent *sp;
    struct sockaddr_in server_socket_info; // in netinet/in.h
    int sockfd;

    if (argc != 3) {
        cout << "usage: ./a.out <hostname> <service>" << endl;
        return 0;
    }

    if ( (hp = gethostbyname(argv[1])) == NULL ) {
        if (inet_aton(argv[1], &inetaddr) == 0) { // afraid that user type IP addr
            cout << "hostname error for" << argv[1] << ": " << hstrerror(h_errno) << endl;
            return 0;
        }
        else {
            inetaddrp[0] = &inetaddr;
            inetaddrp[1] = NULL;
            pptr = inetaddrp;
        }
    }
    else {
        pptr = (struct in_addr **) hp->h_addr_list;
    }

    if ( (sp = getservbyname(argv[2], "tcp")) == NULL ) {
        cout << "getservbyname error for " << argv[2] << endl;
        return 0;
    }

    for (; *pptr != NULL; pptr++) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&server_socket_info, sizeof(server_socket_info));
        server_socket_info.sin_family = AF_INET;
        server_socket_info.sin_port = sp->s_port;
        memcpy(&server_socket_info.sin_addr, *pptr, sizeof(struct in_addr));
        cout << "trying " << sock_ntop((struct sockaddr *)&server_socket_info, sizeof(server_socket_info)) << endl;

        if (connect(sockfd, (struct sockaddr *)&server_socket_info, sizeof(server_socket_info)) == 0) {
            break; /* success */
        }

        cout << "connect error" << endl;
        close(sockfd);
    }

    if (*pptr == NULL) {
        bail("unable to connect");
    }
    
    char line[BUF_SIZE];
    int n;
    while ( (n = readline(sockfd, line, BUF_SIZE)) > 0) {
        cout << line << endl;
    }
}
