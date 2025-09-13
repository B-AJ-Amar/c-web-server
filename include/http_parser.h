#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#define HTTP_METHODS_LEN  9
#define HTTP_VERSIONS_LEN 2
#define URI_PATTERN       "^/[A-Za-z0-9._~!$&'()*+,;=:@/-]*(\\?[A-Za-z0-9._~!$&'()*+,;=:@/?-]*)?$"

#define HTTP_GET     "GET"
#define HTTP_POST    "POST"
#define HTTP_PUT     "PUT"
#define HTTP_DELETE  "DELETE"
#define HTTP_CONNECT "CONNECT"
#define HTTP_OPTIONS "OPTIONS"
#define HTTP_HEAD    "HEAD"
#define HTTP_TRACE   "TRACE"
#define HTTP_PATCH   "PATCH"

#define HTTP_V1_0 "HTTP/1.0"
#define HTTP_V1_1 "HTTP/1.1"

#include <netinet/in.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>

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
    char               *file_path;
    char               *file_ext;
    FILE               *req_data;
    bool                use_file;
    int                 head_line_len;
    char               *str_quary_params;
    int         content_len;
    const char *content_type;
    http_quary_params  *quary;
    http_headers       *headers;
    int                 headers_count;
    int                 quary_params_count;
    char                body[4096]; // Fixed-size buffer for request body
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

char **parse_env_cgi_php(http_request *req, char *buffer, int *readed_len);
void   free_env_cgi_php(char **envp);

void free_http_request(http_request *req);
void free_http_response(http_response *res);

#endif // HTTP_PARSER_H