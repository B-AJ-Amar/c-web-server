#ifndef CONFIG_H
#define CONFIG_H

#include "http_parser.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char *host;
    int   port;
    int   max_connections;
    int   sock_buffer_size;
    char *default_index_name;
} server_config;

typedef struct {
    log_level level;
    bool      use_colors;
    char     *time_format;
    FILE     *output; // stdout, stderr, or file
} logging_config;

typedef struct {
    char  *path;       // "/" , "/static" , "/api" ....
    char  *root;       //  "./public"
    char  *index;      // default file (e.g. "index.html")
    char  *proxy_pass; // TODO: backend URL (e.g "http://127.0.0.1:5000")
    bool   autoindex;  // true = list directory, false = no
    char **methods;    // allowed methods { "GET", "POST" }
} route_config;

typedef struct {
    server_config  server;
    logging_config logging;
    route_config  *routes;
} app_config;

int load_config(const char *filename, app_config *cfg);

#endif
