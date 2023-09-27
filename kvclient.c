#include "kvclient.h"
#include "functions.h"

int kvclient(int argc, char* argv[], char *res, int newsock) {
    if (argc < 3) {
        printf("usage: ./client <fifo name> \"message to send\"\n");
        exit(-1);
    }
    pid_t pid = getpid();
    char PID[6];
    sprintf(PID, "%d", pid);
    int fifo_fd;
    char *server_fifo = argv[1];
    char *getorset = argv[2];
    char *key = argv[3];
    char *value = argv[4];
    char buf[BUFSIZ];
    memset(buf, 0, sizeof(buf));
    fifo_fd = open(server_fifo, O_RDWR);
    if (fifo_fd == -1) {
        sendHeader(newsock, 500, 0);
        exit(1);
    }
    int fifo_fd2;
    char *server_fifo2 = PID;
    char buf2[BUFSIZ];

    char str[1059];
    char *g = "get";
    if (strcmp(getorset, g) == 0) {
        sprintf(str, "%s,%s,%s\n", getorset,PID, key);
        int n = write(fifo_fd, str, strlen(str));
        fifo_fd2 = open(server_fifo2, O_RDONLY);
        if (read(fifo_fd2, buf2, BUFSIZ)>0) {
            sprintf(res, "%s\n", buf2);
        } else {
            sendHeader(newsock, 500, 0);
        }
        close(fifo_fd2);
        unlink(server_fifo2);
    } else {
        sprintf(str, "%s,%s,%s\n", getorset,key, value);
        int n = write(fifo_fd, str, strlen(str));
    }

    close(fifo_fd);
    return 0;
}
