#include <errno.h>

int _close(int file) {
    (void)file;
    errno = EBADF;
    return -1;
}

int _lseek(int file, int offset, int whence) {
    (void)file;
    (void)offset;
    (void)whence;
    errno = ESPIPE;
    return -1;
}

int _read(int file, char *buffer, int length) {
    (void)file;
    (void)buffer;
    (void)length;
    errno = EBADF;
    return -1;
}

int _write(int file, char *buffer, int length) {
    (void)file;
    (void)buffer;
    (void)length;
    errno = EBADF;
    return -1;
}
