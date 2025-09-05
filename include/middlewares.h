#ifndef MIDDLEWARES_H
#define MIDDLEWARES_H

#include "config.h"

int is_allowed_method(char **methods, const char *method);
int path_router(route_config *routes, char *endpoint);

#endif // MIDDLEWARES_H
