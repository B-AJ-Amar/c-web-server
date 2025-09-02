#ifndef LOGGER_H
#define LOGGER_H

#include "http_parser.h"

#define COLOR_RESET  "\033[0m"
#define COLOR_GREEN  "\033[32m" // success/info
#define COLOR_YELLOW "\033[33m" // warning
#define COLOR_RED    "\033[31m" // error/fatal
#define COLOR_CYAN   "\033[36m" // trace

#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"

#define DEFAULT_TIME_FORMAT "%Y-%m-%d %H:%M:%S"

static const char *level_names[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

static const char *level_colors[] = {COLOR_CYAN,   COLOR_MAGENTA, COLOR_GREEN,
                                     COLOR_YELLOW, COLOR_RED,     COLOR_RED};

typedef enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL } log_level;

typedef struct {
    log_level level;
    int       use_colors;
    char     *time_format;
    FILE     *output; // (stdout, stderr, file)
} logger;

extern logger lg;

int  init_logger(logger *lg, log_level level, int use_colors, char *time_format, FILE *output);
void log_message(logger *lg, log_level level, const char *fmt, ...);
void http_log(logger *lg, http_request *req, http_response *res);

#endif // LOGGER_H