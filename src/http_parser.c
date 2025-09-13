#include "http_parser.h"
#include "logger.h"
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
                // If realloc fails, free existing memory and break
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

void parse_http_request(char *request_text, http_request *request) {
    request->is_invalid    = 0;
    request->headers       = NULL;
    request->headers_count = 0;

    char *line = strtok(request_text, "\r\n");
    if (line) {
        parse_request_line(line, request);
        if (request->is_invalid)
            return;
    } else {
        request->is_invalid = 1;
        return;
    }

    while ((line = strtok(NULL, "\r\n")) && strlen(line) > 0) {
        request->headers =
            realloc(request->headers, sizeof(http_headers) * (request->headers_count + 1));
        parse_header_line(line, &request->headers[request->headers_count]);
        request->headers_count++;
    }

    char *body_start = strstr(request_text, "\r\n\r\n");
    if (body_start) {
        body_start += 4; // skip \n\r\n\r
        strncpy(request->body, body_start, sizeof(request->body) - 1);
    }
}

char **parse_env_cgi_php(http_request *req, char *buffer, FILE *file_buffer, int *readed_len) {

    char **envp = malloc(7 * sizeof(char *));

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
    envp[6] = NULL;

    log_message(&lg, LOG_DEBUG, "CGI SCRIPT_FILENAME: %s", envp[2]);
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

    // file_ext points within file_path, so just set to NULL
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