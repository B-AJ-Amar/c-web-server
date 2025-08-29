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
    int client_sock;
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;  // Any local IP
    serv_addr.sin_port = htons(PORT);       
    

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        close(serv_sock);
        exit(1);
    }

    listen(serv_sock, 5);
    printf("Server listening on port %d...\n", PORT);

    client_sock = accept(serv_sock, NULL, NULL);

    if (client_sock < 0) {
        perror("ERROR on accept");
        close(serv_sock);
        exit(1);
    }
    
    read(client_sock, buffer, sizeof(buffer));
    write(client_sock, HTTP_RESPONSE_DEMO, strlen(HTTP_RESPONSE_DEMO));

    close(client_sock);
    close(serv_sock);
    return 0;
}