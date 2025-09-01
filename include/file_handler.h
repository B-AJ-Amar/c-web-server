#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#define DEFAULT_FILE_NAME  "index.html"
#define DEFAULT_FILES_ROOT "./public"

char *get_file_path(const char *uri);
char *get_file_extension(char *file_path);
char *read_file(char *file_path, size_t *file_size, int *status);

#endif // FILE_HANDLER_H