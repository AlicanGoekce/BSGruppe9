
#include "keyValStore.h"
#include "main.h"

#define PORT 5678

void processCommands(int socket_fd) {
    char buffer[1024];
    char *command, *key, *value, *response;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        read(socket_fd, buffer, 1023);

        command = strtok(buffer, " ");
        key = strtok(NULL, " ");
        value = strtok(NULL, "\n");

        if (strcmp(command, "GET") == 0) {
            if (get(key, &response) == 1) {
                snprintf(buffer, sizeof(buffer), "GET:%s:%s\n", key, response);
            } else {
                snprintf(buffer, sizeof(buffer), "GET:%s:key_nonexistent\n", key);
            }
        } else if (strcmp(command, "PUT") == 0) {
            put(key, value);
            snprintf(buffer, sizeof(buffer), "PUT:%s:%s\n", key, value);
        } else if (strcmp(command, "DEL") == 0) {
            if (del(key) == 1) {
                snprintf(buffer, sizeof(buffer), "DEL:%s:key_deleted\n", key);
            } else {
                snprintf(buffer, sizeof(buffer), "DEL:%s:key_nonexistent\n", key);
            }
        } else if (strcmp(command, "QUIT") == 0) {
            break;
        } else {
            snprintf(buffer, sizeof(buffer), "Invalid Command\n");
        }

        write(socket_fd, buffer, strlen(buffer));
    }
}

int main() {
    int sockfd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        new_socket = accept(sockfd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        processCommands(new_socket);
        close(new_socket);
    }

    return 0;
}