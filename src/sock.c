#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "logger.h"

#define HTTP_RESPONSE_502 "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 11\r\n\r\nBad Gateway"
#define HTTP_RESPONSE_500                                                                          \
    "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "                                       \
    "21\r\n\r\nInternal Server Error"

#define HTTP_RESPONSE_405 "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 18\r\n\r\nMethod Not Allowed"
#define HTTP_RESPONSE_404 "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found"



ssize_t send_500(int client_sock){
    return write(client_sock, HTTP_RESPONSE_500, strlen(HTTP_RESPONSE_500));
}

ssize_t send_502(int client_sock){
    return write(client_sock, HTTP_RESPONSE_502, strlen(HTTP_RESPONSE_502));
}
ssize_t send_404(int client_sock){
    return write(client_sock, HTTP_RESPONSE_404, strlen(HTTP_RESPONSE_404));
}

ssize_t send_405(int client_sock){
    return write(client_sock, HTTP_RESPONSE_405, strlen(HTTP_RESPONSE_405));
}





int send_file(int client_sock, const char *filepath, char *buffer, size_t buffer_size) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return 0;

    size_t n;
    while ((n = fread(buffer, 1, buffer_size, fp)) > 0) {
        size_t sent = 0;
        while (sent < n) {
            ssize_t s = send(client_sock, buffer + sent, n - sent, 0);
            if (s <= 0) {
                fclose(fp);
                return 0;
            }
            sent += s;
        }
    }

    fclose(fp);

    return 1;
}

