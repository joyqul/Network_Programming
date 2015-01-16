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

using namespace std;

static void bail(string on_what) {
    cerr << strerror(errno) << ": " << on_what << "\n";
}

int main (int argc, char **argv) {
    char *ptr, **pptr;
    char str[INET_ADDRSTRLEN];

    struct hostent *hptr;
    
    while (--argc > 0) {
        ptr = *++argv;
        if ((hptr = gethostbyname(ptr)) == NULL) {
            char msg[100];
            sprintf(msg, "gethostbyname error for host: %s: %s", ptr, hstrerror(h_errno));
            bail(msg);
            continue;
        }
        printf("official hostname: %s\n", hptr->h_name);
        for (pptr = hptr->h_aliases; *pptr != NULL; pptr++) {
            printf("\talias: %s\n", *pptr);
        }

        switch (hptr->h_addrtype) {
            case AF_INET:
                pptr = hptr->h_addr_list;
                for (; *pptr != NULL; pptr++) {
                    printf("\taddress: %s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
                }
                break;
            default:
                bail("unknow address type");
                break;
        }
    }
    return 0;
}
