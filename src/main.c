#include "config.h"
#include "http_handler.h"
#include "http_parser.h"
#include "http_response.h"
#include "logger.h"
#include "middlewares.h"
#include "sock.h"
#include "thread_pool.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

        add_task(queue, handle_http_req_task, client_sock_ptr);
    }

    kill_serv(serv_sock);
    return 0;
}
