/* read one line */
#define BUF_SIZE 1024*16

ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t readline_r(int fd, void *vptr, size_t maxlen);
