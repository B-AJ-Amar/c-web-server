#ifndef PHP_HANDLER_H
#define PHP_HANDLER_H

#include "http_parser.h"
#include <stdio.h>
#include <stdlib.h>

int handle_php_request(int client_sock, http_request *req, char *php_cgi_path, char *buffer,
                       int *readed_len);
int handle_php_request(int client_sock, http_request *req, char *php_cgi_path, char *buffer,
                       int *readed_len);

#endif // PHP_HANDLER_H