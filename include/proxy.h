#ifndef PROXY_H
#define PROXY_H

struct route_config;

typedef struct proxy_info {
    char *scheme; // "http"
    char *host;   // "127.0.0.1"
    int   port;   // 5000
} proxy_info;

int parse_proxy(struct route_config *route);
int handle_proxy(int client_sock,struct route_config *route, char* buffer, int buffer_size,int readed);

#endif // PROXY_H