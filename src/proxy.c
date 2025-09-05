#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "proxy.h"
#include "config.h"
#include "logger.h"

int parse_proxy(route_config *route) {
    if (route->proxy_pass) {
        proxy_info *proxy = malloc(sizeof(proxy_info));
        if (!proxy) {
            log_message(&lg,LOG_FATAL,"Failed to allocate memory for proxy_info");
            return 0;
        }

        char *url_copy = strdup(route->proxy_pass);
        if (!url_copy) {
            free(proxy);
            log_message(&lg,LOG_FATAL,"Failed to allocate memory for URL copy");
            return 0;
        }

        char *scheme = strtok(url_copy, ":");     // "http"
        char *host   = strtok(NULL, ":");          // "//127.0.0.1"
        char *port   = strtok(NULL, ":");          // "5000" (may be NULL)

        if (host && strncmp(host, "//", 2) == 0) {
            host += 2;
        }

        proxy->scheme = scheme ? strdup(scheme) : NULL;
        proxy->host   = host ? strdup(host) : NULL;
        proxy->port   = port ? strdup(port) : NULL;

        route->proxy = proxy;
        
        // Free the temporary copy
        free(url_copy);
    }
    return 1;
}



int handle_proxy(int client_sock,route_config *route,char* client_request){
    int backend_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in backend_addr;
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port   = htons(route->proxy->port ? atoi(route->proxy->port) : 80);
    inet_pton(AF_INET, route->proxy->host, &backend_addr.sin_addr);

    log_message(&lg,LOG_DEBUG,"[handle_proxy] CLIENTS message : %s",client_request);

    if (connect(backend_sock, (struct sockaddr*)&backend_addr, sizeof(backend_addr)) < 0) {
        log_message(&lg, LOG_DEBUG, "[handle_proxy]Failed to connect to proxy backend %s:%s", route->proxy->host, route->proxy->port ? route->proxy->port : "80");
        return 1;
    }

    log_message(&lg, LOG_DEBUG, "[handle_proxy] Connected to proxy backend %s:%s , Sending request", route->proxy->host, route->proxy->port ? route->proxy->port : "80");
    write(backend_sock, client_request, strlen(client_request));
    log_message(&lg, LOG_DEBUG, "[handle_proxy] Request sent to backend, waiting for response");    

    char proxy_buf[8192];
    int n = recv(backend_sock, proxy_buf, sizeof(proxy_buf),MSG_WAITALL);
    log_message(&lg, LOG_DEBUG, "[handle_proxy] Read %d bytes from backend", n);
    if (n < 0) {
        log_message(&lg, LOG_DEBUG, "[handle_proxy] Failed to read response from backend");
        close(backend_sock);
        return 1;
    }
    log_message(&lg, LOG_DEBUG, "[handle_proxy] forwarding to client");

    write(client_sock, proxy_buf, n);

    close(backend_sock);

    return 0;

}