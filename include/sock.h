#ifndef SOCK_H
#define SOCK_H

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "http_parser.h"

int init_socket(server_config *cfg);

ssize_t send_500(int client_sock, http_request *req);
ssize_t send_502(int client_sock, http_request *req);
ssize_t send_404(int client_sock, http_request *req);
ssize_t send_405(int client_sock, http_request *req);

int send_file(int client_sock, const char *filepath, char *buffer, size_t buffer_size);
#endif // SOCK_H