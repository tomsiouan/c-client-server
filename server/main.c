#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include "server.h"
#include "common.h"

int main() {
    int server_fd = create_server_socket(PORT);
    if (server_fd == -1) {
        return 1;
    }

    int discovery_fd = create_discovery_socket(DISCOVERY_PORT);
    if (discovery_fd == -1) {
        close(server_fd);
        return 1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(server_fd, &master);
    FD_SET(discovery_fd, &master);
    int max_fd = server_fd > discovery_fd ? server_fd : discovery_fd;

    while (1) {
        fd_set read_fds = master;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        for (int fd = 0; fd <= max_fd; fd++) {
            if (!FD_ISSET(fd, &read_fds)) {
                continue;
            }

            if (fd == discovery_fd) {
                handle_discovery(discovery_fd);
            } else if (fd == server_fd) {
                struct sockaddr_in client_sockaddr_in;
                socklen_t len = sizeof(client_sockaddr_in);

                int client_fd = accept(server_fd, (struct sockaddr *)&client_sockaddr_in, &len);
                if (client_fd == -1) {
                    perror("accept");
                    continue;
                }

                FD_SET(client_fd, &master);
                if (client_fd > max_fd) {
                    max_fd = client_fd;
                }
            } else {
                if (handle_client(fd) == 0) {
                    close(fd);
                    FD_CLR(fd, &master);
                }
            }
        }
    }

    close(discovery_fd);
    close(server_fd);
    return 0;
}
