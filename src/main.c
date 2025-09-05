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

#define HTTP_RESPONSE_500                                                                          \
    "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "                                       \
    "21\r\n\r\nInternal Server Error"

#define HTTP_RESPONSE_404 "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found"
#define HTTP_RESPONSE_502 "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 11\r\n\r\nBad Gateway"
void init_serv() {

    if (!init_uri_regex())
        exit(EXIT_FAILURE);

    if (!load_config("./cws.conf", &cfg))
    exit(EXIT_FAILURE);

    for (int i = 0; cfg.routes[i].path != NULL; i++) {
        if (cfg.routes[i].proxy_pass != NULL){
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

    struct sockaddr_in serv_addr;
    char               buffer[cfg.server.sock_buffer_size];
    int                serv_sock, client_sock, route_index;
    struct sockaddr_in client_addr;
    socklen_t          client_len = sizeof(client_addr);

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow reuse of local addresses
    int opt = 1;
    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(cfg.server.host);
    serv_addr.sin_port        = htons(cfg.server.port);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        close(serv_sock);
        exit(1);
    }

    listen(serv_sock, cfg.server.max_connections);
    log_message(&lg, LOG_INFO, "Server listening on port %d...", cfg.server.port);
    while (1) {
        client_sock = accept(serv_sock, (struct sockaddr *)&client_addr, &client_len);

        if (client_sock < 0) {
            log_message(&lg, LOG_ERROR, "ERROR on accept");
            close(serv_sock);
            exit(1);
        }

        int n     = read(client_sock, buffer, sizeof(buffer));
        buffer[n] = '\0';

        char buf2[sizeof(buffer)];

        strcpy(buf2,buffer);

        log_message(&lg, LOG_DEBUG, " client request : %s",buffer);

        http_request http_req;
        memset(&http_req, 0, sizeof(http_req));

        http_req.client_addr = &client_addr;

        parse_http_request(buffer, &http_req);

        if (1) {

            route_index = path_router(cfg.routes, &http_req);
            
            if (route_index == -1) {
                // todo :  make it more clear
                log_message(&lg, LOG_WARN, "No matching route for %s", http_req.endpoint);
                write(client_sock, HTTP_RESPONSE_404, strlen(HTTP_RESPONSE_404));
                close(client_sock);
                continue;
            }
            else{
                // handel allowed methods
                if (cfg.routes[route_index].proxy_pass != NULL)
                {
                    int status = handle_proxy(client_sock,&cfg.routes[route_index],buf2);

                    if (status != 0)
                    {
                        log_message(&lg, LOG_WARN, "Bad Gateway to %s", cfg.routes[route_index].proxy_pass);
                        write(client_sock, HTTP_RESPONSE_502, strlen(HTTP_RESPONSE_502));
                    }
                }
                else
                {
                    
                    http_response http_res;
                    memset(&http_res, 0, sizeof(http_res));
                    generateFileResponse(&http_req, &http_res,cfg.routes[route_index]);

                    char *response = parseResponse(&http_res);

                    write(client_sock, response, strlen(response));

                    http_log(&lg, &http_req, &http_res);
                }
                
                

            }


        } else {
            log_message(&lg, LOG_ERROR, "something went wrong\n");
            write(client_sock, HTTP_RESPONSE_500, strlen(HTTP_RESPONSE_500));
        }

        close(client_sock);
    }

    kill_serv(serv_sock);
    return 0;
}