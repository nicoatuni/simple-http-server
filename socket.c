#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

// Handles creating, listening, and accepting socket connection
int listen_socket(char* port_number) {
    int fd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;

    // Create TCP socket
    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    memset((char *) &server_addr, 0, sizeof(server_addr));

    int port_number = atoi(port_number);

    // Create address we're going to listen on (given port number) - converted
    // to network byte order & any IP address for this machine
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    // Bind socket
    if (bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error during binding");
        exit(EXIT_FAILURE);
    }

    // Listen on socket - means we're ready to accept connections - incoming
    // connection requests will be queued
    listen(fd, 5);

    client_len = sizeof(client_addr);

    // Accept a connection - block until a connection is ready to be accepted.
    // Get back a new file descriptor to communicate on.
    new_fd = accept(fd, (struct sockaddr *) &client_addr, &client_len);
    if (new_fd < 0) {
        perror("Error during accepting");
        exit(EXIT_FAILURE);
    }

    return new_fd;
}