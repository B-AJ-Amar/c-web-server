#include <arpa/inet.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config.h"
#include "http_parser.h"
#include "http_response.h"
#include "logger.h"
#include "middlewares.h"
#include "sock.h"
#include "thread_pool.h"

void init_serv() {

    if (!init_uri_regex())
        exit(EXIT_FAILURE);

    if (!load_config("./cws.conf", &cfg))
        exit(EXIT_FAILURE);

    for (int i = 0; cfg.routes[i].path != NULL; i++) {
        if (cfg.routes[i].proxy_pass != NULL) {
            if (!parse_proxy(&cfg.routes[i])) {
                exit(EXIT_FAILURE);
            }
        }
    }

    if (!init_logger(&lg, cfg.logging.level, 1, NULL, NULL))
        exit(EXIT_FAILURE);
}

void kill_serv(int serv_sock) {
    free_uri_regex();
    close(serv_sock);
    if (cfg.logging.output && cfg.logging.output != stdout && cfg.logging.output != stderr)
        fclose(cfg.logging.output);
}

void handle_http_request(int client_sock) {
    char *buffer      = get_thread_buffer();
    int   buffer_size = cfg.server.sock_buffer_size;

    if (!buffer) {
        log_message(&lg, LOG_ERROR, "Failed to get thread buffer");
        send_500(client_sock, NULL);
        close(client_sock);
        return;
    }

    int n     = read(client_sock, buffer, buffer_size - 1);
    buffer[n] = '\0';

    char buf2[n + 1];
    strcpy(buf2, buffer); // tofix

    http_request http_req;
    memset(&http_req, 0, sizeof(http_req));

    struct sockaddr_in client_addr;
    socklen_t          client_len = sizeof(client_addr);
    getpeername(client_sock, (struct sockaddr *)&client_addr, &client_len);
    http_req.client_addr = &client_addr;

    parse_http_request(buffer, &http_req);

    if (!http_req.is_invalid) {
        int route_index = path_router(cfg.routes, http_req.endpoint);

        if (route_index == -1) {
            // todo :  make it more clear
            log_message(&lg, LOG_WARN, "No matching route for %s", http_req.endpoint);
            send_404(client_sock, &http_req);
            close(client_sock);
            return;
        } else {
            int is_allowed = is_allowed_method(cfg.routes[route_index].methods, http_req.method);
            if (!is_allowed) {
                log_message(&lg, LOG_WARN, "Method %s not allowed for %s", http_req.method,
                            http_req.endpoint);
                send_405(client_sock, &http_req);
                close(client_sock);
                return;
            }

            if (cfg.routes[route_index].proxy_pass != NULL) {
                log_message(&lg, LOG_INFO, "Serving %s for %s", cfg.routes[route_index].root,
                            http_req.uri);
                int status = handle_proxy(client_sock, &cfg.routes[route_index], buf2);

                if (status != 0) {
                    log_message(&lg, LOG_WARN, "Bad Gateway to %s",
                                cfg.routes[route_index].proxy_pass);
                    send_502(client_sock, &http_req);
                }
            } else {
                send_file_response(client_sock, &http_req, cfg.routes[route_index], buffer,
                                   buffer_size);
            }
        }
    } else {
        log_message(&lg, LOG_ERROR, "something went wrong\n");
        send_500(client_sock, &http_req);
        close(client_sock);
        return;
    }

    close(client_sock);
}

void handle_client_task(void *args) {
    int client_sock = *((int *)args);
    free(args);
    handle_http_request(client_sock);
}

int main() {

    init_serv();

    task_queue *queue   = create_task_queue();
    pthread_t  *threads = init_thread_pool(cfg.server.workers, queue);
    if (!queue || !threads) {
        log_message(&lg, LOG_ERROR, "Failed to initialize thread pool");
        exit(EXIT_FAILURE);
    }
    log_message(&lg, LOG_INFO, "Thread pool initialized with %d threads", cfg.server.workers);

    int                serv_sock, client_sock;
    struct sockaddr_in client_addr;
    socklen_t          client_len = sizeof(client_addr);

    serv_sock = init_socket(&cfg.server);
    log_message(&lg, LOG_INFO, "Server listening on port %d...", cfg.server.port);

    while (1) {
        client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &client_len);

        if (client_sock < 0) {
            log_message(&lg, LOG_ERROR, "ERROR on accept");
            continue;
        }

        int *client_sock_ptr = malloc(sizeof(int));
        if (!client_sock_ptr) {
            log_message(&lg, LOG_ERROR, "Failed to allocate memory for client socket");
            close(client_sock);
            continue;
        }
        *client_sock_ptr = client_sock;

        add_task(queue, handle_client_task, client_sock_ptr);
    }

    kill_serv(serv_sock);
    return 0;
}
