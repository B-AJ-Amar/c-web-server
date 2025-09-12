

#include "http_handler.h"
#include "http_parser.h"
#include "http_response.h"
#include "logger.h"
#include "middlewares.h"
#include "proxy.h"
#include "sock.h"
#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

int handle_php_request(int client_sock, http_request *req, char *php_cgi_path, char *buffer,
                       int *readed_len) {
    char **envp = parse_env_cgi_php(req, buffer, req->req_data, readed_len);
    if (!envp) {
        log_message(&lg, LOG_ERROR, "Failed to prepare CGI environment");
        send_500(client_sock, req);
        return 0;
    }

    int fd[2];
    if (pipe(fd)) {
        log_message(&lg, LOG_ERROR, "Failed to create pipe");
        send_500(client_sock, req);
        return 0;
    }
    pid_t pid = fork();

    if (pid == 0) {
        close(fd[0]);
        char *argv[] = {"php-cgi", NULL};

        // tofix : just for test
        char *envp[] = {"GATEWAY_INTERFACE=CGI/1.1",
                        "REQUEST_METHOD=GET",
                        "SCRIPT_FILENAME=/home/amar/Desktop/web-server/static/test.php",
                        "SCRIPT_NAME=/test.php",
                        "QUERY_STRING=name=amar",
                        "SERVER_PROTOCOL=HTTP/1.1",
                        "DOCUMENT_ROOT=home/amar/Desktop/web-server/static/",
                        "REDIRECT_STATUS=200",
                        NULL};

        dup2(fd[1], STDOUT_FILENO);

        execve("/usr/bin/php-cgi", argv, envp);

        perror("execve failed");
        _exit(1);
    } else if (pid > 0) {
        // waitpid(pid, NULL, 0);
        close(fd[1]);
        int n = read(fd[0], buffer, BUFFER_SIZE - 1);
        if (n > 0) {
            buffer[n] = '\0';
            // get status from the first line
            // char *status_line = strtok(buffer, "\r\n");
            send(client_sock, "HTTP/1.1 200 OK\r\n", 17, 0);
            send(client_sock, buffer, n, 0);
            close(fd[0]);
            return 1;
        } else {
            log_message(&lg, LOG_ERROR, "Failed to read CGI output");
            send_500(client_sock, req);
            close(fd[0]);
            return 0;
        }

    } else {
        perror("fork failed");
    }
}