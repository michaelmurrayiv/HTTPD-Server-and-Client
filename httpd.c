#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "functions.h"
#include "kvclient.h"

#define DEFAULT_BACKLOG 100

int main(int argc, char *argv[]) {
    int mlen, sock_fd, newsock, port;
    char *fifo = argv[1];
    sscanf(argv[2], "%d", &port);

    struct sockaddr_in sa, newsockinfo, peerinfo;
    socklen_t len;
    char *head = "HEAD";
    char *get = "GET";
    char *put = "PUT";

    char localaddr[INET_ADDRSTRLEN], peeraddr[INET_ADDRSTRLEN], buff[1001];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); // Create a socket

    sa.sin_family = AF_INET; // Set address family
    sa.sin_port = htons(port); // Set port number
    sa.sin_addr.s_addr = htonl(INADDR_ANY); // Set IP address to any available

    bind(sock_fd, (struct sockaddr *) &sa, sizeof(sa)); // Bind the socket to a specific address and port
    listen(sock_fd, DEFAULT_BACKLOG); // Start listening for incoming connections

    while(1) {
        len = sizeof(newsockinfo);
        newsock = accept(sock_fd, (struct sockaddr *) &peerinfo, &len); // Accept a new incoming connection
        pid_t pid;
        int status;
        if ((pid = fork()) < 0) {
            sendHeader(newsock, 500, 0);
        }
        if (pid == 0) { // In the child process

            len = sizeof(newsockinfo);
            getsockname(newsock, (struct sockaddr *) &newsockinfo, &len);
            inet_ntop(AF_INET, &newsockinfo.sin_addr.s_addr, localaddr, sizeof(localaddr));
            inet_ntop(AF_INET, &peerinfo.sin_addr.s_addr, peeraddr, sizeof(peeraddr));

            mlen = recv(newsock, buff, sizeof(buff), 0); // get data from client
            write(STDOUT_FILENO,buff,mlen); // Write http request to standard output

            char command[10], file[256], key[256], value[BUFSIZ]; // get command and file path
            sscanf(buff, "%s /%s", command, file);


            if (strlen(file)>=3 && strncmp(file, "../", 3) == 0) {
                sendHeader(newsock, 403, 0);
            } else if (*file == '\0') {
                sendHeader(newsock, 400, 0);
            }

            if (strcmp(command, head) == 0) { // head command
                sendHeader(newsock, 200, 0);

            } else if (strcmp(command, get) == 0) { // get command
                sscanf(buff, "%s /kv/%s", command, key);

                if (*key == '\0') { // getting a file
                    struct stat fStat; // get file length
                    if (stat(file, &fStat) == -1) {
                        sendHeader(newsock, 500, 0);
                    }
                    int length = fStat.st_size;

                    sendHeader(newsock, 200, length);

                    FILE *fp = fopen(file, "r"); // send file contents to client
                    if (fp == NULL) {
                        sendHeader(newsock, 404, 0);
                    }
                    char *buffer = malloc(length+1);
                    if (buffer == NULL) {
                        sendHeader(newsock, 500, 0);
                    }
                    fread(buffer, 1, length, fp);
                    buffer[length] = '\0';
                    send(newsock, buffer, length, 0); // Send the file contents as a string
                    free(buffer);
                    fclose(fp);

                } else { // getting from kvstore
                    char *args[5];
                    args[0] = "space";
                    args[1] = fifo;
                    args[2] = "get";
                    args[3] = key;
                    args[4] = "space";
                    char str[BUFSIZ];
                    memset(str, 0, sizeof(str));
                    kvclient(5, args, str, newsock);
                    if (strlen(str) > 0) {
                        sendHeader(newsock, 200, 0);
                        send(newsock, str, strlen(str), 0);
                    } else {
                        sendHeader(newsock, 404, 0);
                    }
                }


            } else if (strcmp(command, put) == 0) { // put command
                sscanf(buff, "%s /kv/%s", command, key);
                char *tmp = strstr(buff, "\r\n\r\n");
                memset(value, 0, sizeof(value));
                strncpy(value, tmp+4, sizeof(value)-1);

                char *args[5];
                args[0] = "space";
                args[1] = fifo;
                args[2] = "set";
                args[3] = key;
                args[4] = value;
                char str[BUFSIZ];
                memset(str, 0, sizeof(str));
                kvclient(5, args, str, newsock);
                sprintf(str, "HTTP/1.1 200 OK\r\n"
                             "Content-Length: 0\r\n"
                             "\r\n");
                send(newsock, str, strlen(str), 0); // Send to client

            } else { // invalid command
                sendHeader(newsock, 501, 0);
            }
            shutdown(newsock, SHUT_RDWR); // Shutdown the new connection (client end)
            exit(0); // Exit the child process
        } else {
            if (waitpid(pid, &status, 0) < 0) {
                sendHeader(newsock, 500, 0);
            }



        }
    }
    shutdown(sock_fd, SHUT_RDWR); // Shutdown the server socket

    return 0;
}
