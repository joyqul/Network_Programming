#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include "readline.h"

static pthread_key_t rl_key;
static pthread_once_t rl_once = PTHREAD_ONCE_INIT;

static void readline_destructor(void *ptr) {
    free(ptr);
}

static void readline_once(void) {
    pthread_key_create(&rl_key, readline_destructor);
}

typedef struct {
    int rl_cnt;
    char *rl_bufptr;
    char rl_buf[BUF_SIZE];
} rline;

static ssize_t my_read_r(rline *tsd, int fd, char *ptr) {
    while (tsd->rl_cnt <= 0) {
        if ( (tsd->rl_cnt = read(fd, tsd->rl_buf, BUF_SIZE)) < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        else if (tsd->rl_cnt == 0) {
            return 0;
        }
        tsd->rl_bufptr = tsd->rl_buf;
        break;
    }
    tsd->rl_cnt--;
    *ptr = *tsd->rl_bufptr++;
    return 1;
}

ssize_t readline_r(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;
    rline *tsd;

    pthread_once(&rl_once, readline_once);
    if ( (tsd = (rline *)pthread_getspecific(rl_key)) == NULL ) {
        tsd = (rline *)calloc(1, sizeof(rline));
        pthread_setspecific(rl_key, tsd);
    }
    
    ptr = (char *)vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = my_read_r(tsd, fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0) {
            *ptr = 0;
            return (n - 1);
        }
        else
            return -1;
    }
    *ptr = 0;
    return n;
}

static int read_cnt;
static char *read_ptr;
static char read_buf[BUF_SIZE];
static ssize_t my_read(int fd, char *ptr) {
    while (read_cnt <= 0) {
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0 ) {
            if (errno == EINTR) continue;
            return -1;
        }
        else if (read_cnt == 0) {
            return 0;
        }
        read_ptr = read_buf;
        break;
    }
    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c;
    char *ptr = (char *)vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0) {
            *ptr = 0;
            return (n - 1);
        }
        else
            return -1;
    }
    *ptr = 0;
    return n;
}
