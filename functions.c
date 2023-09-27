#include "functions.h"

int sendHeader(int socket, int error, int length) {
    char *errstr;
    if (error == 200) { errstr = "200 OK"; }
    else if (error == 400) { errstr = "400 Bad Request"; }
    else if (error == 403) { errstr = "403 Permission Denied"; }
    else if (error == 404) { errstr = "404 Not Found"; }
    else if (error == 500) { errstr = "500 Internal Error"; }
    else if (error == 501) { errstr = "501 Not Implemented"; }

    char *str = malloc(100);
    if (str == NULL) {
        sendHeader(socket, 500, 0);
    }
    if (error == 200) {
        sprintf(str, "HTTP/1.1 %s\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: %d\r\n"
                     "\r\n", errstr, length);
    } else {
        sprintf(str, "HTTP/1.1 %s\r\n"
                     "Content-Type: text/html\r\n"
                     "\r\n", errstr);
    }

    send(socket, str, strlen(str), 0); // Send to client
    free(str);
    return 0;
}
