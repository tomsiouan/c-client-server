#include <stdio.h>
#include "client.h"
#include "common.h"

#ifdef _WIN32
#define QUIT_KEYS "Ctrl+Z then Enter"
#else
#define QUIT_KEYS "Ctrl+D"
#endif

#define IP_SIZE 16   // enough for "255.255.255.255" and the '\0'

int main() {
    if (net_init() == -1) {
        return 1;
    }

    char server_ip[IP_SIZE];

    printf("Looking for the server on the local network...\n");
    if (discover_server(server_ip, sizeof(server_ip)) == -1) {
        fprintf(stderr, "No server found on the local network\n");
        net_cleanup();
        return 1;
    }
    printf("Server found at %s\n", server_ip);

    socket_t server_fd = connect_to_server(server_ip, PORT);
    if (server_fd == INVALID_SOCKET) {
        net_cleanup();
        return 1;
    }

    char buffer[MAX_MESSAGE_SIZE];

    // stay connected: keep sending messages until the user ends input
    while (1) {
        printf("Enter a message (" QUIT_KEYS " to quit):\n");

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;   // end of input
        }

        if (send_message(server_fd, buffer) == -1) {
            break;
        }

        // wait for the server's reply before prompting again
        char status;
        if (receive_status(server_fd, &status) == 0) {
            printf("Server closed the connection\n");
            break;
        }
        printf("Status: %d\n", status);
    }

    close_socket(server_fd);
    net_cleanup();
    return 0;
}
