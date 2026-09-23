#ifndef SERVER_H
#define SERVER_H

int create_server_socket(int port);
int handle_client(int client_fd);

#endif
