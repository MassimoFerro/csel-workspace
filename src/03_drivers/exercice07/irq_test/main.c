#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

int main()
{
    int fd = open("/dev/mymodule", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    fd_set rfds;
    int count = 0;

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        int ret = select(fd + 1, &rfds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(fd, &rfds)) {
            count++;
            printf("Interruption %d reçue !\n", count);
            char dummy;
            read(fd, &dummy, 1);  // pour réarmer si besoin
        }
    }

    close(fd);
    return 0;
}
