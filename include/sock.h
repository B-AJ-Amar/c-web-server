#ifndef SOCK_H
#define SOCK_H

#include <stdio.h>
#include <stdlib.h>

ssize_t send_500(int client_sock);

ssize_t send_502(int client_sock);
ssize_t send_404(int client_sock);

ssize_t send_405(int client_sock);




int send_file(int client_sock, const char *filepath, char *buffer, size_t buffer_size);
#endif // SOCK_H