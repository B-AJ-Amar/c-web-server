#include "http_handler.h"
#include "config.h"
#include "file_handler.h"
#include "http_parser.h"
#include "http_response.h"
#include "http_status.h"
#include "logger.h"
#include "middlewares.h"
#include "php_handler.h"
#include "proxy.h"
#include "sock.h"
#include "thread_pool.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void handle_http_request(int client_sock) {
    char *buffer      = get_thread_buffer();
    int   buffer_size = BUFFER_SIZE;
    int   readed_len;

    if (!buffer) {
        log_message(&lg, LOG_ERROR, "Failed to get thread buffer");
        send_500(client_sock, NULL);
        close(client_sock);
        return;
    }

    char *head_line = read_request_head_line(client_sock, buffer, buffer_size, &readed_len);

    http_request http_req;
    memset(&http_req, 0, sizeof(http_req));

    parse_request_line(head_line, &http_req);

    struct sockaddr_in client_addr;
    socklen_t          client_len = sizeof(client_addr);
    getpeername(client_sock, (struct sockaddr *)&client_addr, &client_len);
    http_req.client_addr = &client_addr;

    if (!http_req.is_invalid) {
        int route_index = path_router(cfg.routes, http_req.endpoint);

        if (route_index == -1) {
            // todo :  make it more clear
            log_message(&lg, LOG_WARN, "No matching route for %s", http_req.endpoint);
            send_404(client_sock, &http_req);
            close(client_sock);
            return;
        } else {

            if (!is_allowed_method(cfg.routes[route_index].methods, http_req.method)) {
                log_message(&lg, LOG_WARN, "Method %s not allowed for %s", http_req.method,
                            http_req.endpoint);
                send_405(client_sock, &http_req);
                close(client_sock);
                return;
            }

            if (cfg.routes[route_index].proxy_pass != NULL) {
                // ? proxy
                handle_proxy(client_sock, &http_req, &cfg.routes[route_index], buffer, buffer_size,
                             readed_len);

            } else {
                // ? http and static files
                http_req.file_path = get_file_path(http_req.uri, cfg.routes[route_index]);
                http_req.file_ext  = get_file_extension(http_req.file_path);

                if (is_php(http_req.file_ext)) {
                    FILE *req_data =
                        read_long_http_request(client_sock, buffer, buffer_size, &readed_len);
                    if (req_data) {
                        http_req.req_data = req_data;
                        http_req.use_file = true;
                    }
                    handle_php_request(client_sock, &http_req, cfg.server.php_cgi_path, buffer,
                                       &readed_len);

                    // send_status_response(client_sock,HTTP_NOT_IMPLEMENTED,&http_req);
                } else if (!strcmp(http_req.method, HTTP_GET)) {
                    // ? this must be GET
                    // ? for now i will not care about the headers
                    send_file_response(client_sock, &http_req, cfg.routes[route_index], buffer,
                                       buffer_size);
                } else
                    send_404(client_sock, &http_req);
            }
        }
    } else {
        log_message(&lg, LOG_ERROR, "something went wrong\n");
        send_500(client_sock, &http_req);
        close(client_sock);
        free_http_request(&http_req);
        return;
    }
    free_http_request(&http_req);
    close(client_sock);
}

void handle_http_req_task(void *args) {
    int client_sock = *((int *)args);
    free(args);
    handle_http_request(client_sock);
}