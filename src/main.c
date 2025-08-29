#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>


#define PORT 8080

#define HTTP_RESPONSE_DEMO "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello client!"


int main(){
    
    struct sockaddr_in serv_addr;
    char buffer[1024];

    printf("Hello, World!\n");
    int client_conn;
    int socket_conn = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_conn < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;  // Any local IP
    serv_addr.sin_port = htons(PORT);       
    

    if (bind(socket_conn, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        close(socket_conn);
        exit(1);
    }

    listen(socket_conn, 5);
    printf("Server listening on port %d...\n", PORT);

    client_conn = accept(socket_conn, NULL, NULL);

    if (client_conn < 0) {
        perror("ERROR on accept");
        close(socket_conn);
        exit(1);
    }
    
    read(client_conn, buffer, sizeof(buffer));
    write(client_conn, HTTP_RESPONSE_DEMO, strlen(HTTP_RESPONSE_DEMO));

    close(client_conn);
    close(socket_conn);
    return 0;
}