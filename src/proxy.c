#include "proxy.h"
#include "config.h"
#include "logger.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>     
#include <errno.h>          

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

//  todo : use epoll of better performance
int handle_proxy(int client_sock, route_config *route, char* buffer, int buffer_size, int readed) {
    int backend_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (backend_sock < 0) return 1;


    struct sockaddr_in backend_addr = {0};
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port   = htons(route->proxy->port ? route->proxy->port : 80);

    if (inet_pton(AF_INET, route->proxy->host, &backend_addr.sin_addr) <= 0) {
        struct hostent *he = gethostbyname(route->proxy->host);
        if (!he) {
            close(backend_sock);
            return 1;
        }
        memcpy(&backend_addr.sin_addr, he->h_addr_list[0], he->h_length);
    }

    if (connect(backend_sock, (struct sockaddr *)&backend_addr, sizeof(backend_addr)) < 0) {
        close(backend_sock);
        return 1;
    }

    ssize_t sent = 0;
    while (sent < readed) {
        ssize_t w = write(backend_sock, buffer + sent, readed - sent);
        if (w <= 0) {
            close(backend_sock);
            return 1;
        }
        sent += w;
    }


    fd_set fds;
    int maxfd = (client_sock > backend_sock ? client_sock : backend_sock) + 1;

    for (;;) {
        log_message(&lg, LOG_DEBUG, "[handle_proxy_new] select : loop");
        FD_ZERO(&fds);
        FD_SET(client_sock, &fds);
        FD_SET(backend_sock, &fds);

        int activity = select(maxfd, &fds, NULL, NULL, NULL);
        if (activity < 0) {
            log_message(&lg, LOG_ERROR, "[handle_proxy_new] select() failed");
            break;
        }

        // client -> backend
        if (FD_ISSET(client_sock, &fds)) {
            ssize_t n = recv(client_sock, buffer, buffer_size, 0);
            if (n <= 0) {
                log_message(&lg, LOG_INFO, "[handle_proxy_new] client closed");
                break;
            }
            log_message(&lg, LOG_DEBUG, "[handle_proxy_new] client->backend n=%zd", n);
            ssize_t sent = 0;
            while (sent < n) {
                ssize_t w = write(backend_sock, buffer + sent, n - sent);
                if (w <= 0) {
                    log_message(&lg, LOG_ERROR, "[handle_proxy_new] write to backend failed");
                    goto cleanup;
                }
                sent += w;
            }
        }

        // backend -> client
        if (FD_ISSET(backend_sock, &fds)) {
            ssize_t n = recv(backend_sock, buffer, buffer_size, 0);
            if (n <= 0) {
                log_message(&lg, LOG_INFO, "[handle_proxy_new] backend closed");
                break;
            }
            log_message(&lg, LOG_DEBUG, "[handle_proxy_new] backend->client n=%zd", n);
            ssize_t sent = 0;
            while (sent < n) {
                ssize_t w = write(client_sock, buffer + sent, n - sent);
                if (w <= 0) {
                    log_message(&lg, LOG_ERROR, "[handle_proxy_new] write to client failed");
                    goto cleanup;
                }
                sent += w;
            }
        }
    }

cleanup:
    close(backend_sock);
    return 0;
}

