#include "config.h"
#include "logger.h"
#include "toml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

app_config cfg;

static log_level parse_log_level(const char *s) {
    if (!s)
        return LOG_INFO;
    if (strcasecmp(s, "TRACE") == 0)
        return LOG_TRACE;
    if (strcasecmp(s, "DEBUG") == 0)
        return LOG_DEBUG;
    if (strcasecmp(s, "INFO") == 0)
        return LOG_INFO;
    if (strcasecmp(s, "WARN") == 0)
        return LOG_WARN;
    if (strcasecmp(s, "ERROR") == 0)
        return LOG_ERROR;
    if (strcasecmp(s, "FATAL") == 0)
        return LOG_FATAL;
    return LOG_INFO;
}

static FILE *parse_output(const char *s) {
    if (!s || strcasecmp(s, "stdout") == 0)
        return stdout;
    if (strcasecmp(s, "stderr") == 0)
        return stderr;
    FILE *f = fopen(s, "a");
    return f ? f : stdout;
}

int load_config(const char *filename, app_config *cfg) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return 0;
    }

    char          errbuf[200];
    toml_table_t *conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);
    if (!conf) {
        fprintf(stderr, "TOML parse error: %s\n", errbuf);
        return 0;
    }

    cfg->server.host               = strdup("0.0.0.0");
    cfg->server.port               = 8080;
    cfg->server.max_connections    = 100;
    cfg->server.sock_buffer_size   = 4096;
    cfg->server.default_index_name = strdup("index.html");

    cfg->logging.level       = LOG_INFO;
    cfg->logging.use_colors  = true;
    cfg->logging.time_format = strdup("%Y-%m-%d %H:%M:%S");
    cfg->logging.output      = stdout;

    cfg->routes = NULL;

    // Parse [server]
    toml_table_t *server = toml_table_in(conf, "server");
    if (server) {
        toml_datum_t s;
        if ((s = toml_string_in(server, "host")).ok) {
            free(cfg->server.host);
            cfg->server.host = s.u.s;
        }
        if ((s = toml_int_in(server, "port")).ok)
            cfg->server.port = (int)s.u.i;
        if ((s = toml_int_in(server, "max_connections")).ok)
            cfg->server.max_connections = (int)s.u.i;
        if ((s = toml_int_in(server, "sock_buffer_size")).ok)
            cfg->server.sock_buffer_size = (int)s.u.i;
        if ((s = toml_string_in(server, "default_index_name")).ok) {
            free(cfg->server.default_index_name);
            cfg->server.default_index_name = s.u.s;
        }
    }

    // Parse [logging]
    toml_table_t *logging = toml_table_in(conf, "logging");
    if (logging) {
        toml_datum_t s;
        if ((s = toml_string_in(logging, "level")).ok) {
            cfg->logging.level = parse_log_level(s.u.s);
            free(s.u.s);
        }
        if ((s = toml_bool_in(logging, "use_colors")).ok)
            cfg->logging.use_colors = s.u.b;
        if ((s = toml_string_in(logging, "time_format")).ok) {
            free(cfg->logging.time_format);
            cfg->logging.time_format = s.u.s;
        }
        if ((s = toml_string_in(logging, "output")).ok) {
            cfg->logging.output = parse_output(s.u.s);
            free(s.u.s);
        }
    }

    // Parse [[routes]]
    int n_routes = toml_array_nelem(toml_array_in(conf, "routes"));
    if (n_routes > 0) {
        cfg->routes = calloc(n_routes, sizeof(route_config));
        for (int i = 0; i < n_routes; i++) {
            toml_table_t *r = toml_table_at(toml_array_in(conf, "routes"), i);
            if (!r)
                continue;

            route_config *rc = &cfg->routes[i];

            rc->path       = NULL;
            rc->root       = NULL;
            rc->index      = NULL;
            rc->proxy_pass = NULL;
            rc->autoindex  = false;
            rc->methods    = NULL; // null means all

            toml_datum_t s;

            if ((s = toml_string_in(r, "path")).ok) {
                rc->path = s.u.s;
            } else {
                log_message(&lg, LOG_FATAL,
                            "Config error: route at index %d is missing required 'path'", i);
                return 0;
            }

            if ((s = toml_string_in(r, "proxy_pass")).ok) {
                rc->proxy_pass = s.u.s;
            } else {
                if ((s = toml_string_in(r, "root")).ok) {
                    rc->root = s.u.s;
                } else {
                    log_message(&lg, LOG_FATAL,
                                "Config error: route '%s' must have either 'proxy_pass' or 'root'",
                                rc->path);
                    return 0;
                }

                if ((s = toml_string_in(r, "index")).ok) {
                    rc->index = s.u.s;
                } else {
                    rc->index = strdup(cfg->server.default_index_name);
                }

                if ((s = toml_bool_in(r, "autoindex")).ok) {
                    rc->autoindex = s.u.b;
                }
            }

            // Methods (optional, but must have at least 1 if specified)
            toml_array_t *methods = toml_array_in(r, "methods");
            if (methods) {
                int n_methods = toml_array_nelem(methods);
                if (n_methods == 0) {
                    log_message(&lg, LOG_FATAL,
                                "Config error: route '%s' defines empty 'methods' array", rc->path);
                    return 0;
                }
                rc->methods = calloc(n_methods + 1, sizeof(char *));
                for (int j = 0; j < n_methods; j++) {
                    toml_datum_t m = toml_string_at(methods, j);
                    if (m.ok) {
                        rc->methods[j] = m.u.s;
                    } else {
                        log_message(&lg, LOG_FATAL,
                                    "Config error: route '%s' has invalid method entry", rc->path);
                        return 0;
                    }
                }
            }
        }
    }

    toml_free(conf);
    return 1;
}
