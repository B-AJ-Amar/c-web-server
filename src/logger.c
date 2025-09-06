#include <arpa/inet.h>
#include <netinet/in.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http_parser.h"
#include "logger.h"
#include <stdarg.h>
#include <time.h>

const char *level_names[]  = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
const char *level_colors[] = {COLOR_CYAN,   COLOR_MAGENTA, COLOR_GREEN,
                              COLOR_YELLOW, COLOR_RED,     COLOR_RED};

logger lg;

int init_logger(logger *lg, log_level level, int use_colors, char *time_format, FILE *output) {
    if (!lg) {
        fprintf(stderr, "Logger pointer is NULL\n");
        return 0;
    }

    lg->level       = level;
    lg->use_colors  = use_colors;
    lg->output      = output ? output : stdout;
    lg->time_format = time_format ? time_format : DEFAULT_TIME_FORMAT;

    return 1;
}

void log_message(logger *lg, log_level level, const char *fmt, ...) {
    if (!lg || level < lg->level)
        return;

    time_t     now = time(NULL);
    struct tm *t   = localtime(&now);
    char       timebuf[20];
    strftime(timebuf, sizeof(timebuf), lg->time_format, t);

    const char *color = lg->use_colors ? level_colors[level] : "";
    const char *reset = lg->use_colors ? COLOR_RESET : "";

    fprintf(lg->output, "%s [%s%s%s] ", timebuf, color, level_names[level], reset);

    va_list args;
    va_start(args, fmt);
    vfprintf(lg->output, fmt, args);
    va_end(args);

    fprintf(lg->output, "\n");
    fflush(lg->output);
}

void http_log(logger *lg, http_request *req, int status) {
    char ip[INET_ADDRSTRLEN];
    int  port = 0;

    if (req->client_addr) {
        inet_ntop(AF_INET, &(req->client_addr->sin_addr), ip, sizeof(ip));
        port = ntohs(req->client_addr->sin_port);
    } else {
        snprintf(ip, sizeof(ip), "-");
    }

    char       timebuf[64];
    time_t     now = time(NULL);
    struct tm *t   = localtime(&now);
    strftime(timebuf, sizeof(timebuf), lg->time_format, t);

    const char *method_color = "";
    const char *status_color = "";
    const char *reset        = lg->use_colors ? COLOR_RESET : "";

    if (lg->use_colors) {
        if (strcmp(req->method, HTTP_GET) == 0)
            method_color = COLOR_GREEN;
        else if (strcmp(req->method, HTTP_CONNECT) == 0)
            method_color = COLOR_GREEN;
        else if (strcmp(req->method, HTTP_POST) == 0)
            method_color = COLOR_CYAN;
        else if (strcmp(req->method, HTTP_DELETE) == 0)
            method_color = COLOR_RED;
        else
            method_color = COLOR_YELLOW;

        if (status >= 200 && status < 300)
            status_color = COLOR_GREEN;
        else if (status >= 300 && status < 400)
            status_color = COLOR_CYAN;
        else if (status >= 400 && status < 500)
            status_color = COLOR_YELLOW;
        else if (status >= 500)
            status_color = COLOR_RED;
    }

    fprintf(lg->output, "[%s] %s:%d \"%s%s%s %s %s\" %s%d%s\n", timebuf, ip, port, method_color,
            req->method, reset, req->uri, req->version, status_color, status, reset);
}
