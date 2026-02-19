#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main() {
    char buffer[200];
    memset(buffer, 'a', 200);

    int fd = open("test.txt", O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd < 0) { perror("open"); return 1; }

    write(fd, buffer, 200);      // write 200 bytes
    lseek(fd, 0, SEEK_SET);      // go back to start
    read(fd, buffer, 100);       // read 100 bytes
    lseek(fd, 500, SEEK_CUR);    // move forward 500 bytes
    write(fd, buffer, 100);      // write 100 more bytes

    close(fd);
    return 0;
}
