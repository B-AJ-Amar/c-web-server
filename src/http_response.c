#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "file_handler.h"
#include "http_parser.h"
#include "http_response.h"
#include "http_status.h"
#include "sock.h"

char *get_http_date() {
    time_t    now = time(NULL);
    struct tm gm_time;
    gmtime_r(&now, &gm_time);

    char *buffer = malloc(100);
    if (!buffer)
        return NULL;

    strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", &gm_time);
    return buffer;
}

const char *get_content_type(const char *ext) {
    if (strcmp(ext, "html") == 0)
        return CONTENT_TYPES.html;
    else if (strcmp(ext, "css") == 0)
        return CONTENT_TYPES.css;
    else if (strcmp(ext, "js") == 0)
        return CONTENT_TYPES.js;
    else if (strcmp(ext, "json") == 0)
        return CONTENT_TYPES.json;
    else if (strcmp(ext, "png") == 0)
        return CONTENT_TYPES.png;
    else if (strcmp(ext, "jpg") == 0)
        return CONTENT_TYPES.jpg;
    else if (strcmp(ext, "gif") == 0)
        return CONTENT_TYPES.gif;
    else if (strcmp(ext, "svg") == 0)
        return CONTENT_TYPES.svg;
    else
        return CONTENT_TYPES.txt;
}

int send_file_response(int client_sock, http_request *req, route_config router, char *buffer,
                       int buffer_size) {

    char       *file_path      = get_file_path(req->uri, router);
    char       *temp_file_path = strdup(file_path);
    struct stat st;
    if (stat(file_path, &st) < 0) {
        // if (router.autoindex && req->uri[strlen(req->uri) - 1] == '/' && req->method == HTTP_GET)
        // {
        //     return send_autoindex_response(client_sock, req, router, buffer,
        //     buffer_size,file_path);
        // }
        send_404(client_sock, req);
        return 0;
    }

    char header[512];
    int  header_len = snprintf(header, sizeof(header),
                               "HTTP/1.1 200 OK\r\n"
                                "Server: %s\r\n"
                                "Date: %s\r\n"
                                "Content-Length: %lld\r\n"
                                "Content-Type: %s\r\n\r\n"
                               // "Connection: close\r\n\r\n"
                               ,
                               SERVER_NAME, get_http_date(), (long long)st.st_size,
                               get_content_type(get_file_extension(temp_file_path)));

    send(client_sock, header, header_len, 0);

    send_file(client_sock, file_path, buffer, buffer_size);

    http_log(&lg, req, 200);

    return 1;
}

int send_status_response(int client_sock, int status_code) {
    const char *reason_phrase = get_reason_phrase(status_code);
    if (!reason_phrase) {
        log_message(&lg, LOG_ERROR, "[send_status_response] Invalid status code: %d", status_code);
        return 0;
    }

    char *date = get_http_date();
    if (!date) {
        log_message(&lg, LOG_ERROR, "[send_status_response] Failed to get current date");
        return 0;
    }

    char response[512];
    int  response_len = snprintf(response, sizeof(response),
                                 "HTTP/1.1 %d %s\r\n"
                                  "Server: %s\r\n"
                                  "Date: %s\r\n"
                                  "Content-Length: %zu\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "\r\n"
                                  "%s",
                                 status_code, reason_phrase, SERVER_NAME, date,
                                 strlen(reason_phrase), reason_phrase);

    free(date);

    if (response_len < 0 || response_len >= (int)sizeof(response)) {
        log_message(&lg, LOG_ERROR,
                    "[send_status_response] Response too large or formatting error");
        return 0;
    }

    ssize_t bytes_sent = write(client_sock, response, response_len);
    if (bytes_sent == -1 || bytes_sent != response_len) {
        log_message(&lg, LOG_ERROR, "[send_status_response] Write error or incomplete write");
        return 0;
    }

    return 1;
}
