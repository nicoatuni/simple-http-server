#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

#define REQUEST_BUFFER_SIZE 2048

/* * * * * * * * * * * * * HELPER FUNCTION PROTOTYPES * * * * * * * * * * * * */
void handle_socket(int port_no, char* path_to_root);
void process_request(int new_fd, char* path_to_root, char request_buffer[]);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s port path_to_root\n", argv[0]);
        return (EXIT_FAILURE);
    }

    // Get command line arguments
    int port_no = atoi(argv[1]);
    char* path_to_root = argv[2];
    /* - - - - - - - - - - - - - - - DEBUGGING - - - - - - - - - - - - - - - */
    printf("Port no: %d\n", port_no);
    printf("Path to web root: %s\n", path_to_root);
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    handle_socket(port_no, path_to_root);

    return EXIT_SUCCESS;
}


/**
 * Handles creating, listening and accepting connections to the socket
 * @param port_no the port number through which incoming connections arrive
 * @param path_to_root the path to web root of the HTTP server
 */
void handle_socket(int port_no, char* path_to_root) {
    int fd, new_fd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;

    // Buffer to store HTTP GET request message
    char request_buffer[REQUEST_BUFFER_SIZE];

    // Create TCP socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    memset((char*) &serv_addr, 0, (sizeof serv_addr));

    // Create address we're going to listen on
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port_no);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(fd, (struct sockaddr*) &serv_addr, (sizeof serv_addr)) < 0) {
        perror("Error during binding");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Listen on socket — incoming connection requests will be queued
    if (listen(fd, 20) < 0) {
        perror("Error when listening");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Keep accepting incoming connection requests until the process is killed
    for (;;) {
        cli_len = sizeof cli_addr;

        // Block until a connection is ready to be accepted
        new_fd = accept(fd, (struct sockaddr*) &cli_addr, &cli_len);
        if (new_fd < 0) {
            perror("Error during accepting");
            close(new_fd);
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Initialise request buffer
        memset(request_buffer, 0, REQUEST_BUFFER_SIZE);

        // Read the HTTP request message
        int n = read(new_fd, request_buffer, REQUEST_BUFFER_SIZE-1);
        if (n < 0) {
            perror("Error reading from socket");
            close(new_fd);
            close(fd);
            exit(EXIT_FAILURE);
        }
        /* - - - - - - - - - - - - - - DEBUGGING - - - - - - - - - - - - - - */
        printf("Request:\n%s", request_buffer);
        printf("Strlen(request): %lu\n", strlen(request_buffer));
        printf("Sizeof request: %lu\n", (sizeof request_buffer));
        /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

        // Process the request message
        process_request(new_fd, path_to_root, request_buffer);
        close(new_fd);
    }

    close(fd);
}


/**
 * Parses and processes HTTP request message
 * @param new_fd the server's socket through which connections come in
 * @param path_to_root the path to web root of the HTTP server
 * @param request_buffer the HTTP request message to be processed
 */
void process_request(int new_fd, char* path_to_root, char request_buffer[]) {
    // Get the request-URI
    char* flush = strtok(request_buffer, " ");
    char* request_uri = strtok(NULL, " ");
    /* - - - - - - - - - - - - - - - DEBUGGING - - - - - - - - - - - - - - - */
    printf("Request URI:\n%s", request_uri);
    printf("Strlen(request URI): %lu\n", strlen(request_uri));
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    // Obtain full path to the requested resource
    char* request_path;
    if (!strcmp(request_uri, "/")) {
        // Redirect root directory to its "index.html"
        request_path = "index.html";

    } else if (request_uri[strlen(request_uri)-1] == '/') {
        // Handle case where the request URI is a directory
        request_uri += 1;
        size_t request_len = strlen("index.html") + strlen(request_uri);
        request_path = (char*)malloc((sizeof *request_path) * (request_len+1));
        assert(request_path);

        sprintf(request_path, "%s%s", request_uri, "index.html");
        request_path[request_len] = '\0';

    } else {
        request_path = request_uri + 1;
    }
    /* - - - - - - - - - - - - - - - DEBUGGING - - - - - - - - - - - - - - - */
    printf("Full request path:\n%s", request_path);
    printf("Strlen(full request path): %lu\n", strlen(request_path));
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    

}