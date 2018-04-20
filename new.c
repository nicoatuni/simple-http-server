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
#define INDEX_HTML "index.html"

/* - - - - - - - - - - - - - - - DEBUGGING - - - - - - - - - - - - - - - - - */
#define REQUEST_PATH "index.html"
#define EXTENSION "html"
#define FULL_PATH "./test/index.html"
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* * * * * * * * * * * * * HELPER FUNCTION PROTOTYPES * * * * * * * * * * * * */
void handle_socket(int port_no, char* path_to_root);
char* process_request(char request_buffer[]);
void process_response(int new_fd, char* path_to_root, char* request_path);

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
 * Handles socket operations as well as receiving and sending HTTP messages
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
        request_buffer[n] = '\0';
        printf("Request:\n%s", request_buffer);
        printf("Strlen(request): %lu\n", strlen(request_buffer));
        printf("Sizeof request: %lu\n", (sizeof request_buffer));
        /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

        // Process the HTTP request message
        char* request_path = process_request(request_buffer);
        
        // Formulate and send the HTTP response message
        process_response(new_fd, path_to_root, request_path);

        free(request_path);
        close(new_fd);
    }

    close(fd);
}


/**
 * Processes the HTTP request message to obtain the path of the requested resource
 * @param request_buffer the HTTP request message to be processed
 * @return the path of the requested resource relative to the root
 */
char* process_request(char request_buffer[]) {
    // Get the request-URI
    char* flush = strtok(request_buffer, " ");
    char* request_uri = strtok(NULL, " ");
    /* - - - - - - - - - - - - - - - DEBUGGING - - - - - - - - - - - - - - - */
    printf("Request URI:\n%s", request_uri);
    printf("Strlen(request URI): %lu\n", strlen(request_uri));
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    // Obtain path to the requested resource
    size_t index_html_len = strlen(INDEX_HTML);
    char* request_path;
    if (!strcmp(request_uri, "/")) {
        // Redirect root directory to its "index.html"
        request_path = (char*)malloc((sizeof *request_path) * (index_html_len+1));
        assert(request_path);

        sprintf(request_path, "%s", INDEX_HTML);

    } else if (request_uri[strlen(request_uri)-1] == '/') {
        // Handle case where the request URI is a directory
        request_uri += 1;
        size_t request_len = index_html_len + strlen(request_uri);
        request_path = (char*)malloc((sizeof *request_path) * (request_len+1));
        assert(request_path);

        sprintf(request_path, "%s%s", request_uri, INDEX_HTML);

    } else {
        request_path = (char*)malloc((sizeof *request_path) * strlen(request_uri));
        assert(request_path);

        sprintf(request_path, "%s", request_uri+1);
    }
    /* - - - - - - - - - - - - - - - DEBUGGING - - - - - - - - - - - - - - - */
    printf("Request path:\n%s", request_path);
    printf("Strlen(request path): %lu\n", strlen(request_path));
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    return request_path;
}

/**
 * Formulate and send the HTTP response message
 * @param new_fd the server's socket through which connections come in
 * @param path_to_root the path to web root of the HTTP server
 * @param request_path the path of the requested resource relative to the root
 */
void process_response(int new_fd, char* path_to_root, char* request_path) {
    // Obtain the requested resource's file extension
    char req_buff[strlen(request_path)];
    strcpy(req_buff, request_path);

    char* name = strtok(req_buff, ".");
    char* extension = strtok(NULL, ".");
    /* - - - - - - - - - - - - - - - DEBUGGING - - - - - - - - - - - - - - - */
    // printf("Full request path after getting extension:\n%s", request_path);
    // printf("Extension: %s\n", extension);
    if (!strcmp(request_path, REQUEST_PATH)) {
        printf("PASS: request path okay!\n");
    } else {
        printf("ERROR: request path is not okay.\n");
    }

    if (!strcmp(extension, EXTENSION)) {
        printf("PASS: extension is okay!\n");
    } else {
        printf("ERROR: extension is not okay.\n");
    }
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    // Get the full path to the resource
    size_t full_path_len = strlen(path_to_root) + strlen(request_path);
    printf("full path len: %lu\n", full_path_len);

    char* full_path = (char*)malloc((sizeof *full_path) * (full_path_len + 1));
    assert(full_path);

    sprintf(full_path, "%s%s", path_to_root, request_path);
    /* - - - - - - - - - - - - - - - DEBUGGING - - - - - - - - - - - - - - - */
    // printf("Path_to_root:\n%s", path_to_root);
    // printf("Request path:\n%s", request_path);
    // printf("Strlen(full_path): %lu\n", strlen(full_path));
    // printf("Full path:\n%s", full_path);
    if (!strcmp(full_path, FULL_PATH)) {
        printf("PASS: Full path is okay!\n");
    } else {
        printf("ERROR: Full path is not okay\n");
    }
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    free(full_path);
}