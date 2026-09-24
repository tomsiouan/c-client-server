#ifndef SERVER_H
#define SERVER_H

int create_server_socket(int port);
int create_discovery_socket(int port);
int handle_client(int client_fd);
void handle_discovery(int discovery_fd);

#endif
