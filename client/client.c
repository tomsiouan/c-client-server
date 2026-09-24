#include "client.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#define DISCOVERY_ATTEMPTS 3
#define DISCOVERY_TIMEOUT_SECONDS 1

// perror does not know about Winsock errors, so report them through WSAGetLastError
static void print_socket_error(const char *label) {
#ifdef _WIN32
    fprintf(stderr, "%s: error %d\n", label, WSAGetLastError());
#else
    perror(label);
#endif
}

int net_init(void) {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return -1;
    }
#endif
    return 0;
}

void net_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

void close_socket(socket_t fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

// wait up to timeout_seconds for fd to become readable: returns 1 if readable, 0 on timeout, -1 on error
static int wait_readable(socket_t fd, int timeout_seconds) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    struct timeval timeout = {0};
    timeout.tv_sec = timeout_seconds;

    // the first argument is ignored on Windows
    return select((int)fd + 1, &read_fds, NULL, NULL, &timeout);
}

int discover_server(char *ip, size_t ip_size) {
    socket_t discovery_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (discovery_fd == INVALID_SOCKET) {
        print_socket_error("socket");
        return -1;
    }

    int yes = 1;
    if (setsockopt(discovery_fd, SOL_SOCKET, SO_BROADCAST, (const char *)&yes, sizeof(yes)) == -1) {
        print_socket_error("setsockopt");
        close_socket(discovery_fd);
        return -1;
    }

    struct sockaddr_in broadcast_sockaddr_in = {0};
    broadcast_sockaddr_in.sin_family = AF_INET;
    broadcast_sockaddr_in.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_sockaddr_in.sin_port = htons(DISCOVERY_PORT);

    // UDP can drop packets, so ask a few times before giving up
    for (int attempt = 0; attempt < DISCOVERY_ATTEMPTS; attempt++) {
        if (sendto(discovery_fd, DISCOVERY_REQUEST, strlen(DISCOVERY_REQUEST), 0,
                   (struct sockaddr *)&broadcast_sockaddr_in, sizeof(broadcast_sockaddr_in)) == -1) {
            print_socket_error("sendto");
            break;
        }

        while (wait_readable(discovery_fd, DISCOVERY_TIMEOUT_SECONDS) > 0) {
            char buffer[MAX_MESSAGE_SIZE];
            struct sockaddr_in server_sockaddr_in;
            socklen_t len = sizeof(server_sockaddr_in);

            int num_read = recvfrom(discovery_fd, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr *)&server_sockaddr_in, &len);
            if (num_read <= 0) {
                continue;
            }
            buffer[num_read] = '\0';

            // the reply comes from the server itself, so its source address is the one to connect to
            if (strcmp(buffer, DISCOVERY_RESPONSE) == 0 &&
                inet_ntop(AF_INET, &server_sockaddr_in.sin_addr, ip, ip_size) != NULL) {
                close_socket(discovery_fd);
                return 0;
            }
        }
    }

    close_socket(discovery_fd);
    return -1;
}

socket_t connect_to_server(const char *ip, int port) {
    struct sockaddr_in server_sockaddr_in = {0};

    server_sockaddr_in.sin_family = AF_INET;
    server_sockaddr_in.sin_addr.s_addr = inet_addr(ip);
    server_sockaddr_in.sin_port = htons(port);

    socket_t socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == INVALID_SOCKET) {
        print_socket_error("socket");
        return INVALID_SOCKET;
    }

    if (connect(socket_file_descriptor, (struct sockaddr *)&server_sockaddr_in, sizeof(server_sockaddr_in)) == -1) {
        print_socket_error("connect");
        close_socket(socket_file_descriptor);
        return INVALID_SOCKET;
    }

    return socket_file_descriptor;
}

int send_message(socket_t server_fd, const char *message) {
    // send only the characters typed, not the whole buffer
    // (send/recv instead of write/read: Windows sockets are not file descriptors)
    if (send(server_fd, message, strlen(message), 0) == -1) {
        print_socket_error("send");
        return -1;
    }

    return 0;
}

int receive_status(socket_t server_fd, char *status) {
    char buffer[MAX_MESSAGE_SIZE];

    int num_read = recv(server_fd, buffer, sizeof(buffer), 0);
    if (num_read <= 0) {
        return 0;
    }
    *status = buffer[0];

    return 1;
}
