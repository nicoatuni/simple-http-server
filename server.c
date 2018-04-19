#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

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
    char buffer[2048];

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

        // Process stream of data
        memset(buffer, 0, 2048);

        // Read HTTP request message
        int n = read(new_fd, buffer, 2047);
        if (n < 0) {
            perror("Error reading from socket");
            close(new_fd);
            exit(EXIT_FAILURE);
        }

        // Get the HTTP request-line
        char* input = strtok(buffer, "\n");
        printf("Here is the message: %s\n", input);

        char request[strlen(input)];
        strcpy(request, input);
        printf("Here is the message again: %s\n", request);

        // Get the request-URI
        char* method = strtok(request, " ");
        char* target = strtok(NULL, " ");
        printf("Target file: %s\n", target);

        char* new_target;
        if (strcmp(target, "/") == 0) {
            new_target = "index.html";
        } else {
            new_target = target + 1;
        }

        // Get the full path of the requested resource
        size_t path_len = strlen(path_to_root);
        size_t target_len = strlen(new_target);
        char* file = (char*) malloc((path_len + target_len) * (sizeof *file));
        assert(file);

        sprintf(file, "%s%s", path_to_root, new_target);

        printf("File full path: %s\n", file);

        // Write into response buffer
        char resource[target_len];
        strcpy(resource, new_target);
        printf("Resource: %s\n", resource);

        char* name = strtok(resource, ".");
        char* extension = strtok(NULL, ".");
        printf("File extension: %s\n", extension);

        char* mime_type;
        if (!strcmp(extension, "html")) {
            mime_type = "Content-Type: text/html\r\n";
        } else if (!strcmp(extension, "css")) {
            mime_type = "Content-Type: text/css\r\n";
        } else if (!strcmp(extension, "js")) {
            mime_type = "Content-Type: text/javascript\r\n";
        } else if (!strcmp(extension, "jpg")) {
            mime_type = "Content-Type: image/jpeg\r\n";
        } else {
            mime_type = "\r\n";
        }

        // Formulate HTTP response
        char response[] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html>\n<body>\n<p>Hello!</p>\n</body>\n</html>\0";

        // Send out HTTP response
        n = write(new_fd, response, strlen(response));
        if (n < 0) {
            perror("Error writing to socket");
            close(new_fd);
            exit(EXIT_FAILURE);
        }

        // close connection after everything's done
        close(new_fd);
    }

    close(fd);
    return EXIT_SUCCESS;
}