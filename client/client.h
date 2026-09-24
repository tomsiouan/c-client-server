#ifndef CLIENT_H
#define CLIENT_H

#include <stddef.h>

#ifdef _WIN32
#include <winsock2.h>
typedef SOCKET socket_t;
#else
typedef int socket_t;
#define INVALID_SOCKET -1
#endif

int net_init(void);
void net_cleanup(void);
void close_socket(socket_t fd);

int discover_server(char *ip, size_t ip_size);
socket_t connect_to_server(const char *ip, int port);
int send_message(socket_t server_fd, const char *message);
int receive_status(socket_t server_fd, char *status);

#endif
