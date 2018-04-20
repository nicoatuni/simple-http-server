#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

/* * * * * * * * * * * * * HELPER FUNCTION PROTOTYPES * * * * * * * * * * * * */
void handle_socket(int port_no, char* path_to_root);

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
 */
void handle_socket(int port_no, char* path_to_root) {
    
}