// COMP30023 Sem 1 2018 Assignment 1
// Nico Eka Dinata < n.dinata@student.unimelb.edu.au >
// @ndinata

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
#define FILE_BUFFER_SIZE 512
#define INDEX_HTML "index.html"


/******************************************************************************/
/************************ HELPER FUNCTION PROTOTYPES **************************/
void handle_socket(int port_no, char* path_to_root);
char* get_req_path(char request_buffer[], char* path_to_root);
void process_response(int new_fd, char* full_path);

/******************************************************************************/
/******************************** MAIN ***************************************/
int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s port path_to_root\n", argv[0]);
        return (EXIT_FAILURE);
    }

    // Get command line arguments
    int port_no = atoi(argv[1]);
    char* path_to_root = argv[2];

    // Handle the connection operations
    handle_socket(port_no, path_to_root);

    return EXIT_SUCCESS;
}

/******************************************************************************/
/***************************** HELPER FUNCTIONS *******************************/
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
        request_buffer[n] = '\0';

        // Obtain the full path of the requested resource 
        char* full_path = get_req_path(request_buffer, path_to_root);
        
        // Process and send the HTTP response message
        process_response(new_fd, full_path);

        free(full_path);
        close(new_fd);
    }

    close(fd);
}


/**
 * Processes the HTTP request to obtain the full path of the requested resource
 * @param request_buffer the HTTP request message to be processed
 * @param path_to_root the path to the web root
 * @return the full path of the requested resource
 */
char* get_req_path(char request_buffer[], char* path_to_root) {
    // Get the request-URI
    char* flush = strtok(request_buffer, " ");
    char* request_uri = strtok(NULL, " ");

    // Get the path of the requested resource relative to the web root
    size_t index_html_len = strlen(INDEX_HTML);
    char* relative_path;
    if (!strcmp(request_uri, "/")) {
        // Redirect root directory to its "index.html"
        relative_path = (char*)malloc((sizeof *relative_path) * (index_html_len+2));
        assert(relative_path);

        sprintf(relative_path, "%s%s", "/", INDEX_HTML);

    } else if (request_uri[strlen(request_uri)-1] == '/') {
        // Handle case where the request URI is a directory
        size_t request_len = index_html_len + strlen(request_uri);
        relative_path = (char*)malloc((sizeof *relative_path) * (request_len+1));
        assert(relative_path);

        sprintf(relative_path, "%s%s", request_uri, INDEX_HTML);

    } else {
        relative_path = (char*)malloc((sizeof *relative_path) * (strlen(request_uri)+1));
        assert(relative_path);

        sprintf(relative_path, "%s", request_uri);
    }

    // Get the full path to the resource
    size_t full_path_len = strlen(path_to_root) + strlen(relative_path);

    char* full_path = (char*)malloc((sizeof *full_path) * (full_path_len+1));
    assert(full_path);

    sprintf(full_path, "%s%s", path_to_root, relative_path);

    free(relative_path);
    return full_path;
}


/**
 * Formulate and send the HTTP response message
 * @param new_fd the server's socket through which HTTP messages will be written
 * @param full_path the full path of the requested resource
 */
void process_response(int new_fd, char* full_path) {
    // Obtain the requested resource's file extension
    char req_buff[strlen(full_path)];
    strcpy(req_buff, full_path);

    char* flush = strtok(req_buff, ".");
    char* extension = strtok(NULL, ".");

    long file_len;
    int bytes_read;
    char* status_line;
    int n;

    // Buffer for the file content
    unsigned char file_buffer[FILE_BUFFER_SIZE];
    memset(file_buffer, 0, FILE_BUFFER_SIZE);

    // Read the requested file, if it exists
    FILE* fp = fopen(full_path, "rb");
    if (fp == NULL) {
        status_line = "HTTP/1.0 404 Not Found\r\n\r\n";

        // Send just the response status line if 404
        n = write(new_fd, status_line, strlen(status_line));
        if (n < 0) {
            perror("Error writing response header");
            close(new_fd);
            exit(EXIT_FAILURE);
        }

    } else {
        status_line = "HTTP/1.0 200 OK\r\n";

        // Choose the appropriate MIME-type of the requested file
        char* mime_type;
        if (!strcmp(extension, "html")) {
            mime_type = "Content-Type: text/html\r\n\r\n";
        } else if (!strcmp(extension, "css")) {
            mime_type = "Content-Type: text/css\r\n\r\n";
        } else if (!strcmp(extension, "js")) {
            mime_type = "Content-Type: text/javascript\r\n\r\n";
        } else if (!strcmp(extension, "jpg")) {
            mime_type = "Content-Type: image/jpeg\r\n\r\n";
        } else {
            mime_type = "\r\n";
        }
        
        // Prepare the HTTP response header
        size_t response_len = strlen(status_line) + strlen(mime_type);
        char* response = (char*)malloc((sizeof *response) * response_len);
        assert(response);

        sprintf(response, "%s%s", status_line, mime_type);

        // Send out the HTTP response header
        n = write(new_fd, response, strlen(response));
        if (n < 0) {
            perror("Error writing response header");
            close(new_fd);
            exit(EXIT_FAILURE);
        }

        // Send out the file content
        while ((bytes_read = fread(file_buffer, sizeof(char), FILE_BUFFER_SIZE-1, fp)) > 0) {
            n = write(new_fd, file_buffer, bytes_read);
            if (n < 0) {
                perror("Error writing file content");
                close(new_fd);
                exit(EXIT_FAILURE);
            }
        }

        free(response);
        fclose(fp);
    }
}