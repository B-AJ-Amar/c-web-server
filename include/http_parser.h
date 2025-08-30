#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#define REQUEST_BODY_SIZE 2048

typedef struct http_headers {
    char key[256];
    char value[256];
} http_headers;

typedef struct http_quary_params {
    char key[256];
    char value[256];
} http_quary_params;

typedef struct http_request {
    char method[16];
    char uri[256];
    char version[8];
    char endpoint[256];
    http_quary_params *quary;
    http_headers *headers;
    int headers_count;
    int quary_params_count;
    char body[REQUEST_BODY_SIZE];
    int is_valid;
} http_request;

void parse_request_line(char *line, http_request *request);
void parse_header_line(char *line, http_headers *header);
void parse_http_request(char *request_text, http_request *request);

#endif // HTTP_PARSER_H