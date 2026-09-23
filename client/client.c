#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MAX_MESSAGE_SIZE 256
#define PORT 8081

int main() {
    struct sockaddr_in server_sockaddr_in = {0};

    server_sockaddr_in.sin_family = AF_INET;
    server_sockaddr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_sockaddr_in.sin_port = htons(PORT);

    int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1) {
        perror("socket");
        return 1;
    }

    if (connect(socket_file_descriptor, (struct sockaddr *)&server_sockaddr_in, sizeof(server_sockaddr_in)) == -1) {
        perror("connect");
        close(socket_file_descriptor);
        return 1;
    }

    char buffer[MAX_MESSAGE_SIZE];

    // stay connected: keep sending messages until the user ends input (Ctrl+D)
    while (1) {
        printf("Enter a message (Ctrl+D to quit):\n");

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;   // Ctrl+D or end of input
        }

        // send only the characters typed, not the whole buffer
        size_t len = strlen(buffer);
        if (write(socket_file_descriptor, buffer, len) == -1) {
            perror("write");
            break;
        }

        // wait for the server's reply before prompting again
        ssize_t num_read = read(socket_file_descriptor, buffer, sizeof(buffer) - 1);
        if (num_read <= 0) {
            printf("Server closed the connection\n");
            break;
        }
        buffer[num_read] = '\0';
        printf("Status: %d\n", buffer[0]);
    }

    close(socket_file_descriptor);
    return 0;
}
