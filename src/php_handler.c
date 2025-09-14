

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

    int fd[2];
    if (pipe(fd)) {
        log_message(&lg, LOG_ERROR, "Failed to prepare CGI environment");
        send_500(client_sock, req);
        return 0;
    }
    pid_t pid = fork();

    if (pid == 0) {
        close(fd[0]);
        char *argv[] = {"php-cgi", NULL};

        char **envp = parse_env_cgi_php(req, buffer, readed_len);
        if (!envp) {
            log_message(&lg, LOG_ERROR, "Failed to prepare CGI environment");
            send_500(client_sock, req);
            free_env_cgi_php(envp);
            return 0;
        }

        dup2(fd[1], STDOUT_FILENO);

        execve(php_cgi_path, argv, envp);

        perror("execve failed");
        _exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
        close(fd[1]);

        send(client_sock, "HTTP/1.1 200 OK\r\n", 17, 0);

        int total_sent = 0;
        int n;

        while ((n = read(fd[0], buffer, BUFFER_SIZE - 1)) > 0) {
            size_t sent = 0;
            while (sent < n) {
                ssize_t s = send(client_sock, buffer + sent, n - sent, 0);
                if (s <= 0) {
                    log_message(&lg, LOG_ERROR, "Failed to send CGI output to client");
                    close(fd[0]);
                    return 0;
                }
                sent += s;
            }
            total_sent += n;
        }

        close(fd[0]);

        if (total_sent > 0) {
            return 1;
        } else {
            log_message(&lg, LOG_ERROR, "No CGI output received");
            send_500(client_sock, req);
            return 0;
        }

    } else {
        perror("fork failed");
    }
}