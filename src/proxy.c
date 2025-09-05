#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config.h"
#include "logger.h"
#include "proxy.h"

int parse_proxy(route_config *route) {
    if (route->proxy_pass) {
        proxy_info *proxy = malloc(sizeof(proxy_info));
        if (!proxy) {
            log_message(&lg, LOG_FATAL, "Failed to allocate memory for proxy_info");
            return 0;
        }

        char *url_copy = strdup(route->proxy_pass);
        if (!url_copy) {
            free(proxy);
            log_message(&lg, LOG_FATAL, "Failed to allocate memory for URL copy");
            return 0;
        }

        char *scheme = strtok(url_copy, ":"); // "http"
        char *host   = strtok(NULL, ":");     // "//127.0.0.1"
        char *port   = strtok(NULL, ":");

        if (host && strncmp(host, "//", 2) == 0) {
            host += 2;
        }

        proxy->scheme = scheme ? strdup(scheme) : NULL;
        proxy->host   = host ? strdup(host) : NULL;

        if (port)
            proxy->port = atoi(port);
        else if (scheme && strcmp(scheme, "https") == 0)
            proxy->port = 443;
        else
            proxy->port = 80;

        route->proxy = proxy;

        free(url_copy);
    }
    return 1;
}

int handle_proxy(int client_sock, route_config *route, char *client_request) {
    int                backend_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in backend_addr;
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port   = htons(route->proxy->port ? route->proxy->port : 80);
    inet_pton(AF_INET, route->proxy->host, &backend_addr.sin_addr);

    if (inet_pton(AF_INET, route->proxy->host, &backend_addr.sin_addr) <= 0) {
        struct hostent *he = gethostbyname(route->proxy->host);
        if (!he) {
            close(backend_sock);
            return 1;
        }
        memcpy(&backend_addr.sin_addr, he->h_addr_list[0], he->h_length);
    }

    if (connect(backend_sock, (struct sockaddr *)&backend_addr, sizeof(backend_addr)) < 0)
        return 1;

    write(backend_sock, client_request, strlen(client_request));

    char proxy_buf[8192];
    int  n = recv(backend_sock, proxy_buf, sizeof(proxy_buf), MSG_WAITALL);
    if (n < 0) {
        close(backend_sock);
        return 1;
    }

    write(client_sock, proxy_buf, n);

    close(backend_sock);

    return 0;
}