#include "proxy.h"
#include "config.h"
#include "logger.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

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
void handle_proxy(int client_sock,http_request *req, route_config *route, char *buffer, int buffer_size, int readed) {
    static int status_extracted = 0;
    static int extracted_status = 0;
    
    int backend_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (backend_sock < 0)
        return;

    struct sockaddr_in backend_addr = {0};
    backend_addr.sin_family         = AF_INET;
    backend_addr.sin_port           = htons(route->proxy->port ? route->proxy->port : 80);

    if (inet_pton(AF_INET, route->proxy->host, &backend_addr.sin_addr) <= 0) {
        struct hostent *he = gethostbyname(route->proxy->host);
        if (!he) {
            close(backend_sock);
            return;
        }
        memcpy(&backend_addr.sin_addr, he->h_addr_list[0], he->h_length);
    }

    if (connect(backend_sock, (struct sockaddr *)&backend_addr, sizeof(backend_addr)) < 0) {
        close(backend_sock);
        return;
    }

    char *new_uri = req->uri;
    size_t route_path_len = strlen(route->path);
    
    if (strncmp(req->uri, route->path, route_path_len) == 0) {
        new_uri = req->uri + route_path_len;
        if (new_uri[0] == '\0') {
            new_uri = "/";
        }
    }

    char new_request_line[1024];
    snprintf(new_request_line, sizeof(new_request_line), "%s %s %s\r\n", req->method, new_uri, req->version);
    

    ssize_t sent = 0;
    size_t new_line_len = strlen(new_request_line);
    while (sent < new_line_len) {
        ssize_t w = write(backend_sock, new_request_line + sent, new_line_len - sent);
        if (w <= 0) {
            close(backend_sock);
            return;
        }
        sent += w;
    }

    char *first_line_end = strstr(buffer, "\r\n");
    if (!first_line_end) {
        close(backend_sock);
        return;
    }

    size_t first_line_len = first_line_end - buffer;
    size_t remaining_len = readed - (first_line_len + 2);
    char *remaining_data = buffer + first_line_len + 2;
    
    sent = 0;
    while (sent < remaining_len) {
        ssize_t w = write(backend_sock, remaining_data + sent, remaining_len - sent);
        if (w <= 0) {
            close(backend_sock);
            return;
        }
        sent += w;
    }

    fd_set fds;
    int    maxfd = (client_sock > backend_sock ? client_sock : backend_sock) + 1;

    for (;;) {
        FD_ZERO(&fds);
        FD_SET(client_sock, &fds);
        FD_SET(backend_sock, &fds);

        int activity = select(maxfd, &fds, NULL, NULL, NULL);
        if (activity < 0)break;

        // client -> backend
        if (FD_ISSET(client_sock, &fds)) {
            ssize_t n = recv(client_sock, buffer, buffer_size, 0);
            if (n <= 0)  break;
            ssize_t sent = 0;
            while (sent < n) {
                ssize_t w = write(backend_sock, buffer + sent, n - sent);
                if (w <= 0) {
                    log_message(&lg, LOG_ERROR, "[handle_proxy] write to backend failed");
                    close(backend_sock);
                    return;
                }
                sent += w;
            }
        }

        // backend -> client
        if (FD_ISSET(backend_sock, &fds)) {
            ssize_t n = recv(backend_sock, buffer, buffer_size, 0);
            if (n <= 0) {
                log_message(&lg, LOG_INFO, "[handle_proxy] backend closed");
                break;
            }
            
            if (!status_extracted) {
                if (n >= 12) { 
                    char status_str[4];
                    strncpy(status_str, buffer + 9, 3);
                    status_str[3] = '\0';
                    
                    extracted_status = atoi(status_str);
                    http_log(&lg, req, extracted_status);
                    status_extracted = 1;
                }
            }
            
            ssize_t sent = 0;
            while (sent < n) {
                ssize_t w = write(client_sock, buffer + sent, n - sent);
                if (w <= 0) {
                    log_message(&lg, LOG_ERROR, "[handle_proxy] write to client failed");
                    close(backend_sock);
                    return;
                }
                sent += w;
            }
        }
    }
    
    close(backend_sock);
    return;
}
