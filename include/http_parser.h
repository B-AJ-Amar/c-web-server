#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#define HTTP_METHODS_LEN  9
#define HTTP_VERSIONS_LEN 2
#define URI_PATTERN       "^/[A-Za-z0-9._~!$&'()*+,;=:@/-]*(\\?[A-Za-z0-9._~!$&'()*+,;=:@/?-]*)?$"

#include <netinet/in.h>
#include <regex.h>

extern const char *http_methods[HTTP_METHODS_LEN];
extern const char *http_versions[HTTP_VERSIONS_LEN];

extern regex_t uri_regex;

typedef struct http_headers {
    char key[256];
    char value[256];
} http_headers;

typedef struct http_quary_params {
    char key[256];
    char value[256];
} http_quary_params;

typedef struct http_request {
    char                method[16];
    char                uri[256];
    char                version[8];
    char                endpoint[256];
    http_quary_params  *quary;
    http_headers       *headers;
    int                 headers_count;
    int                 quary_params_count;
    char               *body;
    int                 is_invalid;
    struct sockaddr_in *client_addr;
} http_request;

typedef struct http_response {
    char       *version;
    int         status;
    char       *reason;
    char       *body;
    int         content_len;
    const char *content_type; // html ,css , json ,txt
    char       *date;
    // TODO: extra headers
    // http_headers *headers;
    // int headers_count;
} http_response;

void parse_request_line(char *line, http_request *request);
void parse_header_line(char *line, http_headers *header);
void parse_http_request(char *request_text, http_request *request);

void free_uri_regex();
void validate_uri(char *uri, http_request *request);
int  init_uri_regex();

#endif // HTTP_PARSER_H