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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <regex.h>

#include "http_parser.h"

regex_t uri_regex;

const char *http_methods[HTTP_METHODS_LEN] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE","PATCH"};
const char *http_versions[HTTP_VERSIONS_LEN] = {"HTTP/1.0","HTTP/1.1","HTTP/2"};
void validate_http_method(char *method, http_request *request){
    for (int i = 0; i < HTTP_METHODS_LEN; i++) if (strcmp(method,http_methods[i]) == 0) return; 
    printf("[validate_http_method] error");
    request->is_invalid = 1; 
}

void validate_http_version(char *version, http_request *request){
    for (int i = 0; i < HTTP_VERSIONS_LEN; i++) if (strcmp(version,http_versions[i]) == 0) return;
    printf("[validate_http_version] error");
    request->is_invalid = 1;
    
}

int init_uri_regex() {
    if (regcomp(&uri_regex, URI_PATTERN, REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }
    return 1;
}

void free_uri_regex() {
    regfree(&uri_regex);
}

void validate_uri(char *uri, http_request *request) {
    if (regexec(&uri_regex, uri, 0, NULL, 0) != 0) {
        printf("[validate_uri] error: invalid URI %s\n", uri);
        request->is_invalid = 1;
    }
}
void parse_request_line(char *line, http_request *request) {
    sscanf(line, "%s %s %s", request->method, request->uri, request->version);
    validate_http_method(request->method,request);
    if (request->is_invalid) return;
    validate_http_version(request->version,request); 
    if (request->is_invalid) return;
    validate_uri(request->uri,request);
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
    request->is_invalid = 0;
    request->headers = NULL;
    request->headers_count = 0;

    char *line = strtok(request_text, "\r\n");
    if (line) {
        parse_request_line(line, request);
        if (request->is_invalid) return;   
    }
    else
    {
        request->is_invalid = 1;
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

}