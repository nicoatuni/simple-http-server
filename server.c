#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s port path_to_web_root\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Get command line arguments
    int port_number = atoi(argv[1]);
    char* path_to_root = argv[2];

    // DEBUGGING
    printf("Port no: %d\n", port_number);
    printf("Path to root: %s\n", path_to_root);

    // Handle sockets
    int fd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[256];

    // Create TCP socket
    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    memset((char *) &server_addr, 0, sizeof(server_addr));

    // Create address we're going to listen on (given port number) - converted
    // to network byte order & any IP address for this machine
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    // Bind socket
    if (bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("Error during binding");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Listen on socket - means we're ready to accept connections - incoming
    // connection requests will be queued
    if (listen(fd, 20) < 0) {
        perror("Error when listening");
        close(fd);
        exit(EXIT_FAILURE);
    }

    for (;;) {
        client_len = sizeof(client_addr);

        // Block until a connection is ready to be accepted.
        // Get back a new file descriptor to communicate on.
        new_fd = accept(fd, (struct sockaddr *) &client_addr, &client_len);

        if (new_fd < 0) {
            perror("Error during accepting");
            close(new_fd);
            exit(EXIT_FAILURE);
        }

        // process stream of data
        memset(buffer, 0, 256);

        int n = read(new_fd, buffer, 255);
        if (n < 0) {
            perror("Error reading from socket");
            close(new_fd);
            exit(EXIT_FAILURE);
        }

        printf("Here is the message: %s\n", buffer);

        n = write(new_fd, "I got your message", 18);
        if (n < 0) {
            perror("Error writing to socket");
            close(new_fd);
            exit(EXIT_FAILURE);
        }

        // close connection after everything's done
        fprintf(stdout, "Closing connection...\n");
        close(new_fd);
        fprintf(stdout, "Connection closed.\n");
    }

    close(fd);
    return EXIT_SUCCESS;
}