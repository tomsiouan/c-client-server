#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_MESSAGE_SIZE 256

int create_server_socket(int port) {
    struct sockaddr_in server_sockaddr_in = {0};

    server_sockaddr_in.sin_family = AF_INET;
    server_sockaddr_in.sin_addr.s_addr = htonl(INADDR_ANY);

    server_sockaddr_in.sin_port = htons(port);

    int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1) {
        perror("socket");
        return -1;
    }

    // allow the port to be reused immediately after the server stops,
    // instead of waiting for the kernel to release it
    int yes = 1;
    setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (bind(socket_file_descriptor, (struct sockaddr *)&server_sockaddr_in, sizeof(server_sockaddr_in)) == -1) {
        perror("bind");
        close(socket_file_descriptor);
        return -1;
    }

    if (listen(socket_file_descriptor, 5) == -1) {
        perror("listen");
        close(socket_file_descriptor);
        return -1;
    }

    return socket_file_descriptor;
}

int handle_client(int client_fd) {
    char buffer[MAX_MESSAGE_SIZE];

    ssize_t num_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (num_read <= 0) {
        return 0;
    }
    buffer[num_read] = '\0';
    printf("%s", buffer);
    fflush(stdout);

    char status = 0;
    write(client_fd, &status, 1);

    return 1;
}
