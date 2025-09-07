#include "config.h"
#include "logger.h"
#include "toml.h"
#include <arpa/inet.h>
#include <regex.h>
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

static int validate_ip(const char *ip) {
    struct in_addr addr;
    return inet_pton(AF_INET, ip, &addr) == 1;
}

static int validate_proxy_pass(const char *url) {
    if (!url)
        return 0;

    regex_t     regex;
    const char *pattern = "^(http|https)://([a-zA-Z0-9._-]+|\\[[0-9a-fA-F:]+\\])(:[0-9]{1,5})?$";

    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB) != 0)
        return 0;

    int result = regexec(&regex, url, 0, NULL, 0);
    regfree(&regex);

    if (result == 0)
        return 1;
    return 0;
}

static void sort_paths_by_longer(route_config *routes) {
    for (int i = 0; routes[i].path != NULL; i++) {
        for (int j = i + 1; routes[j].path != NULL; j++) {
            if (strlen(routes[i].path) < strlen(routes[j].path)) {
                route_config temp = routes[i];
                routes[i]         = routes[j];
                routes[j]         = temp;
            }
        }
    }
}

static int validate_route_config(route_config *rc, server_config *srv) {
    if (!rc->path || rc->path[0] != '/') {
        // log_message(&lg, LOG_FATAL, "Invalid route path: must start with '/'");
        return 0;
    }

    if (rc->proxy_pass) {
        if (!validate_proxy_pass(rc->proxy_pass)) {
            log_message(&lg, LOG_FATAL, "Invalid proxy_pass URL in route '%s': %s", rc->path,
                        rc->proxy_pass);
            return 0;
        }
        if (rc->root || rc->index) {
            log_message(&lg, LOG_WARN,
                        "Route '%s' has proxy_pass defined, ignoring root/index/autoindex",
                        rc->path);
        }
        return 1;
    } else {
        if (!rc->root) {
            log_message(&lg, LOG_FATAL, "Route '%s' missing 'root' (or proxy_pass)", rc->path);
            return 0;
        }
        if (!rc->index) {
            rc->index = strdup(srv->default_index_name);
        }
    }

    if (rc->methods) {
        for (int i = 0; rc->methods[i]; i++) {
            const char *m = rc->methods[i];
            if (!(strcmp(m, HTTP_GET) == 0 || strcmp(m, HTTP_POST) == 0 ||
                  strcmp(m, HTTP_HEAD) == 0 || strcmp(m, HTTP_PUT) == 0 ||
                  strcmp(m, HTTP_DELETE) == 0)) {
                log_message(&lg, LOG_FATAL, "Invalid HTTP method '%s' in route '%s'", m, rc->path);
                return 0;
            }
        }
    }

    return 1;
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

    // [server]
    toml_table_t *server = toml_table_in(conf, "server");
    if (server) {
        toml_datum_t s;
        if ((s = toml_string_in(server, "host")).ok) {
            free(cfg->server.host);
            if (!validate_ip(s.u.s)) {
                log_message(&lg, LOG_FATAL, "Invalid server.host: %s", s.u.s);
                return 0;
            }
            cfg->server.host = s.u.s;
        }
        if ((s = toml_int_in(server, "port")).ok) {
            if (s.u.i <= 0 || s.u.i > 65535) {
                log_message(&lg, LOG_FATAL, "Invalid server.port: %lld", s.u.i);
                return 0;
            }
            cfg->server.port = (int)s.u.i;
        }
        if ((s = toml_int_in(server, "max_connections")).ok) {
            if (s.u.i <= 0) {
                log_message(&lg, LOG_FATAL, "Invalid server.max_connections: %lld", s.u.i);
                return 0;
            }
            cfg->server.max_connections = (int)s.u.i;
        }
        if ((s = toml_int_in(server, "sock_buffer_size")).ok) {
            if (s.u.i <= 0) {
                log_message(&lg, LOG_FATAL, "Invalid server.sock_buffer_size: %lld", s.u.i);
                return 0;
            }
            cfg->server.sock_buffer_size = (int)s.u.i;
        }
        if ((s = toml_string_in(server, "default_index_name")).ok) {
            free(cfg->server.default_index_name);
            cfg->server.default_index_name = s.u.s;
        }
    }

    // [logging]
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

    // [[routes]]
    int n_routes = toml_array_nelem(toml_array_in(conf, "routes"));
    if (n_routes > 0) {
        cfg->routes = calloc(n_routes + 1, sizeof(route_config)); // +1 for NULL terminator
        for (int i = 0; i < n_routes; i++) {
            toml_table_t *r = toml_table_at(toml_array_in(conf, "routes"), i);
            if (!r)
                continue;

            route_config *rc = &cfg->routes[i];
            rc->path         = NULL;
            rc->root         = NULL;
            rc->index        = NULL;
            rc->proxy_pass   = NULL;
            rc->autoindex    = false;
            rc->methods      = NULL;

            toml_datum_t s;

            if ((s = toml_string_in(r, "path")).ok) {
                rc->path = s.u.s;
            } else {
                log_message(&lg, LOG_FATAL, "Config error: route at index %d missing 'path'", i);
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

            if (!validate_route_config(rc, &cfg->server)) {
                return 0;
            }
        }

        sort_paths_by_longer(cfg->routes);
    }

    toml_free(conf);
    return 1;
}
