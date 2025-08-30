#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "http_parser.h"

#define REQUEST_BODY_SIZE 2048

/*
http requst syntax

<method> <request-target> <HTTP-version>\r\n
Header-Name: value\r\n
Header-Name: value\r\n
\r\n
[Optional body]


e.g get (without body)
GET /index.html HTTP/1.1\r\n
Host: www.example.com\r\n
User-Agent: curl/7.68.0\r\n
Accept: application/json\r\n
\r\n

with body
POST /login HTTP/1.1\r\n
Host: www.example.com\r\n
Content-Type: application/x-www-form-urlencoded\r\n
Content-Length: 27\r\n
\r\n
username=john&password=1234


*/


void parse_request_line(char *line, http_request *request) {
    sscanf(line, "%s %s %s", request->method, request->uri, request->version);
}

void parse_quary_param(char *line, http_quary_params *quary) {
    char *colon = strchr(line, '=');
    if (colon) {
        *colon = '\0';
        strncpy(quary->key, line, sizeof(quary->key));
        strncpy(quary->value, colon + 1, sizeof(quary->value));
    }
}

void parse_request_uri(http_request *request) {
    char *uri = request->uri;
    request->quary = NULL;
    request->quary_params_count = 0;

    char *query_separator = strchr(uri, '?');
    if (query_separator) {
        *query_separator = '\0';
        strncpy(request->endpoint, uri, sizeof(request->endpoint));

        char query_params[256];
        strncpy(query_params, query_separator + 1, sizeof(query_params) - 1);
        query_params[sizeof(query_params) - 1] = '\0';

        char *param = strtok(query_params, "&");
        while (param != NULL) {
            request->quary = realloc(
                request->quary,
                sizeof(http_quary_params) * (request->quary_params_count + 1)
            );
            parse_quary_param(param, &request->quary[request->quary_params_count]);
            request->quary_params_count++;
            param = strtok(NULL, "&"); 
        }
    } else {
        strncpy(request->endpoint, uri, sizeof(request->endpoint));
    }
}

 

void parse_header_line(char *line, http_headers *header) {
    char *colon = strchr(line, ':');
    if (colon) {
        *colon = '\0';
        strncpy(header->key, line, sizeof(header->key));
        strncpy(header->value, colon + 1, sizeof(header->value));
        // fix the spaces issue
        while (*header->value == ' ') memmove(header->value, header->value + 1, strlen(header->value));
    }
}

void parse_http_request(char *request_text, http_request *request) {

    request->headers = NULL;
    request->headers_count = 0;

    char *line = strtok(request_text, "\r\n");
    if (line) parse_request_line(line, request);
    else
    {
        request->is_valid = 0;
        return;
    }

    
    while ((line = strtok(NULL, "\r\n")) && strlen(line) > 0) {
        request->headers = realloc(request->headers, sizeof(http_headers) * (request->headers_count + 1));
        parse_header_line(line, &request->headers[request->headers_count]);
        request->headers_count++;
    }

    char *body_start = strstr(request_text, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // skip \n\r\n\r
        strncpy(request->body, body_start, sizeof(request->body) - 1);
    }
    
    parse_request_uri(request);

    request->is_valid = 1;
}