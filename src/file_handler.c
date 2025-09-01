

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "file_handler.h"
#include "http_status.h"

char *get_file_path(const char *uri) {
    char  *final_uri = NULL;
    size_t uri_len   = strlen(uri);

    if (uri_len > 0 && uri[uri_len - 1] == '/') {
        size_t new_uri_len = uri_len + strlen("index.html");
        final_uri          = malloc(new_uri_len + 1);
        if (!final_uri)
            return NULL;
        sprintf(final_uri, "%sindex.html", uri);
    } else {
        final_uri = strdup(uri);
        if (!final_uri)
            return NULL;
    }

    size_t path_len  = strlen(DEFAULT_FILES_ROOT) + strlen(final_uri);
    char  *file_path = malloc(path_len + 1);
    if (!file_path) {
        free(final_uri);
        return NULL;
    }
    sprintf(file_path, "%s%s", DEFAULT_FILES_ROOT, final_uri);
    free(final_uri);
    return file_path;
}

char *get_file_extension(char *file_path) {
    strtok(file_path, ".");
    char *ext = strtok(NULL, ".");
    return ext;
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