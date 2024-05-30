#include "keyValStore.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define PORT 5678

void processCommands(int socket_fd, KeyValueStore *kv_store) {
    char buffer[1024];
    char *command, *key, *value, response[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        read(socket_fd, buffer, 1023);

        command = strtok(buffer, " ");
        key = strtok(NULL, " ");
        value = strtok(NULL, "\n");

        if (strcmp(command, "GET") == 0) {
            if (get(kv_store, key, response)) {
                snprintf(buffer, sizeof(buffer), "GET:%s:%s\n", key, response);
            } else {
                snprintf(buffer, sizeof(buffer), "GET:%s:key_nonexistent\n", key);
            }
        } else if (strcmp(command, "PUT") == 0) {
            put(kv_store, key, value);
            snprintf(buffer, sizeof(buffer), "PUT:%s:%s\n", key, value);
        } else if (strcmp(command, "DEL") == 0) {
            if (del(kv_store, key)) {
                snprintf(buffer, sizeof(buffer), "DEL:%s:key_deleted\n", key);
            } else {
                snprintf(buffer, sizeof(buffer), "DEL:%s:key_nonexistent\n", key);
            }
        } else if (strcmp(command, "EXIT") == 0) {
            snprintf(buffer, sizeof(buffer), "EXIT:Bye!\n");
            write(socket_fd, buffer, strlen(buffer));
            close(socket_fd);
            exit(0);
        } else {
            snprintf(buffer, sizeof(buffer), "UNKNOWN_COMMAND\n");
        }

        write(socket_fd, buffer, strlen(buffer));
    }
}

void handleClient(int socket_fd, KeyValueStore *kv_store) {
    if (fork() == 0) {
        // Child process
        processCommands(socket_fd, kv_store);
        exit(0);
    }
    // Parent process
    close(socket_fd);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create shared memory
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, sizeof(KeyValueStore), 0666|IPC_CREAT);
    KeyValueStore *kv_store = (KeyValueStore *) shmat(shmid, (void*)0, 0);

    // Initialize shared memory
    memset(kv_store, 0, sizeof(KeyValueStore));

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port 5678
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        handleClient(new_socket, kv_store);
    }

    // Detach from shared memory
    shmdt(kv_store);

    // Destroy the shared memory
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}