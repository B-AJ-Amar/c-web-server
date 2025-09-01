#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http_parser.h"
#include "http_response.h"

#define PORT        8080
#define BUFFER_SIZE 4096

#define HTTP_RESPONSE_500                                                                          \
    "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "                                       \
    "21\r\n\r\nInternal Server Error"

void init_serv() {
    if (!init_uri_regex())
        exit(EXIT_FAILURE);
}

void kill_serv(int serv_sock) {
    free_uri_regex();
    close(serv_sock);
}

int main() {

    init_serv();

    struct sockaddr_in serv_addr;
    char               buffer[BUFFER_SIZE];
    int                serv_sock, client_sock;

    
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
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Any local IP
    serv_addr.sin_port        = htons(PORT);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        close(serv_sock);
        exit(1);
    }

    listen(serv_sock, 5);
    printf("Server listening on port %d...\n", PORT);
    while (1) {
        client_sock = accept(serv_sock, NULL, NULL);

        if (client_sock < 0) {
            perror("ERROR on accept");
            close(serv_sock);
            exit(1);
        }

        int n     = read(client_sock, buffer, sizeof(buffer));
        buffer[n] = '\0';
        printf("New Message : \n\n%s\n===================================\n", buffer);

        printf("parsing the http request ...\n");

        http_request http_req;
        memset(&http_req, 0, sizeof(http_req));
        
        parse_http_request(buffer, &http_req);

        if (http_req.is_invalid == 0) {
            printf("parsed successfuly\n");
            printf("method : %s , uri : %s , endpoint : %s \n", http_req.method, http_req.uri,
                   http_req.endpoint);
            printf("headers count: %d \n", http_req.headers_count);

            printf("quary_params_count : %d \n", http_req.quary_params_count);
            http_response http_res;
            memset(&http_res, 0, sizeof(http_res));
            generateFileResponse(&http_req,&http_res);

            char* response = parseResponse(&http_res);

            write(client_sock, response, strlen(response));

        } else {
            printf("something went wrong\n");
            write(client_sock, HTTP_RESPONSE_500, strlen(HTTP_RESPONSE_500));
        }

        close(client_sock);
    }

    kill_serv(serv_sock);
    return 0;
}