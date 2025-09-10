#include "config.h"
#include "http_parser.h"
#include "logger.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define HTTP_RESPONSE_502 "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 11\r\n\r\nBad Gateway"
#define HTTP_RESPONSE_500                                                                          \
    "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "                                       \
    "21\r\n\r\nInternal Server Error"

#define HTTP_RESPONSE_405                                                                          \
    "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 18\r\n\r\nMethod Not Allowed"
#define HTTP_RESPONSE_404 "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found"

int init_socket(server_config *cfg) {
    struct sockaddr_in serv_addr;
    int                serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow reuse of local addresses
    int opt = 1;
    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(cfg->host);
    serv_addr.sin_port        = htons(cfg->port);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        close(serv_sock);
        exit(1);
    }

    listen(serv_sock, cfg->max_connections);
    return serv_sock;
}

ssize_t send_500(int client_sock, http_request *req) {
    ssize_t result = write(client_sock, HTTP_RESPONSE_500, strlen(HTTP_RESPONSE_500));
    if (req != NULL)
        http_log(&lg, req, 500);
    return result;
}

ssize_t send_502(int client_sock, http_request *req) {
    ssize_t result = write(client_sock, HTTP_RESPONSE_502, strlen(HTTP_RESPONSE_502));
    if (req != NULL)
        http_log(&lg, req, 502);
    return result;
}
ssize_t send_404(int client_sock, http_request *req) {
    ssize_t result = write(client_sock, HTTP_RESPONSE_404, strlen(HTTP_RESPONSE_404));
    if (req != NULL)
        http_log(&lg, req, 404);
    return result;
}

ssize_t send_405(int client_sock, http_request *req) {
    ssize_t result = write(client_sock, HTTP_RESPONSE_405, strlen(HTTP_RESPONSE_405));
    if (req != NULL)
        http_log(&lg, req, 405);
    return result;
}

int send_file(int client_sock, const char *filepath, char *buffer, size_t buffer_size) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp)
        return 0;

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
