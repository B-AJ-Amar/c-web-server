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


char* read_request_head_line(int client_sock, char *buffer, int buffer_size,int* readed_len) {

    ssize_t n = read(client_sock, buffer , buffer_size);
    if (n <= 0) return NULL;
    
    buffer[n] = '\0';
    *readed_len = n;
    char* find_endl = strstr(buffer, "\r\n");
    if (find_endl) {
        char *head_line = malloc(sizeof(find_endl-buffer) + 1);
        if (head_line) {
            strncpy(head_line, buffer, find_endl-buffer);
            head_line[find_endl-buffer] = '\0';
        }
        return head_line;
    }

    return NULL;
}

FILE* read_http_request(int client_sock, char *buffer, int buffer_size,int readed_len){
    if (buffer_size > readed_len) NULL;

    FILE* req_file = tmpfile();
    

}