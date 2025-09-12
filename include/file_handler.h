#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "config.h"

char *get_file_path(char *uri, route_config router);
char *get_file_extension(char *file_path);
char *read_file(char *file_path, size_t *file_size, int *status);
int   is_php(char *ext);

#endif // FILE_HANDLER_H