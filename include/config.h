#ifndef CONFIG_H
#define CONFIG_H

#include "http_parser.h"
#include "logger.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "proxy.h"
typedef struct {
    char *host;
    int   port;
    int   max_connections;
    int   sock_buffer_size;
    char *default_index_name;
} server_config;

typedef struct logging_config {
    log_level level;
    bool      use_colors;
    char     *time_format;
    FILE     *output; // stdout, stderr, or file
} logging_config;

typedef struct route_config {
    char  *path;       // "/" , "/static" , "/api" ....
    char  *root;       //  "./public"
    char  *index;      // default file (e.g. "index.html")
    char  *proxy_pass; // TODO: backend URL (e.g "http://127.0.0.1:5000")
    bool   autoindex;  // true = list directory, false = no
    char **methods;    // allowed methods { "GET", "POST" }

    proxy_info *proxy;
} route_config;

typedef struct {
    server_config  server;
    logging_config logging;
    route_config  *routes;
} app_config;

extern app_config cfg;

int load_config(const char *filename, app_config *cfg);
int path_router(route_config *routes, http_request *req);
#endif
