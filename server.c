#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s port path_to_web_root\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Get command line arguments
    char* port_number = argv[1];
    char* path_to_root = argv[2];

    // DEBUGGING
    printf("Port no: %s\n", port_number);
    printf("Path to root: %s\n", path_to_root);

    return EXIT_SUCCESS;
}