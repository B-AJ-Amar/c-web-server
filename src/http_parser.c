#include "http_parser.h"
#include "logger.h"
#include <ctype.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

regex_t uri_regex;

const char *http_methods[HTTP_METHODS_LEN]   = {HTTP_GET,     HTTP_HEAD,   HTTP_POST,
                                                HTTP_PUT,     HTTP_DELETE, HTTP_CONNECT,
                                                HTTP_OPTIONS, HTTP_TRACE,  HTTP_PATCH};
const char *http_versions[HTTP_VERSIONS_LEN] = {HTTP_V1_0, HTTP_V1_1};

void validate_http_method(char *method, http_request *request) {
    for (int i = 0; i < HTTP_METHODS_LEN; i++)
        if (strcmp(method, http_methods[i]) == 0)
            return;
    printf("[validate_http_method] error");
    request->is_invalid = 1;
}

void validate_http_version(char *version, http_request *request) {
    for (int i = 0; i < HTTP_VERSIONS_LEN; i++)
        if (strcmp(version, http_versions[i]) == 0)
            return;
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

void free_uri_regex() { regfree(&uri_regex); }

void validate_uri(char *uri, http_request *request) {
    if (regexec(&uri_regex, uri, 0, NULL, 0) != 0) {
        printf("[validate_uri] error: invalid URI %s\n", uri);
        request->is_invalid = 1;
    }
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
    char *uri                   = request->uri;
    request->quary              = NULL;
    request->quary_params_count = 0;

    char *query_separator = strchr(uri, '?');
    if (query_separator) {
        *query_separator = '\0';
        strncpy(request->endpoint, uri, sizeof(request->endpoint));

        char query_params[256];
        strncpy(query_params, query_separator + 1, sizeof(query_params) - 1);
        query_params[sizeof(query_params) - 1] = '\0';

        request->str_quary_params = strdup(query_params);
        char *param               = strtok(query_params, "&");
        while (param != NULL) {
            http_quary_params *new_quary = realloc(
                request->quary, sizeof(http_quary_params) * (request->quary_params_count + 1));
            if (!new_quary) {
                free(request->quary);
                request->quary              = NULL;
                request->quary_params_count = 0;
                break;
            }
            request->quary = new_quary;
            parse_quary_param(param, &request->quary[request->quary_params_count]);
            request->quary_params_count++;
            param = strtok(NULL, "&");
        }
    } else {
        strncpy(request->endpoint, uri, sizeof(request->endpoint));
    }
}

void parse_request_line(char *line, http_request *request) {
    request->head_line_len = strlen(line);
    sscanf(line, "%s %s %s", request->method, request->uri, request->version);
    validate_http_method(request->method, request);
    if (request->is_invalid)
        return;
    validate_http_version(request->version, request);
    if (request->is_invalid)
        return;
    validate_uri(request->uri, request);

    parse_request_uri(request);
}

void parse_header_line(char *line, http_headers *header) {
    char *colon = strchr(line, ':');
    if (colon) {
        *colon = '\0';
        strncpy(header->key, line, sizeof(header->key));
        strncpy(header->value, colon + 1, sizeof(header->value));
        // fix the spaces issue
        while (*header->value == ' ')
            memmove(header->value, header->value + 1, strlen(header->value));
    }
}

void parse_header_line_php_cgi(char *line, http_headers *header) {
    char *colon = strchr(line, ':');
    if (colon) {
        *colon         = '\0';
        size_t key_len = strlen(line);
        for (size_t i = 0; i < key_len && i < sizeof(header->key) - 1; i++) {
            if (line[i] == '-')
                header->key[i] = '_';
            else
                header->key[i] = toupper((unsigned char)line[i]);
        }
        header->key[key_len < sizeof(header->key) - 1 ? key_len : sizeof(header->key) - 1] = '\0';

        strncpy(header->value, colon + 1, sizeof(header->value));
        // fix the spaces issue
        while (*header->value == ' ')
            memmove(header->value, header->value + 1, strlen(header->value));
    }
}

void parse_headers_lines(char *request_text, http_request *request, bool skip_head_line,
                         bool use_php_cgi) {
    char *text_copy = NULL;
    char *line      = NULL;

    if (request->use_file && request->req_data) {
        fseek(request->req_data, 0, SEEK_SET);

        char line_buffer[2048];

        if (skip_head_line) {
            if (!fgets(line_buffer, sizeof(line_buffer), request->req_data))
                return;
        }

        while (fgets(line_buffer, sizeof(line_buffer), request->req_data)) {
            size_t len = strlen(line_buffer);
            while (len > 0 && (line_buffer[len - 1] == '\n' || line_buffer[len - 1] == '\r'))
                line_buffer[--len] = '\0';

            if (len == 0)
                break; // end of headers

            http_headers *new_headers =
                realloc(request->headers, sizeof(http_headers) * (request->headers_count + 1));
            if (!new_headers) {
                free(request->headers);
                request->headers       = NULL;
                request->headers_count = 0;
                break;
            }
            request->headers = new_headers;

            if (use_php_cgi) {
                parse_header_line_php_cgi(line_buffer, &request->headers[request->headers_count]);
            } else {
                parse_header_line(line_buffer, &request->headers[request->headers_count]);
            }
            request->headers_count++;
        }
    } else {
        text_copy = strdup(request_text);
        line      = strtok(text_copy, "\r\n");

        if (skip_head_line && line) {
            line = strtok(NULL, "\r\n");
        }

        if (use_php_cgi) {
            while (line && strlen(line) > 0) {
                http_headers *new_headers =
                    realloc(request->headers, sizeof(http_headers) * (request->headers_count + 1));
                if (!new_headers) {
                    free(request->headers);
                    request->headers       = NULL;
                    request->headers_count = 0;
                    break;
                }
                request->headers = new_headers;
                parse_header_line_php_cgi(line, &request->headers[request->headers_count]);
                request->headers_count++;
                line = strtok(NULL, "\r\n");
            }
        } else {
            while (line && strlen(line) > 0) {
                http_headers *new_headers =
                    realloc(request->headers, sizeof(http_headers) * (request->headers_count + 1));
                if (!new_headers) {
                    free(request->headers);
                    request->headers       = NULL;
                    request->headers_count = 0;
                    break;
                }
                request->headers = new_headers;
                parse_header_line(line, &request->headers[request->headers_count]);
                request->headers_count++;
                line = strtok(NULL, "\r\n");
            }
        }

        if (text_copy) {
            free(text_copy);
        }
    }
}

char **parse_env_cgi_php(http_request *req, char *buffer, int *readed_len) {

    int max_env_vars;

    int env_index = 5;
    parse_headers_lines(buffer, req, 1, 1);
    max_env_vars = 6 + (req->headers_count ? req->headers_count : 0) + 1;
    char **envp  = malloc(max_env_vars * sizeof(char *));

    if (!strcmp(req->method, HTTP_POST)) {
        for (int i = 0; i < req->headers_count; i++) {
            envp[++env_index] = malloc(512);
            if (strcasecmp(req->headers[i].key, "Content-Length") == 0) {

                sprintf(envp[env_index], "CONTENT_LENGTH=%s", req->headers[i].value);
                req->content_len = atoi(req->headers[i].value);
            } else if (strcasecmp(req->headers[i].key, "Content-Type") == 0)
                sprintf(envp[env_index], "CONTENT_TYPE=%s", req->headers[i].value);
            else
                sprintf(envp[env_index], "HTTP_%s=%s", req->headers[i].key, req->headers[i].value);
        }

    } else {

        for (int i = 0; i < req->headers_count; i++) {
            envp[++env_index] = malloc(512);
            sprintf(envp[env_index], "HTTP_%s=%s", req->headers[i].key, req->headers[i].value);
        }
    }

    envp[0] = strdup("GATEWAY_INTERFACE=CGI/1.1");

    envp[1] = malloc(256);
    sprintf(envp[1], "REQUEST_METHOD=%s", req->method);

    envp[2] = malloc(512);
    sprintf(envp[2], "SCRIPT_FILENAME=%s", req->file_path);

    envp[3] = malloc(512);
    sprintf(envp[3], "QUERY_STRING=%s", req->str_quary_params ? req->str_quary_params : "");

    envp[4] = malloc(256);
    sprintf(envp[4], "SERVER_PROTOCOL=%s", req->version);

    envp[5] = strdup("REDIRECT_STATUS=200");

    envp[env_index] = NULL;

    // todo add the other vars and body

    return envp;
}

void free_env_cgi_php(char **envp) {
    if (!envp)
        return;

    for (int i = 0; envp[i] != NULL; i++) {
        free(envp[i]);
    }
    free(envp);
}

void free_http_request(http_request *req) {
    if (req->file_path) {
        free(req->file_path);
        req->file_path = NULL;
    }

    req->file_ext = NULL;

    if (req->str_quary_params) {
        free(req->str_quary_params);
        req->str_quary_params = NULL;
    }
    if (req->quary) {
        free(req->quary);
        req->quary = NULL;
    }
    if (req->headers) {
        free(req->headers);
        req->headers = NULL;
    }
    if (req->req_data) {
        fclose(req->req_data);
        req->req_data = NULL;
    }
}