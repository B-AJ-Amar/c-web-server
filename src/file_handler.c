

#include "file_handler.h"
#include "config.h"
#include "http_status.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_file_path(char *uri, route_config router) {

    char       *final_uri  = NULL;
    const char *index_file = router.index;
    const char *files_root = router.root;
    const char *route_path = router.path;

    const char *relative_path  = uri;
    size_t      route_path_len = strlen(route_path);

    if (strncmp(uri, route_path, route_path_len) == 0) {
        relative_path = uri + route_path_len;
    }

    if (strlen(relative_path) == 0 || relative_path[strlen(relative_path) - 1] == '/') {
        size_t new_uri_len = strlen(relative_path) + strlen(index_file);
        final_uri          = malloc(new_uri_len + 1);
        if (!final_uri)
            return NULL;
        sprintf(final_uri, "%s%s", relative_path, index_file);
    } else {
        final_uri = strdup(relative_path);
        if (!final_uri)
            return NULL;
    }

    size_t path_len  = strlen(files_root) + strlen(final_uri);
    char  *file_path = malloc(path_len + 1);
    if (!file_path) {
        free(final_uri);
        return NULL;
    }
    sprintf(file_path, "%s/%s", files_root, final_uri);
    free(final_uri);

    return file_path;
}

char *get_file_extension(char *file_path) {
    if (!file_path) return NULL;
    char *end = file_path + strlen(file_path) - 1;
    while (end >= file_path) {
        if (*end == '.') return end + 1;
        
        if (*end == '/') return NULL;
        end--;
    }
    return NULL;
}

char *read_file(char *file_path, size_t *file_size, int *status) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        *status = HTTP_NOT_FOUND;
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(*file_size + 1);
    if (!buffer) {
        fclose(file);
        *status = HTTP_SERVICE_UNAVAILABLE;
        return NULL;
    }

    fread(buffer, 1, *file_size, file);
    buffer[*file_size] = '\0';

    fclose(file);
    *status = HTTP_OK;
    return buffer;
}

int is_php(char* ext){
    if (!ext) return 0;
    if (strcmp(ext,"php") == 0) return 1;
    return 0;
}