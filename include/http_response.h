#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#define SERVER_NAME "MyWebServer"

typedef struct {
    const char *html;
    const char *css;
    const char *js;
    const char *json;
    const char *png;
    const char *jpg;
    const char *gif;
    const char *svg;
    const char *txt;
} ContentTypes;

static const ContentTypes CONTENT_TYPES = {
    .html = "text/html",
    .css  = "text/css",
    .js   = "application/javascript",
    .json = "application/json",
    .png  = "image/png",
    .jpg  = "image/jpeg",
    .gif  = "image/gif",
    .svg  = "image/svg+xml",
    .txt  = "text/plain"
};
#endif // HTTP_RESPONSE_H