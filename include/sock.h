#ifndef SOCK_H
#define SOCK_H

#include "config.h"
#include "http_parser.h"
#include <stdio.h>
#include <stdlib.h>

int init_socket(server_config *cfg);

int send_file(int client_sock, const char *filepath, char *buffer, size_t buffer_size);
char* read_request_head_line(int client_sock, char *buffer, int buffer_size,int* readed_len);
#endif // SOCK_H