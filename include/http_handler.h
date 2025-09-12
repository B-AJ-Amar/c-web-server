#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#define BUFFER_SIZE 8192

void handle_http_request(int client_sock);
void handle_http_req_task(void *args);

#endif // HTTP_HANDLER_H