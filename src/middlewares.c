
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_allowed_method(char **methods, const char *method) {
    if (methods == NULL)
        return 1;

    for (int i = 0; methods[i] != NULL; i++) {
        if (strcmp(methods[i], method) == 0)
            return 1;
    }
    return 0;
}

int path_router(route_config *routes, char *endpoint) {
    if (!routes)
        return -1;

    for (int i = 0; routes[i].path != NULL; i++) {
        size_t route_path_len = strlen(routes[i].path);

        if (strncmp(endpoint, routes[i].path, route_path_len) == 0)
            return i;
    }

    return -1;
}

// add is version suported